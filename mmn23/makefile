CC = gcc

CFLAGS = -ansi -Wall -pedantic

OBJ = seek.o

seek: $(OBJ)
	$(CC) $(CFLAGS) -o seek $(OBJ) -lm

seek.o: seek.c
	$(CC) $(CFLAGS) -c seek.c

clean:
	rm -f *.o seek