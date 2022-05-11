#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>


struct HPC_Sparse_Matrix_STRUCT {
  int local_nrow;
  int local_ncol;  // Must be defined in make_local_matrix
  int  * nnz_in_row;
  double ** ptr_to_vals_in_row;
};


typedef struct HPC_Sparse_Matrix_STRUCT HPC_Sparse_Matrix;

void HPC_sparsemv( HPC_Sparse_Matrix *A, double* x, double* y)
{

  int nrow = A->local_nrow;

  for (int i=0; i< nrow; i++)
    {
      double* cur_vals = A->ptr_to_vals_in_row[i];
      int cur_nnz = (A->nnz_in_row)[i];
    }
}


void HPCCG (HPC_Sparse_Matrix * A, double* b, double* x, int max_iter, double tolerance, int* niters, double* normr)

{
  double nrow = A->local_nrow;
  double ncol = A->local_ncol;

  double* p = malloc(ncol * sizeof(double)); // In parallel case, A is rectangular
  double* Ap = malloc(nrow * sizeof(double));

  HPC_sparsemv(A, p, Ap);
}

int main(int argc, char *argv[])
{

  HPC_Sparse_Matrix* A = malloc(sizeof(HPC_Sparse_Matrix)); // Allocate matrix struct and fill it
  int nx,ny,nz;

  nx = 13;
  ny = 42;
  nz = 23;

  double local_nrow = nx*ny*nz; // This is the size of our subblock

  double local_nnz = 27*local_nrow; // Approximately 27 nonzeros per row (except for boundary nodes)

  // Allocate arrays that are of length local_nrow
  A->nnz_in_row = malloc(local_nrow * sizeof(int));
  A->ptr_to_vals_in_row = malloc(local_nrow * sizeof(double*));
  (A->ptr_to_vals_in_row)[0] = malloc(local_nnz * sizeof(double));

  double *x = malloc(local_nrow * sizeof(double));
  double *b = malloc(local_nrow * sizeof(double));

  int* niters = malloc(sizeof(int));
  double* normr = malloc(sizeof(double));

  int max_iter = 150;
  double tolerance = 0.0; // Set tolerance to zero to make all runs do max_iter iterations
  HPCCG(A, b, x, max_iter, tolerance, niters, normr);

  return 0 ;
} 