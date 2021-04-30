# From https://cs.colby.edu/maxwell/courses/tutorials/maketutor/

CC=gcc
CFLAGS=-I.
DEPS = mssb_16.h mssb_64.h utils.h
OBJ = mssb_16.o mssb_64.o main.o utils.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

