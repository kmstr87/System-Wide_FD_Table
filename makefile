# Defining compiler
CC = gcc

CFLAGS = -Wall -Werror

.PHONY: all

all: lab2

lab2: lab2.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm *.o
