/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* deriche.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static void init_array(int w, int h, float *alpha,
                       float imgIn[4096 + 0][2160 + 0])
{
  //int i, j;

  *alpha = 0.25;

  for (int i = 0; i < w; i++)
    for (int j = 0; j < h; j++)
      imgIn[i][j] = (float)((313 * i + 991 * j) % 65536) / 65535.0f;
}

static void print_array(int w, int h,
                        float imgOut[4096 + 0][2160 + 0])

{
  int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "imgOut");
  for (i = 0; i < w; i++)
    for (j = 0; j < h; j++)
    {
      if ((i * h + j) % 20 == 0)
        fprintf(stderr, "\n");
      fprintf(stderr, "%0.2f ", imgOut[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "imgOut");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}

static void kernel_deriche(int w, int h, float alpha,
                           float imgIn[4096 + 0][2160 + 0],
                           float imgOut[4096 + 0][2160 + 0],
                           float y1[4096 + 0][2160 + 0],
                           float y2[4096 + 0][2160 + 0])
{
  //int i,j;
  float xm1, tm1, ym1, ym2;
  float xp1, xp2;
  float tp1, tp2;
  float yp1, yp2;

  float k;
  float a1, a2, a3, a4, a5, a6, a7, a8;
  float b1, b2, c1, c2;

#pragma scop
  k = (1.0f - expf(-alpha)) * (1.0f - expf(-alpha)) / (1.0f + 2.0f * alpha * expf(-alpha) - expf(2.0f * alpha));
  a1 = a5 = k;
  a2 = a6 = k * expf(-alpha) * (alpha - 1.0f);
  a3 = a7 = k * expf(-alpha) * (alpha + 1.0f);
  a4 = a8 = -k * expf(-2.0f * alpha);
  b1 = powf(2.0f, -alpha);
  b2 = -expf(-2.0f * alpha);
  c1 = c2 = 1;

  for (int i = 0; i < w; i++)
  {
    ym1 = 0.0f;
    ym2 = 0.0f;
    xm1 = 0.0f;
    for (int j = 0; j < h; j++)
    {
      y1[i][j] = a1 * imgIn[i][j] + a2 * xm1 + b1 * ym1 + b2 * ym2;
      xm1 = imgIn[i][j];
      ym2 = ym1;
      ym1 = y1[i][j];
    }
  }

  for (int ii = 0; ii < w; ii++)
  {
    yp1 = 0.0f;
    yp2 = 0.0f;
    xp1 = 0.0f;
    xp2 = 0.0f;
    for (int jj = h - 1; jj >= 0; jj--)
    {
      y2[ii][jj] = a3 * xp1 + a4 * xp2 + b1 * yp1 + b2 * yp2;
      xp2 = xp1;
      xp1 = imgIn[ii][jj];
      yp2 = yp1;
      yp1 = y2[ii][jj];
    }
  }

  for (int iiii = 0; iiii < w; iiii++)
    for (int jjjj = 0; jjjj < h; jjjj++)
    {
      imgOut[iiii][jjjj] = c1 * (y1[iiii][jjjj] + y2[iiii][jjjj]);
    }

  for (int j5 = 0; j5 < h; j5++)
  {
    tm1 = 0.0f;
    ym1 = 0.0f;
    ym2 = 0.0f;
    for (int i5 = 0; i5 < w; i5++)
    {
      y1[i5][j5] = a5 * imgOut[i5][j5] + a6 * tm1 + b1 * ym1 + b2 * ym2;
      tm1 = imgOut[i5][j5];
      ym2 = ym1;
      ym1 = y1[i5][j5];
    }
  }

  for (int j6 = 0; j6 < h; j6++)
  {
    tp1 = 0.0f;
    tp2 = 0.0f;
    yp1 = 0.0f;
    yp2 = 0.0f;
    for (int i6 = w - 1; i6 >= 0; i6--)
    {
      y2[i6][j6] = a7 * tp1 + a8 * tp2 + b1 * yp1 + b2 * yp2;
      tp2 = tp1;
      tp1 = imgOut[i6][j6];
      yp2 = yp1;
      yp1 = y2[i6][j6];
    }
  }

  for (int i7 = 0; i7 < w; i7++)
    for (int j7 = 0; j7 < h; j7++)
      imgOut[i7][j7] = c2 * (y1[i7][j7] + y2[i7][j7]);

#pragma endscop
}

int main(int argc, char **argv)
{

  int w = 4096;
  int h = 2160;

  float alpha;
  float(*imgIn)[4096 + 0][2160 + 0];
  imgIn = (float(*)[4096 + 0][2160 + 0]) malloc(((4096 + 0) * (2160 + 0)) * sizeof(float));
  ;
  float(*imgOut)[4096 + 0][2160 + 0];
  imgOut = (float(*)[4096 + 0][2160 + 0]) malloc(((4096 + 0) * (2160 + 0)) * sizeof(float));
  ;
  float(*y1)[4096 + 0][2160 + 0];
  y1 = (float(*)[4096 + 0][2160 + 0]) malloc(((4096 + 0) * (2160 + 0)) * sizeof(float));
  ;
  float(*y2)[4096 + 0][2160 + 0];
  y2 = (float(*)[4096 + 0][2160 + 0]) malloc(((4096 + 0) * (2160 + 0)) * sizeof(float));
  ;

  init_array(w, h, &alpha, *imgIn);

  kernel_deriche(w, h, alpha, *imgIn, *imgOut, *y1, *y2);

  if (argc > 42)
  print_array(w, h, *imgOut);

  free((void *)imgIn);
  ;
  free((void *)imgOut);
  ;
  free((void *)y1);
  ;
  free((void *)y2);
  ;
  //argc=0;argv[0]=0;
  return 0;
}
