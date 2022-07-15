/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* symm.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int m, int n,
                       double *alpha,
                       double *beta,
                       double C[1000 + 0][1200 + 0],
                       double A[1000 + 0][1000 + 0],
                       double B[1000 + 0][1200 + 0])
{
  //int i, j;

  *alpha = 1.5;
  *beta = 1.2;
  for (int i = 0; i < m; i++)
    for (int j = 0; j < n; j++)
    {
      C[i][j] = (double)((i + j) % 100) / m;
      B[i][j] = (double)((n + i - j) % 100) / m;
    }
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j <= i; j++)
      A[i][j] = (double)((i + j) % 100) / m;
    for (int j = i + 1; j < m; j++)
      A[i][j] = -999;
  }
}

static void print_array(int m, int n,
                        double C[1000 + 0][1200 + 0])
{
  //int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "C");
  for (int i = 0; i < m; i++)
    for (int j = 0; j < n; j++)
    {
      if ((i * m + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", C[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "C");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_symm(int m, int n,
                        double alpha,
                        double beta,
                        double C[1000 + 0][1200 + 0],
                        double A[1000 + 0][1000 + 0],
                        double B[1000 + 0][1200 + 0])
{
  //int i, j, k;
  double temp2;
#pragma scop
  for (int i = 0; i < m; i++)
    for (int j = 0; j < n; j++)
    {
      temp2 = 0;
      for (int k = 0; k < i; k++)
      {
        C[k][j] = C[k][j] + (alpha * B[i][j] * A[i][k]);
        temp2 = temp2 + B[k][j] * A[i][k];
      }
      C[i][j] = beta * C[i][j] + alpha * B[i][j] * A[i][i] + alpha * temp2;
    }
#pragma endscop
}

int main(int argc, char **argv)
{

  int m = 1000;
  int n = 1200;

  double alpha;
  double beta;
  double(*C)[1000 + 0][1200 + 0];
  C = (double(*)[1000 + 0][1200 + 0]) malloc((1000 + 0) * (1200 + 0) * sizeof(double));
  ;
  double(*A)[1000 + 0][1000 + 0];
  A = (double(*)[1000 + 0][1000 + 0]) malloc((1000 + 0) * (1000 + 0) * sizeof(double));
  ;
  double(*B)[1000 + 0][1200 + 0];
  B = (double(*)[1000 + 0][1200 + 0]) malloc((1000 + 0) * (1200 + 0) * sizeof(double));
  ;

  init_array(m, n, &alpha, &beta,
             *C,
             *A,
             *B);

  kernel_symm(m, n,
              alpha, beta,
              *C,
              *A,
              *B);

    if (argc > 42 )
  print_array(m, n, *C);

  //argc=0;argv[0]=0;
  free((void *)C);
  ;
  free((void *)A);
  ;
  free((void *)B);
  ;

  return 0;
}
