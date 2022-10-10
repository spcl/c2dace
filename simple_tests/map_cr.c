#include <stdlib.h>

int main(int argc, char** argv) {
	double result = 0.0;
	double* data = malloc(10 * sizeof(double));

	for (int i=0; i<10; i++) {
		data[i] = i*2;
	}

	for (int i=0; i<10; i++) {
		result += data[i];
	}

	printf("%f\n", result);
	return result;
}
