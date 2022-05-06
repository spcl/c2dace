int main(int argc, char** argv) {

	double addition = 5.0;

	double* value = malloc(sizeof(double));
	(*value) = 37;

	double* value2 = value;

	return addition + (*value2);
}