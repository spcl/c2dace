/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* gemver.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int n,
                       double *alpha,
                       double *beta,
                       double A[2000 + 0][2000 + 0],
                       double u1[2000 + 0],
                       double v1[2000 + 0],
                       double u2[2000 + 0],
                       double v2[2000 + 0],
                       double w[2000 + 0],
                       double x[2000 + 0],
                       double y[2000 + 0],
                       double z[2000 + 0])
{
  //int i, j;

  *alpha = 1.5;
  *beta = 1.2;

  double fn = (double)n;

  for (int i = 0; i < n; i++)
  {
    u1[i] = i;
    u2[i] = ((i + 1) / fn) / 2.0;
    v1[i] = ((i + 1) / fn) / 4.0;
    v2[i] = ((i + 1) / fn) / 6.0;
    y[i] = ((i + 1) / fn) / 8.0;
    z[i] = ((i + 1) / fn) / 9.0;
    x[i] = 0.0;
    w[i] = 0.0;
    for (int j = 0; j < n; j++)
      A[i][j] = (double)(i * j % n) / n;
  }
}

static void print_array(int n,
                        double w[2000 + 0])
{
  int i;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "w");
  for (i = 0; i < n; i++)
  {
    if (i % 20 == 0)
      fprintf(stderr, "\n");
    fprintf(stderr, "%0.2lf ", w[i]);
  }
  fprintf(stderr, "\nend   dump: %s\n", "w");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_gemver(int n,
                          double alpha,
                          double beta,
                          double A[2000 + 0][2000 + 0],
                          double u1[2000 + 0],
                          double v1[2000 + 0],
                          double u2[2000 + 0],
                          double v2[2000 + 0],
                          double w[2000 + 0],
                          double x[2000 + 0],
                          double y[2000 + 0],
                          double z[2000 + 0])
{
  //  int i, j;

#pragma scop

  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      A[i][j] = A[i][j] + u1[i] * v1[j] + u2[i] * v2[j];

  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      x[i] = x[i] + beta * A[j][i] * y[j];

  for (int i = 0; i < n; i++)
    x[i] = x[i] + z[i];

  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      w[i] = w[i] + alpha * A[i][j] * x[j];

#pragma endscop
}

int main(int argc, char **argv)
{

  int n = 2000;

  double alpha;
  double beta;
  double(*A)[2000 + 0][2000 + 0];
  A = (double(*)[2000 + 0][2000 + 0]) malloc(((2000 + 0) * (2000 + 0)) * sizeof(double));
  ;
  double(*u1)[2000 + 0];
  u1 = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;
  double(*v1)[2000 + 0];
  v1 = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;
  double(*u2)[2000 + 0];
  u2 = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;
  double(*v2)[2000 + 0];
  v2 = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;
  double(*w)[2000 + 0];
  w = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;
  double(*x)[2000 + 0];
  x = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;
  double(*y)[2000 + 0];
  y = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;
  double(*z)[2000 + 0];
  z = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;

  init_array(n, &alpha, &beta,
             *A,
             *u1,
             *v1,
             *u2,
             *v2,
             *w,
             *x,
             *y,
             *z);

  kernel_gemver(n, alpha, beta,
                *A,
                *u1,
                *v1,
                *u2,
                *v2,
                *w,
                *x,
                *y,
                *z);

  if (argc > 42 )

  print_array(n, *w);

  //argc=0;argv[0]=0;
  free((void *)A);
  ;
  free((void *)u1);
  ;
  free((void *)v1);
  ;
  free((void *)u2);
  ;
  free((void *)v2);
  ;
  free((void *)w);
  ;
  free((void *)x);
  ;
  free((void *)y);
  ;
  free((void *)z);
  ;

  return 0;
}
