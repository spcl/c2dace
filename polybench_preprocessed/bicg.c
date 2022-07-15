/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* bicg.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int m, int n,
                       double A[2100 + 0][1900 + 0],
                       double r[2100 + 0],
                       double p[1900 + 0])
{
  //int i, j;

  for (int i = 0; i < m; i++)
    p[i] = (double)(i % m) / m;
  for (int i = 0; i < n; i++)
  {
    r[i] = (double)(i % n) / n;
    for (int j = 0; j < m; j++)
      A[i][j] = (double)(i * (j + 1) % n) / n;
  }
}

static void print_array(int m, int n,
                        double s[1900 + 0],
                        double q[2100 + 0])

{
  int i;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "s");
  for (i = 0; i < m; i++)
  {
    if (i % 20 == 0)
      fprintf(stderr, "\n");
    fprintf(stderr, "%0.2lf ", s[i]);
  }
  fprintf(stderr, "\nend   dump: %s\n", "s");
  fprintf(stderr, "begin dump: %s", "q");
  for (i = 0; i < n; i++)
  {
    if (i % 20 == 0)
      fprintf(stderr, "\n");
    fprintf(stderr, "%0.2lf ", q[i]);
  }
  fprintf(stderr, "\nend   dump: %s\n", "q");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_bicg(int m, int n,
                        double A[2100 + 0][1900 + 0],
                        double s[1900 + 0],
                        double q[2100 + 0],
                        double p[1900 + 0],
                        double r[2100 + 0])
{
  // int i, j;

#pragma scop
  for (int i = 0; i < m; i++)
    s[i] = 0;
  for (int i = 0; i < n; i++)
  {
    q[i] = 0.0;
    for (int j = 0; j < m; j++)
    {
      s[j] = s[j] + r[i] * A[i][j];
      q[i] = q[i] + A[i][j] * p[j];
    }
  }
#pragma endscop
}

int main(int argc, char **argv)
{

  int n = 2100;
  int m = 1900;

  double(*A)[2100 + 0][1900 + 0];
  A = (double(*)[2100 + 0][1900 + 0]) malloc((2100 + 0) * (1900 + 0) * sizeof(double));
  ;
  double(*s)[1900 + 0];
  s = (double(*)[1900 + 0]) malloc((1900 + 0) * sizeof(double));
  ;
  double(*q)[2100 + 0];
  q = (double(*)[2100 + 0]) malloc((2100 + 0) * sizeof(double));
  ;
  double(*p)[1900 + 0];
  p = (double(*)[1900 + 0]) malloc((1900 + 0) * sizeof(double));
  ;
  double(*r)[2100 + 0];
  r = (double(*)[2100 + 0]) malloc((2100 + 0) * sizeof(double));
  ;

  init_array(m, n,
             *A,
             *r,
             *p);

  kernel_bicg(m, n,
              *A,
              *s,
              *q,
              *p,
              *r);

  if (argc > 42 )
  print_array(m, n, *s, *q);
  //argc=0;argv[0]=0;

  free((void *)A);
  ;
  free((void *)s);
  ;
  free((void *)q);
  ;
  free((void *)p);
  ;
  free((void *)r);
  ;

  return 0;
}
