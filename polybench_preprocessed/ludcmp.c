/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* ludcmp.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>

static void init_array(int n,
                       double A[2000 + 0][2000 + 0],
                       double b[2000 + 0],
                       double x[2000 + 0],
                       double y[2000 + 0])
{
  //int i, j;
  double fn = (double)n;

  for (int i = 0; i < n; i++)
  {
    x[i] = 0;
    y[i] = 0;
    b[i] = (i + 1) / fn / 2.0 + 4;
  }

  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j <= i; j++)
      A[i][j] = (double)(-j % n) / n + 1;
    for (int j = i + 1; j < n; j++)
    {
      A[i][j] = 0;
    }
    A[i][i] = 1;
  }

  //int r,s,t;
  double B[2000 + 0][2000 + 0]; // B = (double(*)[2000 + 0][2000 + 0])malloc ((2000 + 0) * (2000 + 0)* sizeof(double));;
  for (int r = 0; r < n; ++r)
    for (int s = 0; s < n; ++s)
      B[r][s] = 0;
  for (int t = 0; t < n; ++t)
    for (int r = 0; r < n; ++r)
      for (int s = 0; s < n; ++s)
        B[r][s] = B[r][s] + A[r][t] * A[s][t];
  for (int r = 0; r < n; ++r)
    for (int s = 0; s < n; ++s)
      A[r][s] = B[r][s];
  //free((void*)B);;
}

static void print_array(int n,
                        double x[2000 + 0])

{
  int i;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "x");
  for (i = 0; i < n; i++)
  {
    if (i % 20 == 0)
      fprintf(stderr, "\n");
    fprintf(stderr, "%0.2lf ", x[i]);
  }
  fprintf(stderr, "\nend   dump: %s\n", "x");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_ludcmp(int n,
                          double A[2000 + 0][2000 + 0],
                          double b[2000 + 0],
                          double x[2000 + 0],
                          double y[2000 + 0])
{
  //int i, j, k;

  double w;

#pragma scop
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < i; j++)
    {
      w = A[i][j];
      for (int k = 0; k < j; k++)
      {
        w = w - A[i][k] * A[k][j];
      }
      A[i][j] = w / A[j][j];
    }
    for (int j = i; j < n; j++)
    {
      w = A[i][j];
      for (int k = 0; k < i; k++)
      {
        w = w - A[i][k] * A[k][j];
      }
      A[i][j] = w;
    }
  }

  for (int i = 0; i < n; i++)
  {
    w = b[i];
    for (int j = 0; j < i; j++)
      w = w - A[i][j] * y[j];
    y[i] = w;
  }

  for (int i = n - 1; i >= 0; i--)
  {
    w = y[i];
    for (int j = i + 1; j < n; j++)
      w = w - A[i][j] * x[j];
    x[i] = w / A[i][i];
  }
#pragma endscop
}

int main(int argc, char **argv)
{

  int n = 2000;

  double(*A)[2000 + 0][2000 + 0];
  A = (double(*)[2000 + 0][2000 + 0]) malloc((2000 + 0) * (2000 + 0) * sizeof(double));
  ;
  double(*b)[2000 + 0];
  b = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;
  double(*x)[2000 + 0];
  x = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;
  double(*y)[2000 + 0];
  y = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;

  init_array(n,
             *A,
             *b,
             *x,
             *y);

  kernel_ludcmp(n,
                *A,
                *b,
                *x,
                *y);

  if (argc > 42 )
  print_array(n, *x);
  //argc=0;argv[0]=0;

  free((void *)A);
  ;
  free((void *)b);
  ;
  free((void *)x);
  ;
  free((void *)y);
  ;

  return 0;
}
