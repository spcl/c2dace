#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct example_struct {
	double* x ;
	double* y ;
};

typedef struct example_struct example;

int main (int argc, char** argv) {
	long N = pow(2, 26);
	example* ex = malloc(sizeof(example));
	ex->x = malloc(N*sizeof(double));
	ex->y = malloc(N*sizeof(double));
	for (long i=0; i < N ; i++) {
		ex->x[i] = i+2;
		ex->y[i] = i*2;
	}

	double sum = 0;

	for (long i=0; i<N; i++) {
		sum += ex->x[i];
		sum += ex->y[i];
	}

	double tmp = ex->x[0] + ex->y[0];

	printf("%f\n", sum);

	return 0;
}
