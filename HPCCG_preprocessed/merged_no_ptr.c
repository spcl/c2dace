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
  if (y==x)
    for (int i=0; i<n; i++) local_result += x[i]*x[i];
  else
    for (int i=0; i<n; i++) local_result += x[i]*y[i];

  *result = local_result;
}


void waxpby (int n, double alpha, double* x, double beta, double* y, double* w)
{  
  for (int i=0; i<n; i++) {
    w[i] = alpha * x[i] + beta * y[i];
  }
}


void HPC_sparsemv( HPC_Sparse_Matrix *A, double* x, double* y)
{

  int nrow = A->local_nrow;

  for (int i=0; i< nrow; i++)
    {
      double sum = 0.0;

      int cur_nnz = (A->nnz_in_row)[i];

      for (int j=0; j< cur_nnz; j++) {
        int cur_ind = A->ptr_to_inds_in_row[i][j]; 
        sum += (A->ptr_to_vals_in_row)[i][j]*x[cur_ind];
      }

      y[i] = sum;
    }
}


void HPCCG (HPC_Sparse_Matrix * A, double* b, double* x, int max_iter, double tolerance, int* niters, double* normr)

{
  double nrow = A->local_nrow;
  double ncol = A->local_ncol;

  //double r[nrow];
  //double p[ncol]; // In parallel case, A is rectangular
  //double Ap[nrow];
  double* r = malloc(nrow * sizeof(double));
  double* p = malloc(ncol * sizeof(double)); // In parallel case, A is rectangular
  double* Ap = malloc(nrow * sizeof(double));

  double norm = 0.0;
  double* rtrans = malloc(sizeof(double));
  (*rtrans) = 0.0;
  double oldrtrans = 0.0;

  int rank = 0; // Serial case (not using MPI)

  int print_freq = max_iter/10; 
  if (print_freq>50) print_freq=50;
  if (print_freq<1)  print_freq=1;

  // p is of length ncols, copy x to p for sparse MV operation
  waxpby(nrow, 1.0, x, 0.0, x, p);
  HPC_sparsemv(A, p, Ap);
  waxpby(nrow, 1.0, b, -1.0, Ap, r);
  ddot(nrow, r, r, rtrans);
  norm = sqrt(*rtrans);

  if (rank==0) printf("Initial Residual = %e\n", norm);

  for(int k=1; (k<max_iter) && (norm > tolerance); k++ ) {
    if (k == 1) {
      waxpby(nrow, 1.0, r, 0.0, r, p);
	  } else {
      oldrtrans = (*rtrans);
      ddot (nrow, r, r, rtrans);// 2*nrow ops
      double beta = (*rtrans)/oldrtrans;
      waxpby (nrow, 1.0, r, beta, p, p);// 2*nrow ops
    }
    norm = sqrt(*rtrans);
    if (rank==0 && (k%print_freq == 0 || k+1 == max_iter))
    printf("Iteration = %d, Residual = %e\n", k, norm);
    

    HPC_sparsemv(A, p, Ap); // 2*nnz ops
    double* alpha = malloc(sizeof(double));
    (*alpha) = 0.0;
    ddot(nrow, p, Ap, alpha); // 2*nrow ops
    (*alpha) = (*rtrans)/(*alpha);
    waxpby(nrow, 1.0, x, (*alpha), p, x);// 2*nrow ops
    waxpby(nrow, 1.0, r, -(*alpha), Ap, r);// 2*nrow ops
    *niters = k;
    }

  *normr = norm;
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

  double local_nrow = nx*ny*nz; // This is the size of our subblock

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
        int currow = start_row+iz*nx*ny+iy*nx+ix;
        int nnzrow = 0;
        int curvalptr = 0;
        (A->ptr_to_vals_in_row)[curlocalrow] = malloc(max_nnz * sizeof(double));
        (A->ptr_to_inds_in_row)[curlocalrow] = malloc(max_nnz * sizeof(int));
        for (int sz=-1; sz<=1; sz++) {
          for (int sy=-1; sy<=1; sy++) {
            for (int sx=-1; sx<=1; sx++) {
              int curcol = currow+sz*nx*ny+sy*nx+sx;
              // Since we have a stack of nx by ny by nz domains , stacking in the z direction, we check to see
              // if sx and sy are reaching outside of the domain, while the check for the curcol being valid
              // is sufficient to check the z values
              if (((((ix+sx>=0) && (ix+sx<nx)) && (iy+sy>=0)) && (iy+sy<ny)) && (curcol>=0 && curcol<total_nrow)) {
                if (!use_7pt_stencil || (sz*sz+sy*sy+sx*sx<=1)) { // This logic will skip over point that are not part of a 7-pt stencil
                  if (curcol==currow) {
                    (A->ptr_to_vals_in_row)[curlocalrow][curvalptr] = 27;
                  } else {
                    (A->ptr_to_vals_in_row)[curlocalrow][curvalptr] = -1;
                  }
                  (A->ptr_to_inds_in_row)[curlocalrow][curvalptr] = curcol;
                  curvalptr++;
                  nnzrow++;
                } 
              }
            } // end sx loop
          } // end sy loop
        } // end sz loop
        (A->nnz_in_row)[curlocalrow] = nnzrow;
        nnzglobal += nnzrow;

        if (nnzrow > 27) {
          printf("WARNING: Row %d has %d nonzeros\n", currow, nnzrow);
        }

        (x)[curlocalrow] = 0.0;
        (b)[curlocalrow] = 27.0 - ((double) (nnzrow-1));
        (xexact)[curlocalrow] = 1.0;
      } // end ix loop
     } // end iy loop
  } // end iz loop  
  if (debug) printf("Process %d of %d has %d",rank,size,local_nrow);
  
  if (debug) printf(" rows. Global rows %d through %d\n",start_row,stop_row);

  if (debug) printf("Process %d of %d has %lld nonzeros.\n",rank,size,nnzglobal);

  A->start_row = start_row ; 
  A->stop_row = stop_row;
  A->total_nrow = total_nrow;
  A->total_nnz = total_nnz;
  A->local_nrow = local_nrow;
  A->local_ncol = local_nrow;
  A->local_nnz = local_nnz;

  int* niters = malloc(sizeof(int));
  double* normr = malloc(sizeof(double));

  int max_iter = 150;
  double tolerance = 0.0; // Set tolerance to zero to make all runs do max_iter iterations
  HPCCG(A, b, x, max_iter, tolerance, niters, normr);

  double fniters = *niters; 
  double fnrow = A->total_nrow;
  double fnnz = A->total_nnz;
  double fnops_ddot = fniters*4*fnrow;
  double fnops_waxpby = fniters*6*fnrow;
  double fnops_sparsemv = fniters*2*fnnz;
  double fnops = fnops_ddot+fnops_waxpby+fnops_sparsemv;

  printf("Dimensions: nx=%d, ny=%d, nz=%d\n",nx,ny,nz);
  printf("Number of iterations: %d\n", *niters);
  printf("Final residual: %e\n", *normr);

  printf("FLOPS Summary:\n");
  printf("Total   : %e\n",fnops);
  printf("DDOT    : %e\n",fnops_ddot);
  printf("WAXPBY  : %e\n",fnops_waxpby);
  printf("SPARSEMV: %e\n",fnops_sparsemv);

  // Compute difference between known exact solution and computed solution
  // All processors are needed here.

  double residual = 0;
  //  if ((ierr = compute_residual(A->local_nrow, x, xexact, &residual)))
  //  cerr << "Error in call to compute_residual: " << ierr << ".\n" << endl;

  // if (rank==0)
  //   cout << "Difference between computed and exact  = " 
  //        << residual << ".\n" << endl;


  // Finish up
  return 0 ;
} 