##############################
# Makefile
# Author: Wyatt Emery
# Date: Sep 30, 2016
#
# COSC 3750, Homework 5
# 
# this will compile and generate an executable from wyls.c
##############################

CC=gcc
CFLAGS=-ggdb -Wall

.PHONY: clean

wyls: wyls.c
	${CC} ${CFLAGS} wyls.c -o wyls

clean: 
	/bin/rm -f *.o wyls
