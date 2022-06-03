#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  int size = 2;
  double* p = malloc(size * sizeof(double)); // In parallel case, A is rectangular

  if (1) {
    p[0] = 12;
  } else {
    p[0] = 32;
  }

  return 0;
} 