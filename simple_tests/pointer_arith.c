int main(int argc, char** argv) {

	double size = 3;
	double* arr = malloc(size * sizeof(double));

	for (int i = 0; i < size; i++) {
		arr[0] = i*3.0;
		arr++;
	}

	return arr[0] + arr[2];
}