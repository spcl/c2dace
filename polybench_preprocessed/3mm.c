/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* 3mm.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int ni, int nj, int nk, int nl, int nm,
                       double A[800 + 0][1000 + 0],
                       double B[1000 + 0][900 + 0],
                       double C[900 + 0][1200 + 0],
                       double D[1200 + 0][1100 + 0])
{
  //int i, j;

  for (int i = 0; i < ni; i++)
    for (int j = 0; j < nk; j++)
      A[i][j] = (double)((i * j + 1) % ni) / (5 * ni);
  for (int i = 0; i < nk; i++)
    for (int j = 0; j < nj; j++)
      B[i][j] = (double)((i * (j + 1) + 2) % nj) / (5 * nj);
  for (int i = 0; i < nj; i++)
    for (int j = 0; j < nm; j++)
      C[i][j] = (double)(i * (j + 3) % nl) / (5 * nl);
  for (int i = 0; i < nm; i++)
    for (int j = 0; j < nl; j++)
      D[i][j] = (double)((i * (j + 2) + 2) % nk) / (5 * nk);
}

static void print_array(int ni, int nl,
                        double G[800 + 0][1100 + 0])
{
  int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "G");
  for (i = 0; i < ni; i++)
    for (j = 0; j < nl; j++)
    {
      if ((i * ni + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", G[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "G");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_3mm(int ni, int nj, int nk, int nl, int nm,
                       double E[800 + 0][900 + 0],
                       double A[800 + 0][1000 + 0],
                       double B[1000 + 0][900 + 0],
                       double F[900 + 0][1100 + 0],
                       double C[900 + 0][1200 + 0],
                       double D[1200 + 0][1100 + 0],
                       double G[800 + 0][1100 + 0])
{
  //  int i, j, k;

#pragma scop

  for (int i = 0; i < ni; i++)
    for (int j = 0; j < nj; j++)
    {
      E[i][j] = 0.0;
      for (int k = 0; k < nk; ++k)
        E[i][j] = E[i][j] + (A[i][k] * B[k][j]);
    }

  for (int i = 0; i < nj; i++)
    for (int j = 0; j < nl; j++)
    {
      F[i][j] = 0.0;
      for (int k = 0; k < nm; ++k)
        F[i][j] = F[i][j] + (C[i][k] * D[k][j]);
    }

  for (int i = 0; i < ni; i++)
    for (int j = 0; j < nl; j++)
    {
      G[i][j] = 0.0;
      for (int k = 0; k < nj; ++k)
        G[i][j] = G[i][j] + (E[i][k] * F[k][j]);
    }
#pragma endscop
}

int main(int argc, char **argv)
{

  int ni = 800;
  int nj = 900;
  int nk = 1000;
  int nl = 1100;
  int nm = 1200;

  double(*E)[800 + 0][900 + 0];
  E = (double(*)[800 + 0][900 + 0]) malloc((800 + 0) * (900 + 0) * sizeof(double));
  ;
  double(*A)[800 + 0][1000 + 0];
  A = (double(*)[800 + 0][1000 + 0]) malloc((800 + 0) * (1000 + 0) * sizeof(double));
  ;
  double(*B)[1000 + 0][900 + 0];
  B = (double(*)[1000 + 0][900 + 0]) malloc((1000 + 0) * (900 + 0) * sizeof(double));
  ;
  double(*F)[900 + 0][1100 + 0];
  F = (double(*)[900 + 0][1100 + 0]) malloc((900 + 0) * (1100 + 0) * sizeof(double));
  ;
  double(*C)[900 + 0][1200 + 0];
  C = (double(*)[900 + 0][1200 + 0]) malloc((900 + 0) * (1200 + 0) * sizeof(double));
  ;
  double(*D)[1200 + 0][1100 + 0];
  D = (double(*)[1200 + 0][1100 + 0]) malloc((1200 + 0) * (1100 + 0) * sizeof(double));
  ;
  double(*G)[800 + 0][1100 + 0];
  G = (double(*)[800 + 0][1100 + 0]) malloc((800 + 0) * (1100 + 0) * sizeof(double));
  ;

  init_array(ni, nj, nk, nl, nm,
             *A,
             *B,
             *C,
             *D);

  kernel_3mm(ni, nj, nk, nl, nm,
             *E,
             *A,
             *B,
             *F,
             *C,
             *D,
             *G);

  if (argc > 2 )
  print_array(ni, nl, *G);

  //argc=0;argv[0]=0;
  free((void *)E);
  ;
  free((void *)A);
  ;
  free((void *)B);
  ;
  free((void *)F);
  ;
  free((void *)C);
  ;
  free((void *)D);
  ;
  free((void *)G);
  ;

  return 0;
}
