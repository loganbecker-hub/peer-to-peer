CC=gcc
CFLAGS= -Wall -std=c99

all: multi.o

multi.o:multi.c
	$(CC) $(CFLAGS) $< -o $@ -lpthread

clean:
	rm *.o

run:
	./multi.o

.PHONY: all clean run
