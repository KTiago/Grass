SRCDIR   = src
BINDIR   = bin
INCLUDES = include

CC=g++
CFLAGS= -Wall -Wextra -g -fno-stack-protector -z execstack -lpthread -std=c++11 -I $(INCLUDES)/ -m32
DEPS = $(wildcard $(INCLUDES)/%.h)

all: $(BINDIR)/client $(BINDIR)/server $(DEPS)

$(BINDIR)/client: $(SRCDIR)/client.cpp networking.o
	$(CC) $(CFLAGS) $< networking.o -o $@

$(BINDIR)/server: $(SRCDIR)/server.cpp parser.o user.o networking.o
	$(CC) $(CFLAGS) $< parser.o commands.o user.o networking.o -o $@
	rm parser.o commands.o	
	
user.o: $(SRCDIR)/user.cpp $(SRCDIR)/user.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/user.cpp

parser.o: $(SRCDIR)/parser.cpp $(SRCDIR)/parser.h commands.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/parser.cpp
	
commands.o: $(SRCDIR)/commands.cpp $(SRCDIR)/commands.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/commands.cpp

networking.o: $(SRCDIR)/networking.cpp $(SRCDIR)/networking.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/networking.cpp
	
.PHONY: clean
clean:
	rm -f $(BINDIR)/client $(BINDIR)/server parser.o commands.o
