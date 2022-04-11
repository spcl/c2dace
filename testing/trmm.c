/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* trmm.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int m, int n,
                       double *alpha,
                       double A[1000 + 0][1000 + 0],
                       double B[1000 + 0][1200 + 0])
{
  //int i, j;

  *alpha = 1.5;
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < i; j++)
    {
      A[i][j] = (double)((i + j) % m) / m;
    }
    A[i][i] = 1.0;
    for (int j = 0; j < n; j++)
    {
      B[i][j] = (double)((n + (i - j)) % n) / n;
    }
  }
}

static void print_array(int m, int n,
                        double B[1000 + 0][1200 + 0])
{
  int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "B");
  for (i = 0; i < m; i++)
    for (j = 0; j < n; j++)
    {
      if ((i * m + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", B[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "B");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_trmm(int m, int n,
                        double alpha,
                        double A[1000 + 0][1000 + 0],
                        double B[1000 + 0][1200 + 0])
{
  // int i, j, k;
# 68 "trmm.c"
#pragma scop
  for (int i = 0; i < m; i++)
    for (int j = 0; j < n; j++)
    {
      for (int k = i + 1; k < m; k++)
        B[i][j] = B[i][j] + (A[k][i] * B[k][j]);
      B[i][j] = alpha * B[i][j];
    }
#pragma endscop
}

int main(int argc, char **argv)
{

  int m = 1000;
  int n = 1200;

  double alpha;
  double(*A)[1000 + 0][1000 + 0];
  A = (double(*)[1000 + 0][1000 + 0]) malloc((1000 + 0) * (1000 + 0) * sizeof(double));
  ;
  double(*B)[1000 + 0][1200 + 0];
  B = (double(*)[1000 + 0][1200 + 0]) malloc((1000 + 0) * (1200 + 0) * sizeof(double));
  ;

  init_array(m, n, &alpha, *A, *B);

  kernel_trmm(m, n, alpha, *A, *B);

  if (argc > 42 )
  print_array(m, n, *B);
  //argc=0;argv[0]=0;

  free((void *)A);
  ;
  free((void *)B);
  ;

  return 0;
}
