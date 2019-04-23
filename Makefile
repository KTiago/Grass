SRCDIR   = src
BINDIR   = bin
INCLUDES = include

CC=g++
CFLAGS= -Wall -Wextra -g -fno-stack-protector -z execstack -pthread -std=c++11 -I $(INCLUDES)/ 
DEPS = $(wildcard $(INCLUDES)/%.h)

all: $(BINDIR)/client $(BINDIR)/server $(DEPS)

$(BINDIR)/client: $(SRCDIR)/client.cpp networking.o
	$(CC) $(CFLAGS) $< networking.o -o $@

$(BINDIR)/server: $(SRCDIR)/server.cpp Parser.o User.o networking.o
	$(CC) $(CFLAGS) $< Parser.o commands.o User.o networking.o -o $@
	rm Parser.o commands.o
	
User.o: $(SRCDIR)/User.cpp $(SRCDIR)/User.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/User.cpp

Parser.o: $(SRCDIR)/Parser.cpp $(SRCDIR)/Parser.h commands.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/Parser.cpp
	
commands.o: $(SRCDIR)/commands.cpp $(SRCDIR)/commands.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/commands.cpp

networking.o: $(SRCDIR)/networking.cpp $(SRCDIR)/networking.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/networking.cpp
	
.PHONY: clean
clean:
	rm -f $(BINDIR)/client $(BINDIR)/server Parser.o commands.o
