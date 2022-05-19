int main(int argc, char** argv) {

	double size = 3;
	double* data = malloc(sizeof(double) * size);
	double** arr = malloc(size * sizeof(double*));
	arr[0] = malloc(size * sizeof(double));

	for (int i = 0; i < size; i++) {
		data[i] = i;
	}

	for (int i = 0; i < size; i++) {
		arr[0] = data;
		data++;
	}

	return arr[1][0];
}