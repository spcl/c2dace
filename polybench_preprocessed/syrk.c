/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* syrk.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int n, int m,
                       double *alpha,
                       double *beta,
                       double C[1200 + 0][1200 + 0],
                       double A[1200 + 0][1000 + 0])
{
  //  int i, j;

  *alpha = 1.5;
  *beta = 1.2;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < m; j++)
      A[i][j] = (double)((i * j + 1) % n) / n;
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      C[i][j] = (double)((i * j + 2) % m) / m;
}

static void print_array(int n,
                        double C[1200 + 0][1200 + 0])
{
  int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "C");
  for (i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++)
    {
      if ((i * n + j) % 20 == 0)
      {
        fprintf(stderr, "\n");
      }
      fprintf(stderr, "%0.2lf ", C[i][j]);
    }
  }
  fprintf(stderr, "\nend   dump: %s\n", "C");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_syrk(int n, int m,
                        double alpha,
                        double beta,
                        double C[1200 + 0][1200 + 0],
                        double A[1200 + 0][1000 + 0])
{

#pragma scop
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j <= i; j++)
    {
      C[i][j] = C[i][j] * beta;
    }
    for (int k = 0; k < m; k++)
    {
      for (int j = 0; j <= i; j++)
      {
        C[i][j] = C[i][j] + alpha * A[i][k] * A[j][k];
      }
    }
  }
#pragma endscop
}

int main(int argc, char **argv)
{

  int n = 1200;
  int m = 1000;

  double alpha;
  double beta;
  double(*C)[1200 + 0][1200 + 0];
  C = (double(*)[1200 + 0][1200 + 0]) malloc(((1200 + 0) * (1200 + 0)) * sizeof(double));
  ;
  double(*A)[1200 + 0][1000 + 0];
  A = (double(*)[1200 + 0][1000 + 0]) malloc(((1200 + 0) * (1000 + 0)) * sizeof(double));

  init_array(n, m, &alpha, &beta, *C, *A);

  kernel_syrk(n, m, alpha, beta, *C, *A);

   if (argc > 42 )
  print_array(n, *C);

  //argc=0;argv[0]=0;
  free((void *)C);
  ;
  free((void *)A);
  ;

  return 0;
}
