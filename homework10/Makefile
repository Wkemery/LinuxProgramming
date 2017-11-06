##############################
# Makefile
# Author: Wyatt Emery
# Date: NOV 15, 2016
#
# COSC 3750, Homework 10
# 
# this will compile and generate an executable from wyshell.c
##############################

CC=gcc
CFLAGS=-ggdb -Wall

.PHONY: clean


wyls: wyshell.c ShellFunctions.c ShellFunctions.h 
	${CC} ${CFLAGS} wyshell.c wyscanner.o ShellFunctions.c -o wyshell

wyscanner: wyscanner.c wyscanner.h
	${CC} ${CFLAGS} wyscanner.c -c

clean: 
	/bin/rm -f *.o wyshell
