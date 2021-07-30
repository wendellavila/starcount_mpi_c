CC=mpicc
CFLAGS=-lz -lpng16 -g

starCount: main.c utils.c
	$(CC) $(CFLAGS) main.c utils.c -o starCount
