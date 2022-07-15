
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>


extern "C"  int __dace_init_Top(double * __restrict__ _argv, int _argc, int _argcount) ;    

extern "C"  void __dace_exit_Top(double * __restrict__ _argv, int _argc, int _argcount);
extern "C" void __program_Top(double * __restrict__ _argv, int _argc, int _argcount);


int main(int argc, char** argv)
{
   __dace_init_Top((double*) argv,argc,1);
__program_Top((double*) argv,argc,1);
__dace_exit_Top((double*)argv,argc,1);
  return 0;
}
