CC = gcc

CFLAGS = -ansi -Wall -pedantic

OBJ = mycomp.o parser.o complex.o

mycomp: $(OBJ)
	$(CC) $(CFLAGS) -o mycomp $(OBJ) -lm

mycomp.o: mycomp.c parser.h complex.h
	$(CC) $(CFLAGS) -c mycomp.c

parser.o: parser.c parser.h complex.h
	$(CC) $(CFLAGS) -c parser.c

complex.o: complex.c complex.h
	$(CC) $(CFLAGS) -c complex.c

clean:
	rm -f *.o mycomp