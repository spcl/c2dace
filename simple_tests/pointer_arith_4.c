#include <stdlib.h>

int main(int argc, char** argv) {

	double size = 3;
	double* data = malloc(sizeof(double) * size);

	for (int i = 0; i < size; i++) {
		data[i] = i;
	}

	double x = (data)[1];

	return x;
}