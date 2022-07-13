#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct example_struct {
	double* x ;
	double* y ;
};

typedef struct example_struct example;

int main (int argc, char** argv) {
	long N = pow(2, 20);
	example* ex = malloc(sizeof(example));
	ex->x = malloc(N*sizeof(double));
	ex->y = malloc(N*sizeof(double));
	for (long i=0; i < N ; i++) {
		ex->x[i] = i+2;
		ex->y[i] = i*2;
	}

	double tmp = ex->x[123] + ex->y[598];

	printf("%f\n", tmp);

	return 0;
}
