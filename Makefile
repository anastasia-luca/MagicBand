CC=gcc -g
CFLAGS=-Wall -std=c99

build: tema1

tema1: 
	$(CC) $(CFLAGS) -o tema1 tema1.c
clean:
	rm tema1

run: build 
	./tema1
