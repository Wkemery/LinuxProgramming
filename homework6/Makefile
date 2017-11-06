##############################
# Makefile
# Author: Wyatt Emery
# Date: OCT 7, 2016
#
# COSC 3750, Homework 6
# 
# this will compile and generate an executable from wytar.c using TarFunctions.c
# and linking with the math library
##############################

CC=gcc
CFLAGS=-ggdb -Wall

.PHONY: clean

wytar: wytar.c TarFunctions.c
	${CC} ${CFLAGS} wytar.c TarFunctions.c -lm -o wytar

clean: 
	/bin/rm -f *.o wytar
