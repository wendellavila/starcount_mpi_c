CC=gcc
CFLAGS=-lz -lpng16

starCount: main.c utils.c
	$(CC) $(CFLAGS) main.c utils.c -o starCount.