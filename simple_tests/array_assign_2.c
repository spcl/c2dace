#include <stdlib.h>

int main(int argc, char** argv) {

	double size = 3;
	double* data = malloc(sizeof(double) * size);
	double* tmp = malloc(sizeof(double) * size);

	for (int i = 0; i < size; i++) {
		tmp[i] = 1;
	}

	func1(data, tmp, size);

	return data[1];
}

void func1(double* arr, double* src, double size) {
	func2(arr, src, size);
}

void func2(double* arr, double* src, double size) {
	for (int i = 0; i < 3; i++) {
		arr[i] = src[i];
	}
}