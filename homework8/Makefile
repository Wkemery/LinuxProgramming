##############################
# Makefile
# Author: Wyatt Emery
# Date: Oct 29, 2016
#
# COSC 3750, Homework 8
# 
# this will compile and generate an executable from wyget.c
##############################

CC=gcc
CFLAGS=-ggdb -Wall

.PHONY: clean

wyget: wyget.c socketfun.c socketfun.h wygetFunctions.c wygetFunctions.h
	${CC} ${CFLAGS} wyget.c socketfun.c wygetFunctions.c -o wyget

clean: 
	/bin/rm -f *.o wyget
