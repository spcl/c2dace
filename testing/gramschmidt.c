/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* gramschmidt.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>
static void init_array(int m, int n,
                       double A[1000 + 0][1200 + 0],
                       double R[1200 + 0][1200 + 0],
                       double Q[1000 + 0][1200 + 0])
{
  //int i, j;

  for (int i = 0; i < m; i++)
    for (int j = 0; j < n; j++)
    {
      A[i][j] = (((double)((i * j) % m) / m) * 100) + 10;
      Q[i][j] = 0.0;
    }
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      R[i][j] = 0.0;
}

static void print_array(int m, int n,
                        double A[1000 + 0][1200 + 0],
                        double R[1200 + 0][1200 + 0],
                        double Q[1000 + 0][1200 + 0])
{
  int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "R");
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
    {
      if ((i * n + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", R[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "R");

  fprintf(stderr, "begin dump: %s", "Q");
  for (i = 0; i < m; i++)
    for (j = 0; j < n; j++)
    {
      if ((i * n + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", Q[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "Q");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_gramschmidt(int m, int n,
                               double A[1000 + 0][1200 + 0],
                               double R[1200 + 0][1200 + 0],
                               double Q[1000 + 0][1200 + 0])
{
  // int i, j, k;

  double nrm;

#pragma scop
  for (int k = 0; k < n; k++)
  {
    nrm = 0.0;
    for (int i = 0; i < m; i++)
      nrm = nrm + (A[i][k] * A[i][k]);
    R[k][k] = sqrt(nrm);
    for (int i = 0; i < m; i++)
      Q[i][k] = A[i][k] / R[k][k];
    for (int j = k + 1; j < n; j++)
    {
      R[k][j] = 0.0;
      for (int i = 0; i < m; i++)
        R[k][j] = R[k][j] + (Q[i][k] * A[i][j]);
      for (int i = 0; i < m; i++)
        A[i][j] = A[i][j] - Q[i][k] * R[k][j];
    }
  }
#pragma endscop
}

int main(int argc, char **argv)
{

  int m = 1000;
  int n = 1200;

  double(*A)[1000 + 0][1200 + 0];
  A = (double(*)[1000 + 0][1200 + 0]) malloc((1000 + 0) * (1200 + 0) * sizeof(double));
  ;
  double(*R)[1200 + 0][1200 + 0];
  R = (double(*)[1200 + 0][1200 + 0]) malloc((1200 + 0) * (1200 + 0) * sizeof(double));
  ;
  double(*Q)[1000 + 0][1200 + 0];
  Q = (double(*)[1000 + 0][1200 + 0]) malloc((1000 + 0) * (1200 + 0) * sizeof(double));
  ;

  init_array(m, n,
             *A,
             *R,
             *Q);

  kernel_gramschmidt(m, n,
                     *A,
                     *R,
                     *Q);

  if (argc > 42 )
  print_array(m, n, *A, *R, *Q);
  //argc=0;argv[0]=0;

  free((void *)A);
  ;
  free((void *)R);
  ;
  free((void *)Q);
  ;

  return 0;
}
