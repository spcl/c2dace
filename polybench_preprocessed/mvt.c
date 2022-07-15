/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* mvt.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>

static void init_array(int n,
                       double x1[2000 + 0],
                       double x2[2000 + 0],
                       double y_1[2000 + 0],
                       double y_2[2000 + 0],
                       double A[2000 + 0][2000 + 0])
{
  //int i, j;

  for (int i = 0; i < n; i++)
  {
    x1[i] = (double)(i % n) / n;
    x2[i] = (double)((i + 1) % n) / n;
    y_1[i] = (double)((i + 3) % n) / n;
    y_2[i] = (double)((i + 4) % n) / n;
    for (int j = 0; j < n; j++)
    {
      A[i][j] = (double)(i * j % n) / n;
    }
  }
}

static void print_array(int n,
                        double x1[2000 + 0],
                        double x2[2000 + 0])

{
  int i;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "x1");
  for (i = 0; i < n; i++)
  {
    if (i % 20 == 0)
      fprintf(stderr, "\n");
    fprintf(stderr, "%0.2lf ", x1[i]);
  }
  fprintf(stderr, "\nend   dump: %s\n", "x1");

  fprintf(stderr, "begin dump: %s", "x2");
  for (i = 0; i < n; i++)
  {
    if (i % 20 == 0)
      fprintf(stderr, "\n");
    fprintf(stderr, "%0.2lf ", x2[i]);
  }
  fprintf(stderr, "\nend   dump: %s\n", "x2");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_mvt(int n,
                       double x1[2000 + 0],
                       double x2[2000 + 0],
                       double y_1[2000 + 0],
                       double y_2[2000 + 0],
                       double A[2000 + 0][2000 + 0])
{
  int i, j;

#pragma scop
  for (i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++)
    {
      x1[i] = x1[i] + A[i][j] * y_1[j];
    }
  }
  for (i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++)
    {
      x2[i] = x2[i] + A[j][i] * y_2[j];
    }
  }
#pragma endscop
}

int main(int argc, char **argv)
{

  int n = 2000;

  double(*A)[2000 + 0][2000 + 0];
  A = (double(*)[2000 + 0][2000 + 0]) malloc(((2000 + 0) * (2000 + 0)) * sizeof(double));
  ;
  double(*x1)[2000 + 0];
  x1 = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;
  double(*x2)[2000 + 0];
  x2 = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;
  double(*y_1)[2000 + 0];
  y_1 = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;
  double(*y_2)[2000 + 0];
  y_2 = (double(*)[2000 + 0]) malloc((2000 + 0) * sizeof(double));
  ;

  init_array(n,
             *x1,
             *x2,
             *y_1,
             *y_2,
             *A);

  kernel_mvt(n,
             *x1,
             *x2,
             *y_1,
             *y_2,
             *A);

  if (argc > 42 )
  print_array(n, *x1, *x2);
  //argc=0;argv[0]=0;

  free((void *)A);
  ;
  free((void *)x1);
  ;
  free((void *)x2);
  ;
  free((void *)y_1);
  ;
  free((void *)y_2);
  ;

  return 0;
}
