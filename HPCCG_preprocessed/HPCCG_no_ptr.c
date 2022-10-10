//@HEADER
// ************************************************************************
// 
//               HPCCG: Simple Conjugate Gradient Benchmark Code
//                 Copyright (2006) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// BSD 3-Clause License
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// 
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

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


void waxpby (int n, double alpha, double* x, double beta, double* y, double* w)
{  
  if (alpha==1.0) {
    for (int i=0; i<n; i++) w[i] = x[i] + beta * y[i];
  }
  else if(beta==1.0) {
    for (int i=0; i<n; i++) w[i] = alpha * x[i] + y[i];
  }
  else {
    for (int i=0; i<n; i++) w[i] = alpha * x[i] + beta * y[i];
  }

}

void ddot (int n, double* x, double* y, double* result)
{  
  double local_result = 0.0;
  if (y==x)
    for (int i=0; i<n; i++) local_result += x[i]*x[i];
  else
    for (int i=0; i<n; i++) local_result += x[i]*y[i];

  *result = local_result;
}

void HPC_sparsemv( HPC_Sparse_Matrix *A, double* x, double* y)
{

  int nrow = A->local_nrow;

  for (int i=0; i< nrow; i++) {
    double sum = 0.0;

    int cur_nnz = A->nnz_in_row[i];

    for (int j=0; j< cur_nnz; j++) {
      sum += A->ptr_to_vals_in_row[i][j]*x[A->ptr_to_inds_in_row[i][j]];
    }

    y[i] = sum;
  }
}

void dump_matlab_matrix(HPC_Sparse_Matrix *A, int rank) {
  double nrow = A->local_nrow;
  double start_row = nrow*rank; // Each processor gets a section of a chimney stack domain

  printf("===== MATRIX DUMP ======\n");
  for (int i=0; i< nrow; i++) {
    int cur_nnz = A->nnz_in_row[i];
    for (int j=0; j< cur_nnz; j++) {
      printf(" %f %d %22.16e,",start_row+i+1,A->ptr_to_inds_in_row[i][j]+1,A->ptr_to_vals_in_row[i][j]);
    }
    printf("\n");
  }
  printf("===== END DUMP ======\n");
}

void HPCCG (HPC_Sparse_Matrix * A, double* b, double* x, int max_iter, double tolerance, int* niters, double* normr) {
  int nrow = A->local_nrow;
  int ncol = A->local_ncol;

  double* r = calloc(nrow, sizeof(double));
  double* p = calloc(ncol, sizeof(double));
  double* Ap = calloc(nrow, sizeof(double));

  double norm = 0.0;
  double* rtrans = calloc(1, sizeof(double));
  rtrans[0] = 0;

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
  norm = sqrt(rtrans[0]);

  if (rank==0) printf("Initial Residual = %e\n", norm);

  for(int k=1; ((k<max_iter) && (norm > tolerance)); k++ ) {
    if (k == 1) {
      waxpby(nrow, 1.0, r, 0.0, r, p);
    } else {
      oldrtrans = rtrans[0];
      ddot (nrow, r, r, rtrans);// 2*nrow ops
      double beta = rtrans[0]/oldrtrans;
      waxpby (nrow, 1.0, r, beta, p, p);// 2*nrow ops
    }
    norm = sqrt(rtrans[0]);
    if (rank==0 && (k%print_freq == 0 || k+1 == max_iter))
    printf("Iteration = %d, Residual = %e\n", k, norm);
    

    HPC_sparsemv(A, p, Ap); // 2*nnz ops
    double* alpha = calloc(1, sizeof(double));
    alpha[0] = 0;
    ddot(nrow, p, Ap, alpha); // 2*nrow ops
    alpha[0] = rtrans[0]/alpha[0];
    waxpby(nrow, 1.0, x, alpha[0], p, x);// 2*nrow ops
    waxpby(nrow, 1.0, r, -(alpha[0]), Ap, r);// 2*nrow ops
    niters[0] = k;

    free(alpha);
  }

  double tmp = r[0] + Ap[0] + p[0];

  normr[0] = norm;
}

int main(int argc, char *argv[])
{

  HPC_Sparse_Matrix *A = calloc(1, sizeof(HPC_Sparse_Matrix));
  double norm;
  double d;
  int ierr = 0;
  int i, j;
  int ione = 1;

  int debug = 1;
  int size = 1; // Serial case (not using MPI)
  int rank = 0; 

  int nx = 89;
  int ny = 96;
  int nz = 101;

  A->title = 0;

  // Set this bool to true if you want a 7-pt stencil instead of a 27 pt stencil
  int use_7pt_stencil = 0;

  int local_nrow = nx*ny*nz; // This is the size of our subblock

  double max_nnz = 27;
  int local_nnz = max_nnz*local_nrow; // Approximately 27 nonzeros per row (except for boundary nodes)

  int total_nrow = local_nrow*size; // Total number of grid points in mesh
  long long total_nnz = max_nnz* (long long) total_nrow; // Approximately 27 nonzeros per row (except for boundary nodes)

  double start_row = local_nrow*rank; // Each processor gets a section of a chimney stack domain
  double stop_row = start_row+local_nrow-1;
  

  // Allocate arrays that are of length local_nrow
  A->nnz_in_row = calloc(local_nrow, sizeof(int));
  A->ptr_to_vals_in_row = calloc(local_nrow, sizeof(double*));
  A->ptr_to_inds_in_row = calloc(local_nrow, sizeof(int*));

  int max_nnz_int = max_nnz;
  A->ptr_to_vals_in_row[0] = calloc(max_nnz_int, sizeof(double));
  A->ptr_to_inds_in_row[0] = calloc(max_nnz_int, sizeof(int));

  A->ptr_to_vals_in_row[0][0] = 0;
  A->ptr_to_inds_in_row[0][0] = 0;

  double *x = calloc(local_nrow, sizeof(double));
  double *b = calloc(local_nrow, sizeof(double));

  long long nnzglobal = 0;
  for (int iz=0; iz<nz; iz++) {
    for (int iy=0; iy<ny; iy++) {
      for (int ix=0; ix<nx; ix++) {
        int curlocalrow = iz*nx*ny+iy*nx+ix;
        int currow = start_row+iz*nx*ny+iy*nx+ix;
        int nnzrow = 0;
        int curvalptr = 0;

        max_nnz_int = max_nnz;
        A->ptr_to_vals_in_row[curlocalrow] = calloc(max_nnz_int, sizeof(double));
        A->ptr_to_inds_in_row[curlocalrow] = calloc(max_nnz_int, sizeof(int));
        for (int sz=-1; sz<=1; sz++) {
          for (int sy=-1; sy<=1; sy++) {
            for (int sx=-1; sx<=1; sx++) {
              int curcol = currow+sz*nx*ny+sy*nx+sx;
              if (((((ix+sx>=0) && (ix+sx<nx)) && (iy+sy>=0)) && (iy+sy<ny)) && (curcol>=0 && curcol<total_nrow)) {
                if (!use_7pt_stencil || (sz*sz+sy*sy+sx*sx<=1)) { // This logic will skip over point that are not part of a 7-pt stencil
                  if (curcol==currow) {
                    A->ptr_to_vals_in_row[curlocalrow][curvalptr] = 27;
                  } else {
                    A->ptr_to_vals_in_row[curlocalrow][curvalptr] = -1;
                  }
                  //printf("%f", (A->ptr_to_vals_in_row)[0][0]);
                  A->ptr_to_inds_in_row[curlocalrow][curvalptr] = curcol;
                  curvalptr++;
                  nnzrow++;
                } 
              }
            } // end sx loop
          } // end sy loop
        } // end sz loop
        A->nnz_in_row[curlocalrow] = nnzrow;
        nnzglobal += nnzrow;
        x[curlocalrow] = 0.0;
        b[curlocalrow] = 27.0 - ((double) (nnzrow-1));
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

  int dump_matrix = 0;
  if (dump_matrix && size<=4) dump_matlab_matrix(A, rank);

  int* niters = calloc(1, sizeof(int));
  double* normr = calloc(1, sizeof(double));

  int max_iter = 150;
  double tolerance = 0.0; // Set tolerance to zero to make all runs do max_iter iterations
  HPCCG( A, b, x, max_iter, tolerance, niters, normr);

  double fniters = niters[0]; 
  double fnrow = A->total_nrow;
  double fnnz = A->total_nnz;
  double fnops_ddot = fniters*4*fnrow;
  double fnops_waxpby = fniters*6*fnrow;
  double fnops_sparsemv = fniters*2*fnnz;
  double fnops = fnops_ddot+fnops_waxpby+fnops_sparsemv;

  printf("Dimensions: x=%d, y=%d, z=%d\n",nx,ny,nz);
  printf("Number of iterations: %d\n", niters[0]);
  printf("Final residual: %e\n", normr[0]);

  printf("FLOPS Summary:\n");
  printf("Total   : %e\n",fnops);
  printf("DDOT    : %e\n",fnops_ddot);
  printf("WAXPBY  : %e\n",fnops_waxpby);
  printf("SPARSEMV: %e\n",fnops_sparsemv);

  // keep memory until the end
  //printf("%f", start_row);
  //printf("%f", stop_row);
  //printf("%d", b[0]);
  //printf("%d", x[0]);
  //printf("%f", (A->ptr_to_vals_in_row)[0][0]);
  //printf("%f", (A->ptr_to_inds_in_row)[0][0]);
  //printf("%d", (A->nnz_in_row)[0]);
  //printf("%d", *niters);
  //printf("%f", *normr);

  return 0;
} 
