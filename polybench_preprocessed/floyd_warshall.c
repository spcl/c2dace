/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* floyd-warshall.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>

static
void init_array (int n,
   int path[ 2800 + 0][2800 + 0])
{
  //int i, j;
    int tmp;
  for (int i = 0; i < n; i++){
    for (int j = 0; j < n; j++) {
      path[i][j] = i*j%7+1;
      tmp=(i+j)%13 == 0 || (i+j)%7==0 || (i+j)%11 == 0;
      if (tmp!=0){
         path[i][j] = 999;}
    }}
}




static
void print_array(int n,
   int path[ 2800 + 0][2800 + 0])

{
  int i, j;

  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "path");
  for (i = 0; i < n; i++){
    for (j = 0; j < n; j++) {
      if ((i * n + j) % 20 == 0) fprintf (stderr, "\n");
      fprintf (stderr, "%d ", path[i][j]);
    }}
  fprintf(stderr, "\nend   dump: %s\n", "path");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}




static
void kernel_floyd_warshall(int n,
      int path[ 2800 + 0][2800 + 0])
{
 // int i, j, k;
int tmp;
#pragma scop
  for (int k = 0; k < n; k++)
    {
      for(int i = 0; i < n; i++){
 for (int j = 0; j < n; j++){
 tmp=path[i][j] < path[i][k] + path[k][j];
    if (tmp!=0){
   path[i][j] =  path[i][j];}
   else{
     path[i][j] = path[i][k] + path[k][j];
   }}
}
    }
#pragma endscop

}


int main(int argc, char** argv)
{

  int n = 2800;


  int (*path)[2800 + 0][2800 + 0]; path = (int(*)[2800 + 0][2800 + 0])malloc ((2800 + 0) * (2800 + 0)* sizeof(int));;



  init_array (n, *path);



  kernel_floyd_warshall (n, *path);



  if (argc > 42 )
 print_array(n, *path);

//argc=0;argv[0]=0;
  free((void*)path);;

  return 0;
}
