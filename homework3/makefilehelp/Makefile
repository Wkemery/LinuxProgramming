############################################
# Makefile
# Kim Buckner
#
# Fall 2016
# This file is copied from an existing project for use
# in demonstrating makefiles and the compilation process.
# ##########################################
LEX=flex
LFLAGS=--warn
CC=gcc
#CFLAGS=-ggdb -fno-diagnostics-color
CFLAGS=-ggdb 

.PHONY: clean tidy install

generate: lex.yy.c
	${CC} ${CFLAGS} lex.yy.c -o generate 

lex.yy.c: slides.l
	${LEX} ${LFLAGS} slides.l

proba: proba.c dfunc.o
	${CC} ${CFLAGS} proba.c dfunc.o -o proba

dfunc.o: dfunc.h dfunc.c
	${CC} ${CFLAGS} -c dfunc.c


hello: hello.c
	${CC} ${CFLAGS} hello.c -o Dhello
	${CC} ${CFLAGS} -S hello.c -o Dhello.s
	${CC} ${CFLAGS} -E hello.c -o Dhello.i
	${CC} ${CFLAGS} -c hello.c -o Dhello.o
	${CC} hello.c -o hello
	${CC} -S hello.c -o hello.s
	${CC} -E hello.c -o hello.i
	${CC} -c hello.c -o hello.o


install:
	mv -f generate ${HOME}/bin

tidy: 
	/bin/rm -f lex.yy.c *.o *.s *.i a.out

clean: tidy
	/bin/rm -f generate proba hello Dhello
