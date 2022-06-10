#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>


struct HPC_Sparse_Matrix_STRUCT {
  char   *title;
  int start_row;
  int stop_row;
  int total_nrow;
  long long total_nnz;
  int local_nrow;
  int local_ncol;  // Must be defined in make_local_matrix
  int local_nnz;
  int  * nnz_in_row;
  double ** ptr_to_vals_in_row;
  int ** ptr_to_inds_in_row;
};


typedef struct HPC_Sparse_Matrix_STRUCT HPC_Sparse_Matrix;

void ddot (int n, double* x, double* y, double* result)
{  
  double local_result = 0.0;
  for (int i=0; i<n; i++) {
    local_result += x[i];
  }

  *result = local_result;
}


void waxpby (int n, double alpha, double* x, double beta, double* y, double* w)
{  
  for (int i=0; i<n; i++) {
    w[i] = alpha * x[i] + beta * y[i];
  }
}

void HPCCG (HPC_Sparse_Matrix * A, double* b, double* x, int max_iter, double tolerance, int* niters, double* normr)

{
  double nrow = A->local_nrow;
  double ncol = A->local_ncol;

  int nrow_int = nrow;
  int ncol_int = ncol;

  //double r[nrow];
  //double p[ncol]; // In parallel case, A is rectangular
  //double Ap[nrow];
  double* r = malloc(nrow_int * sizeof(double));
  double* p = malloc(ncol_int * sizeof(double)); // In parallel case, A is rectangular
  double* Ap = malloc(nrow_int * sizeof(double));

  double norm = 0.0;
  double* rtrans = malloc(sizeof(double));
  (*rtrans) = 0.0;
  double oldrtrans = 0.0;

  int rank = 0; // Serial case (not using MPI)

  // p is of length ncols, copy x to p for sparse MV operation
  waxpby(nrow, 1.0, b, -1.0, Ap, r);
  ddot(nrow, r, r, rtrans);
  norm = sqrt(*rtrans);

  printf("Initial Residual = %e\n", norm);
}

int main(int argc, char *argv[])
{

  HPC_Sparse_Matrix* A = malloc(sizeof(HPC_Sparse_Matrix)); // Allocate matrix struct and fill it
  double norm, d;
  int ierr = 0;
  int ione = 1;

  int nx = 13;
  int ny = 42;
  int nz = 23;

  int debug = 1;

  int size = 1; // Serial case (not using MPI)
  int rank = 0;

  (A)->title = 0;


  // Set this bool to true if you want a 7-pt stencil instead of a 27 pt stencil
  int use_7pt_stencil = 0;

  int local_nrow = nx*ny*nz; // This is the size of our subblock

  double max_nnz = 27;
  double local_nnz = max_nnz*local_nrow; // Approximately 27 nonzeros per row (except for boundary nodes)

  double total_nrow = local_nrow*size; // Total number of grid points in mesh
  long long total_nnz = 27* (long long) total_nrow; // Approximately 27 nonzeros per row (except for boundary nodes)

  int start_row = local_nrow*rank; // Each processor gets a section of a chimney stack domain
  int stop_row = start_row+local_nrow-1;

  // Allocate arrays that are of length local_nrow
  A->nnz_in_row = malloc(local_nrow * sizeof(int));
  A->ptr_to_vals_in_row = malloc(local_nrow * sizeof(double*));
  A->ptr_to_inds_in_row = malloc(local_nrow * sizeof(int*));

  double *x = malloc(local_nrow * sizeof(double));
  double *b = malloc(local_nrow * sizeof(double));
  double *xexact = malloc(local_nrow * sizeof(double));

  long long nnzglobal = 0;
  for (int iz=0; iz<nz; iz++) {
    for (int iy=0; iy<ny; iy++) {
      for (int ix=0; ix<nx; ix++) {
        int curlocalrow = iz*nx*ny+iy*nx+ix;

        (x)[curlocalrow] = 0.0;
        (b)[curlocalrow] = 27.0;
        (xexact)[curlocalrow] = 1.0;
      } // end ix loop
    } // end iy loop
  } // end iz loop  

  if (debug) printf("Process %d of %d has %d",rank,size,local_nrow);
  
  if (debug) printf(" rows. Global rows %d through %d\n",start_row,stop_row);

  if (debug) printf("Process %d of %d has %lld nonzeros.\n",rank,size,nnzglobal);

  A->local_nrow = local_nrow;
  A->local_ncol = local_nrow;

  int* niters = malloc(sizeof(int));
  double* normr = malloc(sizeof(double));

  int max_iter = 150;
  double tolerance = 0.0; // Set tolerance to zero to make all runs do max_iter iterations
  HPCCG(A, b, x, max_iter, tolerance, niters, normr);

  printf("%d", b[0]);
  printf("%d", x[0]);
  printf("%d", xexact[0]);
  //printf("%d", A->ptr_to_vals_in_row[0][0]);

  return 0 ;
} 
