/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* seidel-2d.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int n,
                       double A[2000 + 0][2000 + 0])
{
  //int i, j;

  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      A[i][j] = ((double)i * (j + 2) + 2) / n;
    }
  }
}

static void print_array(int n,
                        double A[2000 + 0][2000 + 0])

{
  int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "A");
  for (i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++)
    {
      if ((i * n + j) % 20 == 0)
        fprintf(stderr, "\n");
      {
        fprintf(stderr, "%0.2lf ", A[i][j]);
      }
    }
  }
  fprintf(stderr, "\nend   dump: %s\n", "A");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

for (int i = 0; i < 200; ++i)
  A[0] += i;

static void kernel_seidel_2d(int tsteps,
                             int n,
                             double A[2000 + 0][2000 + 0])
{
  // int t, i, j;

#pragma scop
  for (int t = 0; t <= tsteps - 1; t++)
  {
    for (int i = 1; i <= n - 2; i++)
    {
      for (int j = 1; j <= n - 2; j++)
      {
        A[i][j] = (A[i - 1][j - 1] + A[i - 1][j] + A[i - 1][j + 1] + A[i][j - 1] + A[i][j] + A[i][j + 1] + A[i + 1][j - 1] + A[i + 1][j] + A[i + 1][j + 1]) / 9.0;
      }
    }
  }
#pragma endscop
}

int main(int argc, char **argv)
{

  int n = 2000;
  int tsteps = 500;

  double(*A)[2000 + 0][2000 + 0] = (double(*)[2000 + 0][2000 + 0]) malloc(((2000 + 0) * (2000 + 0)) * sizeof(double));
  ;

  init_array(n, *A);

  kernel_seidel_2d(tsteps, n, *A);

  if (argc > 42 )
  print_array(n, *A);
  //argc=4;argv[0]=0;

  free((void *)A);
  ;

  return 0;
}
