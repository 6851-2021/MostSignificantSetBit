# From https://cs.colby.edu/maxwell/courses/tutorials/maketutor/

CC=gcc
CFLAGS=-I.
DEPS = mssb_8.h mssb_16.h mssb_32.h mssb_64.h utils.h
OBJ = mssb_8.o mssb_8_lookup.o mssb_16.o mssb_16_lookup.o mssb_32.o mssb_64.o main.o utils.o

%.o: %.c $(DEPS)
	$(CC) -O3 -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -O3 -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o main
