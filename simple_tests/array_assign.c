#include <stdlib.h>

int main(int argc, char** argv) {

	double size = 3;
	double* data = malloc(sizeof(double) * (size-1));
	double* tmp = malloc(sizeof(double) * size);

	for (int i = 0; i < size; i++) {
		tmp[i] = i;
	}

	tmp++;
	data = tmp;

	return data[1];
}