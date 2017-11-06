/*
 * dfunc.c
 * Kim Buckner
 *
 * this function is designed to do nothing important but give a chance
 * to trace execution with the debugger.
 */


int dfunc(int one, int two)
{

  int temp;

  temp=one * 5;
  temp+= two;
  temp=temp * 3;

  return temp;
}

