##############################
# Makefile
# Author: Wyatt Emery
# Date: Oct 18, 2016
#
# COSC 3750, Homework 7
# 
# this will compile and generate an executable from wytalkc.c and socketfun.c
##############################

CC=gcc
CFLAGS=-ggdb -Wall

.PHONY: clean

all: wytalkc wytalkd

wytalkc: wytalkc.c socketfun.h socketfun.c
	${CC} ${CFLAGS} wytalkc.c socketfun.c -o wytalkc

wytalkd: wytalkd.c socketfun.h socketfun.c
	${CC} ${CFLAGS} wytalkd.c socketfun.c -o wytalkd

clean: 
	/bin/rm -f *.o wytalkc
