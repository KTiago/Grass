SRCDIR   = src
BINDIR   = bin
INCLUDES = include

CC=g++
CFLAGS= -Wall -Wextra -g -fno-stack-protector -z execstack -lpthread -std=c++11 -I $(INCLUDES)/ -m32
DEPS = $(wildcard $(INCLUDES)/%.h)

all: $(BINDIR)/client $(BINDIR)/server $(DEPS)

$(BINDIR)/client: $(SRCDIR)/client.cpp
	$(CC) $(CFLAGS) $< -o $@

$(BINDIR)/server: $(SRCDIR)/server.cpp parser.o
	$(CC) $(CFLAGS) $< parser.o commands.o -o $@
	rm parser.o commands.o	

parser.o: $(SRCDIR)/parser.cpp $(SRCDIR)/parser.h commands.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/parser.cpp
	
commands.o: $(SRCDIR)/commands.cpp $(SRCDIR)/commands.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/commands.cpp
	
.PHONY: clean
clean:
	rm -f $(BINDIR)/client $(BINDIR)/server parser.o commands.o
