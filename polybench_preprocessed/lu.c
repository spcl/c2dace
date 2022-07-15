/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* lu.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>

static void init_array(int n,
                       double A[2000 + 0][2000 + 0])
{

  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j <= i; j++)
      A[i][j] = (double)(-j % n) / n + 1;
    for (int j = i + 1; j < n; j++)
    {
      A[i][j] = 0;
    }
    A[i][i] = 1;
  }

  double B[2000 + 0][2000 + 0]; //B = (double(*)[2000 + 0][2000 + 0])malloc ((2000 + 0) * (2000 + 0)* sizeof(double));;
  for (int r = 0; r < n; ++r)
    for (int s = 0; s < n; ++s)
      B[r][s] = 0;
  for (int t = 0; t < n; ++t)
    for (int r = 0; r < n; ++r)
      for (int s = 0; s < n; ++s)
        B[r][s] = B[r][s] + (A[r][t] * A[s][t]);
  for (int r = 0; r < n; ++r)
    for (int s = 0; s < n; ++s)
      A[r][s] = B[r][s];
  //free((void*)B);;
}

static void print_array(int n,
                        double A[2000 + 0][2000 + 0])

{
  int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "A");
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
    {
      if ((i * n + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", A[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "A");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_lu(int n,
                      double A[2000 + 0][2000 + 0])
{

#pragma scop
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < i; j++)
    {
      for (int k = 0; k < j; k++)
      {
        A[i][j] = A[i][j] - (A[i][k] * A[k][j]);
      }
      A[i][j] = A[i][j] / A[j][j];
    }
    for (int j = i; j < n; j++)
    {
      for (int k = 0; k < i; k++)
      {
        A[i][j] = A[i][j] - (A[i][k] * A[k][j]);
      }
    }
  }
#pragma endscop
}

int main(int argc, char **argv)
{

  int n = 2000;

  double(*A)[2000 + 0][2000 + 0];
  A = (double(*)[2000 + 0][2000 + 0]) malloc((2000 + 0) * (2000 + 0) * sizeof(double));
  ;

  init_array(n, *A);

  kernel_lu(n, *A);

  //argc = 0;
  //argv[0] = 0;
  if (argc > 42)
  print_array(n, *A);

  free((void *)A);
  ;

  return 0;
}