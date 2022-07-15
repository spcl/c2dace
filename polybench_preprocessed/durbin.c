/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* durbin.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>
static
void init_array (int n,
   double r[ 2000 + 0])
{
 // int i, j;

  for (int i = 0; i < n; i++)
    {
      r[i] = (n+1-i);
    }
}




static
void print_array(int n,
   double y[ 2000 + 0])

{
//  int i;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "y");
  for (int i = 0; i < n; i++) {
    if (i % 20 == 0) fprintf (stderr, "\n");
    fprintf (stderr, "%0.2lf ", y[i]);
  }
  fprintf(stderr, "\nend   dump: %s\n", "y");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}




static
void kernel_durbin(int n,
     double r[ 2000 + 0],
     double y[ 2000 + 0])
{
 double z[2000];
 double alpha;
 double beta;
 double sum;

 //int i,k;

#pragma scop
 y[0] = -r[0];
 beta = 1.0;
 alpha = -r[0];

 for (int k = 1; k < n; k++) {
   beta = (1-alpha*alpha)*beta;
   sum = 0.0;
   for (int i=0; i<k; i++) {
      sum =sum + (r[k-i-1]*y[i]);
   }
   alpha = - (r[k] + sum)/beta;

   for (int i=0; i<k; i++) {
      z[i] = y[i] + alpha*y[k-i-1];
   }
   for (int i=0; i<k; i++) {
     y[i] = z[i];
   }
   y[k] = alpha;
 }
#pragma endscop

}


int main(int argc, char** argv)
{

  int n = 2000;


  double (*r)[2000 + 0]; r = (double(*)[2000 + 0])malloc ((2000 + 0)* sizeof(double));;
  double (*y)[2000 + 0]; y = (double(*)[2000 + 0])malloc ((2000 + 0)* sizeof(double));;



  init_array (n, *r);



  kernel_durbin (n,
   *r,
   *y);



if (argc > 42 ) 
print_array(n, *y);

//argc=0;argv[0]=0;
  free((void*)r);;
  free((void*)y);;

  return 0;
}
