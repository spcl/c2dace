
int main(int argc, char** argv) {

    int size_1 = 5;
    int size_2 = 10;
	double** data = malloc(size_1 * sizeof(double*));
    data[0] = malloc(size_2 * sizeof(double));

    data[size_1-3][size_2-5] = 32;

	return 0;
}