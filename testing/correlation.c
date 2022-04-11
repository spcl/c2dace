/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* correlation.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int m,
                       int n,
                       double *float_n,
                       double data[1400 + 0][1200 + 0])
{

  *float_n = (double)1400;

  for (int i = 0; i < n; i++)
    for (int j = 0; j < m; j++)
      data[i][j] = (double)(i * j) / 1200 + i;
}

static void print_array(int m,
                        double corr[1200 + 0][1200 + 0])

{
  //int i, j;
  //int aux=12;
  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "corr");
  for (int i = 0; i < m; i++)
    for (int j = 0; j < m; j++)
    {
      if ((i * m + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", corr[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "corr");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_correlation(int m, int n,
                               double float_n,
                               double data[1400 + 0][1200 + 0],
                               double corr[1200 + 0][1200 + 0],
                               double mean[1200 + 0],
                               double stddev[1200 + 0])
{

  double eps = 0.1;

#pragma scop
  for (int j = 0; j < m; j++)
  {
    mean[j] = 0.0;
    for (int i = 0; i < n; i++)
      mean[j] = mean[j] + data[i][j];
    mean[j] = mean[j] / float_n;
  }

  for (int j = 0; j < m; j++)
  {
    stddev[j] = 0.0;
    for (int i = 0; i < n; i++)
      stddev[j] = stddev[j] + ((data[i][j] - mean[j]) * (data[i][j] - mean[j]));
    stddev[j] = stddev[j] / float_n;
    stddev[j] = sqrt(stddev[j]);

    if (stddev[j] <= eps)
    {
      stddev[j] = 1.0;
    }
    else
    {
      stddev[j] = stddev[j];
    }
  }

  for (int i = 0; i < n; i++)
    for (int j = 0; j < m; j++)
    {
      data[i][j] = data[i][j] - mean[j];
      data[i][j] = data[i][j] / (sqrt(float_n) * stddev[j]);
    }

  for (int i = 0; i < m - 1; i++)
  {
    corr[i][i] = 1.0;
    for (int j = i + 1; j < m; j++)
    {
      corr[i][j] = 0.0;
      for (int k = 0; k < n; k++)
        corr[i][j] = corr[i][j] + (data[k][i] * data[k][j]);
      corr[j][i] = corr[i][j];
    }
  }
  corr[m - 1][m - 1] = 1.0;
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
  double(*corr)[1200 + 0][1200 + 0];
  corr = (double(*)[1200 + 0][1200 + 0]) malloc((1200 + 0) * (1200 + 0) * sizeof(double));
  ;
  double(*mean)[1200 + 0];
  mean = (double(*)[1200 + 0]) malloc((1200 + 0) * sizeof(double));
  ;
  double(*stddev)[1200 + 0];
  stddev = (double(*)[1200 + 0]) malloc((1200 + 0) * sizeof(double));
  ;

  init_array(m, n, &float_n, *data);

  kernel_correlation(m, n, float_n,
                     *data,
                     *corr,
                     *mean,
                     *stddev);

  //argv[0]=0;
  if (argc > 42)
  print_array(m, *corr);
  //for (int i=0;i<30;i++)
  //for (int j=0;j<30;j++)
  //aux=(aux+printf("%f\n",(*corr)[600+i][600+j]);
  free((void *)data);
  ;
  free((void *)corr);
  ;
  free((void *)mean);
  ;
  free((void *)stddev);
  ;

  return 0;
}
