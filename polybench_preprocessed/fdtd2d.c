/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* fdtd-2d.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int tmax,
                       int nx,
                       int ny,
                       double ex[1000 + 0][1200 + 0],
                       double ey[1000 + 0][1200 + 0],
                       double hz[1000 + 0][1200 + 0],
                       double _fict_[500 + 0])
{
  //int i, j;

  for (int i = 0; i < tmax; i++)
    _fict_[i] = (double)i;
  for (int i = 0; i < nx; i++)
    for (int j = 0; j < ny; j++)
    {
      ex[i][j] = ((double)i * (j + 1)) / nx;
      ey[i][j] = ((double)i * (j + 2)) / ny;
      hz[i][j] = ((double)i * (j + 3)) / nx;
    }
}

static void print_array(int nx,
                        int ny,
                        double ex[1000 + 0][1200 + 0],
                        double ey[1000 + 0][1200 + 0],
                        double hz[1000 + 0][1200 + 0])
{
  int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "ex");
  for (i = 0; i < nx; i++)
    for (j = 0; j < ny; j++)
    {
      if ((i * nx + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", ex[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "ex");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");

  fprintf(stderr, "begin dump: %s", "ey");
  for (i = 0; i < nx; i++)
    for (j = 0; j < ny; j++)
    {
      if ((i * nx + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", ey[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "ey");

  fprintf(stderr, "begin dump: %s", "hz");
  for (i = 0; i < nx; i++)
    for (j = 0; j < ny; j++)
    {
      if ((i * nx + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", hz[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "hz");
}

static void kernel_fdtd2d(int tmax,
                           int nx,
                           int ny,
                           double ex[1000 + 0][1200 + 0],
                           double ey[1000 + 0][1200 + 0],
                           double hz[1000 + 0][1200 + 0],
                           double _fict_[500 + 0])
{
  // int t, i, j;

#pragma scop

  for (int t = 0; t < tmax; t++)
  {
    for (int j = 0; j < ny; j++)
      ey[0][j] = _fict_[t];
    for (int i = 1; i < nx; i++)
      for (int j = 0; j < ny; j++)
        ey[i][j] = ey[i][j] - 0.5 * (hz[i][j] - hz[i - 1][j]);
    for (int i = 0; i < nx; i++)
      for (int j = 1; j < ny; j++)
        ex[i][j] = ex[i][j] - 0.5 * (hz[i][j] - hz[i][j - 1]);
    for (int i = 0; i < nx - 1; i++)
      for (int j = 0; j < ny - 1; j++)
        hz[i][j] = hz[i][j] - 0.7 * (ex[i][j + 1] - ex[i][j] +
                                     ey[i + 1][j] - ey[i][j]);
  }

#pragma endscop
}

int main(int argc, char **argv)
{

  int tmax = 500;
  int nx = 1000;
  int ny = 1200;

  double(*ex)[1000 + 0][1200 + 0];
  ex = (double(*)[1000 + 0][1200 + 0]) malloc(((1000 + 0) * (1200 + 0)) * sizeof(double));
  ;
  double(*ey)[1000 + 0][1200 + 0];
  ey = (double(*)[1000 + 0][1200 + 0]) malloc(((1000 + 0) * (1200 + 0)) * sizeof(double));
  ;
  double(*hz)[1000 + 0][1200 + 0];
  hz = (double(*)[1000 + 0][1200 + 0]) malloc(((1000 + 0) * (1200 + 0)) * sizeof(double));
  ;
  double(*_fict_)[500 + 0];
  _fict_ = (double(*)[500 + 0]) malloc((500 + 0) * sizeof(double));
  ;

  init_array(tmax, nx, ny,
             *ex,
             *ey,
             *hz,
             *_fict_);

  kernel_fdtd2d(tmax, nx, ny,
                 *ex,
                 *ey,
                 *hz,
                 *_fict_);

  if (argc > 42)
  print_array(nx, ny, *ex, *ey, *hz)  ;
  //argc=0;argv[0]=0;
  //printf("GR: %f",hz[999][1199]);
  free((void *)ex);
  ;
  free((void *)ey);
  ;
  free((void *)hz);
  ;
  free((void *)_fict_);
  ;

  return 0;
}
