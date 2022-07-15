#include <stdio.h>
#include <math.h>

/*
int pow(int base, int exponent) {
	if (exponent == 0) {
		return 1;
	}

	if (exponent == 1) {
		return base;
	}

	int result = 1;

	for (int i=exponent; i>0; i--) {
		result = result * base;
	}

	return result;
}
*/

int main(int argc, char** argv) {
	double x, result;

	x = 5;
	result = pow(x, 2);

	printf("%f\n", result);

	printf("%f\n", 0);
	return 0;
}
