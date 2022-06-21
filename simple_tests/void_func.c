int main(int argc, char** argv) {

	double addition = 5.0;

	double* value = malloc(sizeof(double));
	value[0] = 0;

	setI(value);


	printf("%f\n", addition + value[0]);
	return addition + value[0];
}

void setI(double* k) {
	k[0] = 5.0;
}