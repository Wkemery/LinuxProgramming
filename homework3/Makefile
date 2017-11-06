##############################
# Makefile
# Wyatt Emery
# COSC 4760 Fall 2016
# 9/21/2017
# 
# this will generate object code for prompt.c display.c and compute.c
# it will then create an executable called approxe from approxe.c 
# which is dependent on the files stated above.
##############################

CC=gcc

# the option -I includes the current directory to search 
# for header files
CFLAGS=-Wall -ggdb -I ./
 
OBJS=prompt.o display.o compute.o

.PHONY: clean

# this compiles its dependancies before anything else
# and then it will compile approxe.c and link the other
# object files with it to make one executable approxe
# you only need to type make because without a specified
# target, make executes the first target.

approxe: approxe.c ${OBJS}
	${CC} ${CFLAGS} ${OBJS} approxe.c -o approxe

prompt.o: prompt.h prompt.c
	${CC} ${CFLAGS} -c prompt.c -o prompt.o

compute.o: compute.h compute.c
	${CC} ${CFLAGS} -c compute.c -o compute.o

display.o: display.h display.c
	${CC} ${CFLAGS} -c display.c -o display.o

clean:
	/bin/rm -f *.o approxe
