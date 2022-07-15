/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* gemm.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int ni, int nj, int nk,
                       double *alpha,
                       double *beta,
                       double C[1000 + 0][1100 + 0],
                       double A[1000 + 0][1200 + 0],
                       double B[1200 + 0][1100 + 0])
{
  // int i, j;

  *alpha = 1.5;
  *beta = 1.2;
  for (int i = 0; i < ni; i++)
    for (int j = 0; j < nj; j++)
      C[i][j] = (double)((i * j + 1) % ni) / ni;
  for (int i = 0; i < ni; i++)
    for (int j = 0; j < nk; j++)
      A[i][j] = (double)(i * (j + 1) % nk) / nk;
  for (int i = 0; i < nk; i++)
    for (int j = 0; j < nj; j++)
      B[i][j] = (double)(i * (j + 2) % nj) / nj;
}

static void print_array(int ni, int nj,
                        double C[1000 + 0][1100 + 0])
{
  //int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "C");
  for (int i = 0; i < ni; i++)
    for (int j = 0; j < nj; j++)
    {
      if ((i * ni + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", C[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "C");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_gemm(int ni, int nj, int nk,
                        double alpha,
                        double beta,
                        double C[1000 + 0][1100 + 0],
                        double A[1000 + 0][1200 + 0],
                        double B[1200 + 0][1100 + 0])
{
  //int i, j, k;
#pragma scop
  for (int i = 0; i < ni; i++)
  {
    for (int j = 0; j < nj; j++)
    {
      C[i][j] = C[i][j] * beta;
      // C[i][j] = C[i][j] * beta + C[j][i];
    }
    for (int k = 0; k < nk; k++)
    {
      for (int j = 0; j < nj; j++)
        C[i][j] = C[i][j] + (alpha * A[i][k] * B[k][j]);
    }
  }
#pragma endscop
}

int main(int argc, char **argv)
{

  int ni = 1000;
  int nj = 1100;
  int nk = 1200;

  double alpha;
  double beta;
  double(*C)[1000 + 0][1100 + 0];
  C = (double(*)[1000 + 0][1100 + 0]) malloc(((1000 + 0) * (1100 + 0)) * sizeof(double));
  ;
  double(*A)[1000 + 0][1200 + 0];
  A = (double(*)[1000 + 0][1200 + 0]) malloc(((1000 + 0) * (1200 + 0)) * sizeof(double));
  ;
  double(*B)[1200 + 0][1100 + 0];
  B = (double(*)[1200 + 0][1100 + 0]) malloc(((1200 + 0) * (1100 + 0)) * sizeof(double));
  ;

  init_array(ni, nj, nk, &alpha, &beta,
             *C,
             *A,
             *B);

  kernel_gemm(ni, nj, nk,
              alpha, beta,
              *C,
              *A,
              *B);

  if (argc > 42 )
  print_array(ni, nj, *C);

  free((void *)C);
  free((void *)A);
  free((void *)B);

  return 0;
}
