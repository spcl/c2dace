/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* trisolv.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>

static void init_array(int n,
                       double L[2000 + 0][2000 + 0],
                       double x[2000 + 0],
                       double b[2000 + 0])
{
  // int i, j;

  for (int i = 0; i < n; i++)
  {
    x[i] = -999;
    b[i] = i;
    for (int j = 0; j <= i; j++)
      L[i][j] = (double)(i + n - j + 1) * 2 / n;
  }
}

static void print_array(int n,
                        double x[2000 + 0])

{
  int i;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "x");
  for (i = 0; i < n; i++)
  {
    fprintf(stderr, "%0.2lf ", x[i]);
    if (i % 20 == 0)
      fprintf(stderr, "\n");
  }
  fprintf(stderr, "\nend   dump: %s\n", "x");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_trisolv(int n,
                           double L[2000 + 0][2000 + 0],
                           double x[2000 + 0],
                           double b[2000 + 0])
{
  //int i, j;

#pragma scop
  for (int i = 0; i < n; i++)
  {
    x[i] = b[i];
    for (int j = 0; j < i; j++)
    {
      double tmp = -L[i][j] * x[j];
      x[i] = x[i] + tmp;
    }
    x[i] = x[i] / L[i][i];
  }
#pragma endscop
}

int main(int argc, char **argv)
{

  int n = 2000;

  double(*L)[2000 + 0][2000 + 0];
  L = (double(*)[2000 + 0][2000 + 0]) malloc((2000 + 0) * (2000 + 0) * sizeof(double));
  ;
  double(*x)[2000 + 0];
  x = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;
  double(*b)[2000 + 0];
  b = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;

  init_array(n, *L, *x, *b);

  kernel_trisolv(n, *L, *x, *b);

   if (argc > 42)
  print_array(n, *x);
  //argc=0;argv[0]=0;

  free((void *)L);
  ;
  free((void *)x);
  ;
  free((void *)b);
  ;

  return 0;
}
