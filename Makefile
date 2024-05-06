CC=gcc
CFLAGS=-g -Wall -pthread
LIBS=-lpthread

all: barber

barber: barber.c
	$(CC) $(CFLAGS) -o barber barber.c $(LIBS)

clean:
	rm -f barber

run: barber
	./barber