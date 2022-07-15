int main(int argc, char** argv) {

	double addition = 5.0;

	double* value = malloc(sizeof(double));

	setI(value);

	printf("%f\n", addition + value[0]);
	return addition + value[0];
}

void setI(double* k) {
	setK(k);
	for (int i = 0; i < 3; i++) {
		setK(k);
	}
}

void setK(double* w) {
	w[0] = 3.0;
}