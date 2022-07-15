int main(int argc, char** argv) {

	int arr_size = 3;
	double* value = malloc(arr_size * sizeof(double));

	setI(value);

	printf("%f\n", value[0] + value[2]);
	return value[0] + value[2];
}

void setI(double* k) {
	k[0] = 5.0;
	k[1] = 7.0;
	k[2] = 9.0;
}