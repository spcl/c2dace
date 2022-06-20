#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>


struct HPC_Sparse_Matrix_STRUCT {
  double ** ptr_to_vals_in_row;
  int ** ptr_to_inds_in_row;

  double *list_of_vals;   //needed for cleaning up memory
  int *list_of_inds;      //needed for cleaning up memory
};


typedef struct HPC_Sparse_Matrix_STRUCT HPC_Sparse_Matrix;

int main(int argc, char *argv[])
{

  HPC_Sparse_Matrix* A = malloc(sizeof(HPC_Sparse_Matrix)); // Allocate matrix struct and fill it
  int nx = 13;
  int ny = 42;
  int nz = 23;

  int local_nrow = nx*ny*nz; // This is the size of our subblock

  int local_nnz = 27*local_nrow; // Approximately 27 nonzeros per row (except for boundary nodes)

  int total_nrow = local_nrow; // Total number of grid points in mesh
  long long total_nnz = 27* (long long) total_nrow; // Approximately 27 nonzeros per row (except for boundary nodes)

  // Allocate arrays that are of length local_nrow
  A->ptr_to_vals_in_row = malloc(local_nrow * sizeof(double*));
  A->ptr_to_inds_in_row = malloc(local_nrow * sizeof(int*));
  (A->ptr_to_vals_in_row)[0] = malloc(local_nnz * sizeof(double));
  (A->ptr_to_inds_in_row)[0] = malloc(local_nnz * sizeof(int));

  // Allocate arrays that are of length local_nnz
  A->list_of_vals = malloc(local_nnz * sizeof(double));
  A->list_of_inds = malloc(local_nnz * sizeof(int));

  double * curvalptr = A->list_of_vals;
  int * curindptr = A->list_of_inds;

  int curlocalrow = 12;
  (A->ptr_to_vals_in_row)[curlocalrow] = curvalptr;
  (A->ptr_to_inds_in_row)[curlocalrow] = curindptr;
  curvalptr[0] = 27.0;

  return 0 ;
} 