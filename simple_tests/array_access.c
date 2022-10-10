#include <stdlib.h>
#include <math.h>

struct COMPLEX_struct{
     double re;
	 double im;
};

typedef struct COMPLEX_struct COMPLEX;

int main(int argc, char** argv) {
	double N = 100;
	COMPLEX* in = malloc(N * sizeof(COMPLEX));
	for (int h = 0; h < N; ++h) {
		(in[h]).re = sqrt(h*9283) - h*h + h + 98 - h*12;
		(in[h]).im = sqrt(h*231) - h*h + h + 22 - h*34;
	}

	double tmp = in[0].re + in[0].im;

	return 0;
}
