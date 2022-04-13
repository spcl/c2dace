/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* adi.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int n,
                       double u[1000 + 0][1000 + 0])
{
  //  int i, j;

  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      u[i][j] = (double)(i + n - j) / n;
    }
}

static void print_array(int n,
                        double u[1000 + 0][1000 + 0])

{
  // int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "u");
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
    {
      if ((i * n + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", u[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "u");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_adi(int tsteps, int n,
                       double u[1000 + 0][1000 + 0],
                       double v[1000 + 0][1000 + 0],
                       double p[1000 + 0][1000 + 0],
                       double q[1000 + 0][1000 + 0])
{
  // int t, i, j;
  double DX, DY, DT;
  double B1, B2;
  double mul1, mul2;
  double a, b, c, d, e, f;

#pragma scop

  DX = 1.0 / (double)n;
  DY = 1.0 / (double)n;
  DT = 1.0 / (double)tsteps;
  B1 = 2.0;
  B2 = 1.0;
  mul1 = B1 * DT / (DX * DX);
  mul2 = B2 * DT / (DY * DY);

  a = -mul1 / 2.0;
  b = 1.0 + mul1;
  c = a;
  d = -mul2 / 2.0;
  e = 1.0 + mul2;
  f = d;

  for (int t = 1; t <= tsteps; t++)
  {

    for (int i = 1; i < n - 1; i++)
    {
      v[0][i] = 1.0;
      p[i][0] = 0.0;
      q[i][0] = v[0][i];
      for (int j = 1; j < n - 1; j++)
      {
        p[i][j] = -c / (a * p[i][j - 1] + b);
        q[i][j] = (-d * u[j][i - 1] + (1.0 + 2.0 * d) * u[j][i] - f * u[j][i + 1] - a * q[i][j - 1]) / (a * p[i][j - 1] + b);
      }

      v[n - 1][i] = 1.0;
      for (int j = n - 2; j >= 1; j--)
      {
        v[j][i] = p[i][j] * v[j + 1][i] + q[i][j];
      }
    }

    for (int i = 1; i < n - 1; i++)
    {
      u[i][0] = 1.0;
      p[i][0] = 0.0;
      q[i][0] = u[i][0];
      for (int j = 1; j < n - 1; j++)
      {
        p[i][j] = -f / (d * p[i][j - 1] + e);
        q[i][j] = (-a * v[i - 1][j] + (1.0 + 2.0 * a) * v[i][j] - c * v[i + 1][j] - d * q[i][j - 1]) / (d * p[i][j - 1] + e);
      }
      u[i][n - 1] = 1.0;
      for (int j = n - 2; j >= 1; j--)
      {
        u[i][j] = p[i][j] * u[i][j + 1] + q[i][j];
      }
    }
  }
#pragma endscop
}

int main(int argc, char **argv)
{

  int n = 1000;
  int tsteps = 500;

  double(*u)[1000 + 0][1000 + 0];
  u = (double(*)[1000 + 0][1000 + 0]) malloc(((1000 + 0) * (1000 + 0)) * sizeof(double));
  ;
  double(*v)[1000 + 0][1000 + 0];
  v = (double(*)[1000 + 0][1000 + 0]) malloc(((1000 + 0) * (1000 + 0)) * sizeof(double));
  ;
  double(*p)[1000 + 0][1000 + 0];
  p = (double(*)[1000 + 0][1000 + 0]) malloc(((1000 + 0) * (1000 + 0)) * sizeof(double));
  ;
  double(*q)[1000 + 0][1000 + 0];
  q = (double(*)[1000 + 0][1000 + 0]) malloc(((1000 + 0) * (1000 + 0)) * sizeof(double));
  ;

  init_array(n, *u);

  kernel_adi(tsteps, n, *u, *v, *p, *q);

  if (argc > 42 )
  print_array(n, *u);

  //argc=0;argv[0]=0;
  free((void *)u);
  ;
  free((void *)v);
  ;
  free((void *)p);
  ;
  free((void *)q);
  ;

  return 0;
}
