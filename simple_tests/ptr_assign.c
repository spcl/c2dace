int main(int argc, char** argv) {

	double addition = 5.0;

	double* value = malloc(sizeof(double));
	value[0] = 37;

	double* value2 = value;

	printf("%f\n", addition + value2[0]);
	return addition + value2[0];
}