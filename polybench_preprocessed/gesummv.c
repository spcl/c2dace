/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* gesummv.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int n,
                       double *alpha,
                       double *beta,
                       double A[1300 + 0][1300 + 0],
                       double B[1300 + 0][1300 + 0],
                       double x[1300 + 0])
{
  //int i, j;

  *alpha = 1.5;
  *beta = 1.2;
  for (int i = 0; i < n; i++)
  {
    x[i] = (double)(i % n) / n;
    for (int j = 0; j < n; j++)
    {
      A[i][j] = (double)((i * j + 1) % n) / n;
      B[i][j] = (double)((i * j + 2) % n) / n;
    }
  }
}

static void print_array(int n,
                        double y[1300 + 0])

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

static void kernel_gesummv(int n,
                           double alpha,
                           double beta,
                           double A[1300 + 0][1300 + 0],
                           double B[1300 + 0][1300 + 0],
                           double tmp[1300 + 0],
                           double x[1300 + 0],
                           double y[1300 + 0])
{
  //int i, j;

#pragma scop
  for (int i = 0; i < n; i++)
  {
    tmp[i] = 0.0;
    y[i] = 0.0;
    for (int j = 0; j < n; j++)
    {
      tmp[i] = A[i][j] * x[j] + tmp[i];
      y[i] = B[i][j] * x[j] + y[i];
    }
    y[i] = alpha * tmp[i] + beta * y[i];
  }
#pragma endscop
}

int main(int argc, char **argv)
{

  int n = 1300;

  double alpha;
  double beta;
  double(*A)[1300 + 0][1300 + 0];
  A = (double(*)[1300 + 0][1300 + 0]) malloc(((1300 + 0) * (1300 + 0)) * sizeof(double));
  ;
  double(*B)[1300 + 0][1300 + 0];
  B = (double(*)[1300 + 0][1300 + 0]) malloc(((1300 + 0) * (1300 + 0)) * sizeof(double));
  ;
  double(*tmp)[1300 + 0];
  tmp = (double(*)[1300 + 0]) malloc((1300 + 0) * sizeof(double));
  ;
  double(*x)[1300 + 0];
  x = (double(*)[1300 + 0]) malloc((1300 + 0) * sizeof(double));
  ;
  double(*y)[1300 + 0];
  y = (double(*)[1300 + 0]) malloc((1300 + 0) * sizeof(double));
  ;

  init_array(n, &alpha, &beta,
             *A,
             *B,
             *x);

  kernel_gesummv(n, alpha, beta,
                 *A,
                 *B,
                 *tmp,
                 *x,
                 *y);

  if (argc > 42 )
  print_array(n, *y);

  //argc=0;argv[0]=0;
  free((void *)A);
  ;
  free((void *)B);
  ;
  free((void *)tmp);
  ;
  free((void *)x);
  ;
  free((void *)y);
  ;

  return 0;
}
