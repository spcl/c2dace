#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>

void waxpby (double* x, double* y, double* w)
{  
  int i = 0;
  if (1) {
    w[i] = x[i];
  }
}

int main(int argc, char *argv[])
{
  int size = 2;
  double* p = malloc(size * sizeof(double)); // In parallel case, A is rectangular
  double* y = malloc(size * sizeof(double));
  double* x = malloc(size * sizeof(double));

  waxpby(x, y, p);
  size++;

  return p[0] + size;
} 