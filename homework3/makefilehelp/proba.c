/*
 * proba.c
 *
 * Kim Buckner
 *
 * this program is for testing and using the debugger.
 * doesn't do anything fun or exciting
 *
 * requires dfunc.c and dfunc.h
 */

#include<stdio.h>
#include"dfunc.h"


int main(int argc, char **argv)
{
  int a, b;
  int retn;
  int i;

  for(i=0;i<argc;i++) {
    printf("argv[%d]: %s\n",i,argv[i]);
  }

  a=10;
  b=12;

  while(10 < 12) {
    printf("a: %d\n",a);
    a--;
    b++;
  }

  retn=dfunc(a,b);

  printf("dfunc(%d,%d): %d\n",a,b,retn);

  return 0;
}
