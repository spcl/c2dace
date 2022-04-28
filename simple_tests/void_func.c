int main(int argc, char** argv) {

	double addition = 5.0;

	double* value = malloc(sizeof(double));
	(*value) = 0;

	setI(value);


	return addition + (*value);
}

void setI(double* k) {
	(*k) = 5.0;
}