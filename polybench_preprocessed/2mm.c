/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* 2mm.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int ni, int nj, int nk, int nl,
                       double *alpha,
                       double *beta,
                       double A[800 + 0][1100 + 0],
                       double B[1100 + 0][900 + 0],
                       double C[900 + 0][1200 + 0],
                       double D[800 + 0][1200 + 0])
{
  // int i, j;

  *alpha = 1.5;
  *beta = 1.2;
  for (int i = 0; i < ni; i++)
    for (int j = 0; j < nk; j++)
      A[i][j] = (double)((i * j + 1) % ni) / ni;
  for (int i = 0; i < nk; i++)
    for (int j = 0; j < nj; j++)
      B[i][j] = (double)(i * (j + 1) % nj) / nj;
  for (int i = 0; i < nj; i++)
    for (int j = 0; j < nl; j++)
      C[i][j] = (double)((i * (j + 3) + 1) % nl) / nl;
  for (int i = 0; i < ni; i++)
    for (int j = 0; j < nl; j++)
      D[i][j] = (double)(i * (j + 2) % nk) / nk;
}

static void print_array(int ni, int nl,
                        double D[800 + 0][1200 + 0])
{
  int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "D");
  for (i = 0; i < ni; i++)
    for (j = 0; j < nl; j++)
    {
      if ((i * ni + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", D[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "D");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_2mm(int ni, int nj, int nk, int nl,
                       double alpha,
                       double beta,
                       double tmp[800 + 0][900 + 0],
                       double A[800 + 0][1100 + 0],
                       double B[1100 + 0][900 + 0],
                       double C[900 + 0][1200 + 0],
                       double D[800 + 0][1200 + 0])
{
  //  int i, j, k;

#pragma scop

  for (int i = 0; i < ni; i++)
    for (int j = 0; j < nj; j++)
    {
      tmp[i][j] = 0.0;
      for (int k = 0; k < nk; ++k)
        tmp[i][j] = tmp[i][j] + (alpha * A[i][k] * B[k][j]);
    }
  for (int i = 0; i < ni; i++)
    for (int j = 0; j < nl; j++)
    {
      D[i][j] = D[i][j] * beta;
      for (int k = 0; k < nj; ++k)
        D[i][j] = D[i][j] + (tmp[i][k] * C[k][j]);
    }
#pragma endscop
}

int main(int argc, char **argv)
{

  int ni = 800;
  int nj = 900;
  int nk = 1100;
  int nl = 1200;

  double alpha;
  double beta;
  double(*tmp)[800 + 0][900 + 0];
  tmp = (double(*)[800 + 0][900 + 0]) malloc((800 + 0) * (900 + 0) * sizeof(double));
  ;
  double(*A)[800 + 0][1100 + 0];
  A = (double(*)[800 + 0][1100 + 0]) malloc((800 + 0) * (1100 + 0) * sizeof(double));
  ;
  double(*B)[1100 + 0][900 + 0];
  B = (double(*)[1100 + 0][900 + 0]) malloc((1100 + 0) * (900 + 0) * sizeof(double));
  ;
  double(*C)[900 + 0][1200 + 0];
  C = (double(*)[900 + 0][1200 + 0]) malloc((900 + 0) * (1200 + 0) * sizeof(double));
  ;
  double(*D)[800 + 0][1200 + 0];
  D = (double(*)[800 + 0][1200 + 0]) malloc((800 + 0) * (1200 + 0) * sizeof(double));
  ;

  init_array(ni, nj, nk, nl, &alpha, &beta,
             *A,
             *B,
             *C,
             *D);

  kernel_2mm(ni, nj, nk, nl,
             alpha, beta,
             *tmp,
             *A,
             *B,
             *C,
             *D);

  if (argc > 42)
  print_array(ni, nl, *D);

  //argc=0;argv[0]=0;
  free((void *)tmp);
  ;
  free((void *)A);
  ;
  free((void *)B);
  ;
  free((void *)C);
  ;
  free((void *)D);
  ;

  return 0;
}
