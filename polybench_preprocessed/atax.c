/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* atax.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int m, int n,
                       double A[1900 + 0][2100 + 0],
                       double x[2100 + 0])
{
  //int i, j;
  double fn;
  fn = (double)n;

  for (int i = 0; i < n; i++)
    x[i] = 1 + (i / fn);
  for (int i = 0; i < m; i++)
    for (int j = 0; j < n; j++)
      A[i][j] = (double)((i + j) % n) / (5 * m);
}

static void print_array(int n,
                        double y[2100 + 0])

{
  int i;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "y");
  for (i = 0; i < n; i++)
  {
    if (i % 20 == 0)
      fprintf(stderr, "\n");
    fprintf(stderr, "%0.2lf ", y[i]);
  }
  fprintf(stderr, "\nend   dump: %s\n", "y");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_atax(int m, int n,
                        double A[1900 + 0][2100 + 0],
                        double x[2100 + 0],
                        double y[2100 + 0],
                        double tmp[1900 + 0])
{
  // int i, j;

#pragma scop
  for (int i = 0; i < n; i++)
    y[i] = 0;
  for (int i = 0; i < m; i++)
  {
    tmp[i] = 0.0;
    for (int j = 0; j < n; j++)
      tmp[i] = tmp[i] + A[i][j] * x[j];
    for (int j = 0; j < n; j++)
      y[j] = y[j] + A[i][j] * tmp[i];
  }
#pragma endscop
}

int main(int argc, char **argv)
{

  int m = 1900;
  int n = 2100;

  double(*A)[1900 + 0][2100 + 0];
  A = (double(*)[1900 + 0][2100 + 0]) malloc((1900 + 0) * (2100 + 0) * sizeof(double));
  ;
  double(*x)[2100 + 0];
  x = (double(*)[2100 + 0]) malloc((2100 + 0) * sizeof(double));
  ;
  double(*y)[2100 + 0];
  y = (double(*)[2100 + 0]) malloc((2100 + 0) * sizeof(double));
  ;
  double(*tmp)[1900 + 0];
  tmp = (double(*)[1900 + 0]) malloc((1900 + 0) * sizeof(double));
  ;

  init_array(m, n, *A, *x);

  kernel_atax(m, n,
              *A,
              *x,
              *y,
              *tmp);

  if (argc > 42 )
  print_array(n, *y);

  //argc=0;argv[0]=0;
  free((void *)A);
  ;
  free((void *)x);
  ;
  free((void *)y);
  ;
  free((void *)tmp);
  ;

  return 0;
}
