#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>


struct HPC_Sparse_Matrix_STRUCT {
  int local_nrow;
};


typedef struct HPC_Sparse_Matrix_STRUCT HPC_Sparse_Matrix;

void waxpby (double* x, double* w)
{  
  for (int i=0; i<3; i++) {
    w[i] = x[i];
  }
}

void HPCCG (HPC_Sparse_Matrix * A, double* x)

{
  double ncol = 11;

  //double r[nrow];
  //double p[ncol]; // In parallel case, A is rectangular
  //double Ap[nrow];
  double* p = malloc(ncol * sizeof(double)); // In parallel case, A is rectangular

  // p is of length ncols, copy x to p for sparse MV operation
  waxpby(x, p);
}

int main(int argc, char *argv[])
{
  int local_nrow = 13;

  HPC_Sparse_Matrix* A = malloc(sizeof(HPC_Sparse_Matrix)); // Allocate matrix struct and fill it
  double *x = malloc(local_nrow * sizeof(double));
  HPCCG(A, x);

  return 0 ;
} 