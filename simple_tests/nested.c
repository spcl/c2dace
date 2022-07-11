#include <stdlib.h>

void foo(double** a) {
	for (int i=0; i<10; i++) {
		a[i] = malloc(3 * sizeof(double));
		a[i][0] = 1;
		a[i][1] = 13;
		a[i][2] = 45;
	}
}

int main(int argc, char** argv) {
	double** a = malloc(10 * sizeof(double*));
	a[0] = malloc(3 * sizeof(double));
	foo(a);

	return 0;
}
