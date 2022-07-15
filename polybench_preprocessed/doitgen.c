/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* doitgen.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>

static void init_array(int nr, int nq, int np,
                       double A[150 + 0][140 + 0][160 + 0],
                       double C4[160 + 0][160 + 0])
{
  //int i, j, k;

  for (int i = 0; i < nr; i++)
    for (int j = 0; j < nq; j++)
      for (int k = 0; k < np; k++)
        A[i][j][k] = (double)((i * j + k) % np) / np;
  for (int i = 0; i < np; i++)
    for (int j = 0; j < np; j++)
      C4[i][j] = (double)(i * j % np) / np;
}

static void print_array(int nr, int nq, int np,
                        double A[150 + 0][140 + 0][160 + 0])
{
  int i, j, k;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "A");
  for (i = 0; i < nr; i++)
    for (j = 0; j < nq; j++)
      for (k = 0; k < np; k++)
      {
        if ((i * nq * np + j * np + k) % 20 == 0)
          fprintf(stderr, "\n");
        fprintf(stderr, "%0.2lf ", A[i][j][k]);
      }
  fprintf(stderr, "\nend   dump: %s\n", "A");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

void kernel_doitgen(int nr, int nq, int np,
                    double A[150 + 0][140 + 0][160 + 0],
                    double C4[160 + 0][160 + 0],
                    double sum[160 + 0])
{
  // int r, q, p, s;

#pragma scop
  for (int r = 0; r < nr; r++)
    for (int q = 0; q < nq; q++)
    {
      for (int p = 0; p < np; p++)
      {
        sum[p] = 0.0;
        for (int s = 0; s < np; s++)
          sum[p] = sum[p] + A[r][q][s] * C4[s][p];
      }
      for (int p = 0; p < np; p++)
        A[r][q][p] = sum[p];
    }
#pragma endscop
}

int main(int argc, char **argv)
{

  int nr = 150;
  int nq = 140;
  int np = 160;

  double(*A)[150 + 0][140 + 0][160 + 0];
  A = (double(*)[150 + 0][140 + 0][160 + 0]) malloc((150 + 0) * (140 + 0) * (160 + 0) * sizeof(double));
  ;
  double(*sum)[160 + 0];
  sum = (double(*)[160 + 0]) malloc((160 + 0) * sizeof(double));
  ;
  double(*C4)[160 + 0][160 + 0];
  C4 = (double(*)[160 + 0][160 + 0]) malloc((160 + 0) * (160 + 0) * sizeof(double));
  ;

  init_array(nr, nq, np,
             *A,
             *C4);

  kernel_doitgen(nr, nq, np,
                 *A,
                 *C4,
                 *sum);

  if (argc > 42 )
  print_array(nr, nq, np, *A);
  //argc=0;argv[0]=0;

  free((void *)A);
  ;
  free((void *)sum);
  ;
  free((void *)C4);
  ;

  return 0;
}
