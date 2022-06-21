int main(int argc, char** argv) {

	double* i = malloc(sizeof(double));

	i[0] = 5.0;

	printf("%f\n", i[0]);
	return i[0];
}