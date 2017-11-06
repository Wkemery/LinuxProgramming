##############################
# Makefile
# Author: Wyatt Emery
# Date: Dec 3, 2016
#
# COSC 3750, Homework 12
# 
# this will compile and generate an executable from mmult.c
##############################

CC=gcc
CFLAGS=-ggdb -Wall

.PHONY: clean

mmult: mmult.c
	${CC} ${CFLAGS} mmult.c -o mmult

clean: 
	/bin/rm -f *.o mmult
