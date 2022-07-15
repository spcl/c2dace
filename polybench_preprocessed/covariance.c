/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* covariance.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>

static void init_array(int m, int n,
                       double *float_n,
                       double data[1400 + 0][1200 + 0])
{
  //int i, j;

  *float_n = (double)n;

  for (int i = 0; i < n; i++)
    for (int j = 0; j < m; j++)
      data[i][j] = ((double)i * j) / 1200;
}

static void print_array(int m,
                        double cov[1200 + 0][1200 + 0])

{
  int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "cov");
  for (i = 0; i < m; i++)
    for (j = 0; j < m; j++)
    {
      if ((i * m + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", cov[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "cov");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_covariance(int m, int n,
                              double float_n,
                              double data[1400 + 0][1200 + 0],
                              double cov[1200 + 0][1200 + 0],
                              double mean[1200 + 0])
{
  // int i, j, k;

#pragma scop
  for (int j = 0; j < m; j++)
  {
    mean[j] = 0.0;
    for (int i = 0; i < n; i++)
      mean[j] = mean[j] + data[i][j];
    mean[j] = mean[j] / float_n;
  }

  for (int i = 0; i < n; i++)
    for (int j = 0; j < m; j++)
      data[i][j] = data[i][j] - mean[j];

  for (int i = 0; i < m; i++)
    for (int j = i; j < m; j++)
    {
      cov[i][j] = 0.0;
      for (int k = 0; k < n; k++)
        cov[i][j] = cov[i][j] + (data[k][i] * data[k][j]);
      cov[i][j] = cov[i][j] / (float_n - 1.0);
      cov[j][i] = cov[i][j];
    }
#pragma endscop
}

int main(int argc, char **argv)
{

  int n = 1400;
  int m = 1200;

  double float_n;
  double(*data)[1400 + 0][1200 + 0];
  data = (double(*)[1400 + 0][1200 + 0]) malloc((1400 + 0) * (1200 + 0) * sizeof(double));
  ;
  double(*cov)[1200 + 0][1200 + 0];
  cov = (double(*)[1200 + 0][1200 + 0]) malloc((1200 + 0) * (1200 + 0) * sizeof(double));
  ;
  double(*mean)[1200 + 0];
  mean = (double(*)[1200 + 0]) malloc((1200 + 0) * sizeof(double));
  ;

  init_array(m, n, &float_n, *data);

  kernel_covariance(m, n, float_n,
                    *data,
                    *cov,
                    *mean);

  if (argc > 42 )
  print_array(m, *cov);
  //argc=0;argv[0]=0;

  free((void *)data);
  ;
  free((void *)cov);
  ;
  free((void *)mean);
  ;

  return 0;
}
