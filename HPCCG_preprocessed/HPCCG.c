
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
/////////////////////////////////////////////////////////////////////////

// Routine to compute an approximate solution to Ax = b where:

// A - known matrix stored as an HPC_Sparse_Matrix struct

// b - known right hand side vector

// x - On entry is initial guess, on exit new approximate solution

// max_iter - Maximum number of iterations to perform, even if
//            tolerance is not met.

// tolerance - Stop and assert convergence if norm of residual is <=
//             to tolerance.

// niters - On output, the number of iterations actually performed.

/////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "HPCCG.h"
#include "ddot.h"
#include "waxpby.h"
#include "generate_matrix.h"
#include "dump_matlab_matrix.h"

void HPCCG (HPC_Sparse_Matrix * A, double* b, double* x, int max_iter, double tolerance, int* niters, double* normr)
{
  int nrow = A->local_nrow;
  int ncol = A->local_ncol;

  double r[nrow];
  double p[ncol]; // In parallel case, A is rectangular
  double Ap[nrow];

  double norm = 0.0;
  double rtrans = 0.0;
  double oldrtrans = 0.0;

  int rank = 0; // Serial case (not using MPI)

  int print_freq = max_iter/10; 
  if (print_freq>50) print_freq=50;
  if (print_freq<1)  print_freq=1;

  // p is of length ncols, copy x to p for sparse MV operation
  waxpby(nrow, 1.0, x, 0.0, x, p);
  HPC_sparsemv(A, p, Ap);
  waxpby(nrow, 1.0, b, -1.0, Ap, r);
  ddot(nrow, r, r, &rtrans);
  norm = sqrt(rtrans);

  if (rank==0) printf("Initial Residual = %e\n", norm);

  for(int k=1; k<max_iter && norm > tolerance; k++ )
    {
      if (k == 1)
	{
	  waxpby(nrow, 1.0, r, 0.0, r, p);
	}
      else
	{
	  oldrtrans = rtrans;
	  ddot (nrow, r, r, &rtrans);// 2*nrow ops
	  double beta = rtrans/oldrtrans;
	  waxpby (nrow, 1.0, r, beta, p, p);// 2*nrow ops
	}
      norm = sqrt(rtrans);
      if (rank==0 && (k%print_freq == 0 || k+1 == max_iter))
      printf("Iteration = %d, Residual = %e\n", k, norm);
     

      HPC_sparsemv(A, p, Ap); // 2*nnz ops
      double alpha = 0.0;
      ddot(nrow, p, Ap, &alpha); // 2*nrow ops
      alpha = rtrans/alpha;
      waxpby(nrow, 1.0, x, alpha, p, x);// 2*nrow ops
      waxpby(nrow, 1.0, r, -alpha, Ap, r);// 2*nrow ops
      *niters = k;
    }

  *normr = norm;
}

int main(int argc, char *argv[])
{

  HPC_Sparse_Matrix *A;
  double *x, *b, *xexact;
  double norm, d;
  int ierr = 0;
  int i, j;
  int ione = 1;
  int nx,ny,nz;


  int size = 1; // Serial case (not using MPI)
  int rank = 0; 

  nx = 13;
  ny = 42;
  nz = 23;

  int debug = 1;

  int size = 1; // Serial case (not using MPI)
  int rank = 0;

  A = calloc(1, sizeof(HPC_Sparse_Matrix)); // Allocate matrix struct and fill it
  A->title = 0;


  // Set this bool to true if you want a 7-pt stencil instead of a 27 pt stencil
  int use_7pt_stencil = 0;

  int local_nrow = nx*ny*nz; // This is the size of our subblock

  double max_nnz = 27;
  int local_nnz = max_nnz*local_nrow; // Approximately 27 nonzeros per row (except for boundary nodes)

  int total_nrow = local_nrow*size; // Total number of grid points in mesh
  long long total_nnz = max_nnz* (long long) total_nrow; // Approximately 27 nonzeros per row (except for boundary nodes)

  int start_row = local_nrow*rank; // Each processor gets a section of a chimney stack domain
  int stop_row = start_row+local_nrow-1;
  

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
              if ((ix+sx>=0) && (ix+sx<nx) && (iy+sy>=0) && (iy+sy<ny) && (curcol>=0 && curcol<total_nrow)) {
                if (!use_7pt_stencil || (sz*sz+sy*sy+sx*sx<=1)) { // This logic will skip over point that are not part of a 7-pt stencil
                  if (curcol==currow) {
                    A->ptr_to_vals_in_row[curlocalrow][curvalptr] = 27;
                  } else {
                    A->ptr_to_vals_in_row[curlocalrow][curvalptr] = -1;
                  }
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
        xexact[curlocalrow] = 1.0;
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

  return 0 ;
} 