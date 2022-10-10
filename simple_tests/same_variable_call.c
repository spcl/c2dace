int main(int argc, char** argv) {

	double* list = malloc(sizeof(double) * 2);

	sum(list, list);

	printf("%f\n", list[0] + list[1]);
	return list[0] + list[1];
}

void sum(double* x, double* y) {
	x[0] = x[0] + y[0];
	x[1] = x[1] + y[1];
}