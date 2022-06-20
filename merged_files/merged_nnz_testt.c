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

  double *list_of_vals;   //needed for cleaning up memory
  int *list_of_inds;      //needed for cleaning up memory
};


typedef struct HPC_Sparse_Matrix_STRUCT HPC_Sparse_Matrix;

int rand_lim(int limit) {
/* return a random number between 0 and limit inclusive.
 */

    int divisor = RAND_MAX/(limit+1);
    int retval;

    do { 
        retval = rand() / divisor;
    } while (retval > limit);

    return retval;
}

int run(double* means)
{

  HPC_Sparse_Matrix* A = malloc(sizeof(HPC_Sparse_Matrix)); // Allocate matrix struct and fill it
  double norm, d;
  int ierr = 0;
  int ione = 1;

  //int nx = 13;
  //int ny = 42;
  //int nz = 23;

  int nx =rand_lim(100) + 1;
  int ny = rand_lim(100) + 1;
  int nz = rand_lim(100) + 1;
  printf("nx = %d, ny = %d, nz = %d\n", nx, ny, nz);

  int debug = 1;

  int size = 1; // Serial case (not using MPI)
  int rank = 0;

  (A)->title = 0;


  // Set this bool to true if you want a 7-pt stencil instead of a 27 pt stencil
  int use_7pt_stencil = 0;

  double local_nrow = nx*ny*nz; // This is the size of our subblock

  double local_nnz = 27*local_nrow; // Approximately 27 nonzeros per row (except for boundary nodes)

  double total_nrow = local_nrow*size; // Total number of grid points in mesh
  long long total_nnz = 27* (long long) total_nrow; // Approximately 27 nonzeros per row (except for boundary nodes)

  int start_row = local_nrow*rank; // Each processor gets a section of a chimney stack domain
  int stop_row = start_row+local_nrow-1;

  // Allocate arrays that are of length local_nrow
  A->nnz_in_row = malloc(local_nrow * sizeof(int));
  A->ptr_to_vals_in_row = malloc(local_nrow * sizeof(double*));
  A->ptr_to_inds_in_row = malloc(local_nrow * sizeof(int*));

  // Allocate arrays that are of length local_nnz
  A->list_of_vals = malloc(local_nnz * sizeof(double));
  A->list_of_inds = malloc(local_nnz * sizeof(int));

  double * curvalptr = A->list_of_vals;
  int * curindptr = A->list_of_inds;

  long long nnzglobal = 0;
  for (int iz=0; iz<nz; iz++) {
    for (int iy=0; iy<ny; iy++) {
      for (int ix=0; ix<nx; ix++) {
        int curlocalrow = iz*nx*ny+iy*nx+ix;
        int currow = start_row+iz*nx*ny+iy*nx+ix;
        int nnzrow = 0;
        (A->ptr_to_vals_in_row)[curlocalrow] = curvalptr;
        (A->ptr_to_inds_in_row)[curlocalrow] = curindptr;
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
                    curvalptr[0] = 27.0;
                    curvalptr++;
                  } else {
                    curvalptr[0] = -1.0;
                    curvalptr++;
                  }
                  curindptr[0] = curcol;
                  curindptr++;
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
          return 1;
        }
      } // end ix loop
     } // end iy loop
  } // end iz loop  
  free(A->nnz_in_row);
  free(A->ptr_to_vals_in_row);
  free(A->ptr_to_inds_in_row);
  free(A->list_of_vals);
  free(A->list_of_inds);

  *means += nnzglobal/local_nrow;

  return 0;
} 

int main(int argc, char *argv[])
{
  srand(time(0));

  int runs = 100;
  double means = 0;

  int result = 0;
  for (int i=0; i<runs; i++) {
    result = run(&means);
    if (result != 0) {
      printf("Error in run %d\n", i);
      return 1;
    }
  }

  printf("nnz mean %f\n", means/runs);

  return 0;
}
