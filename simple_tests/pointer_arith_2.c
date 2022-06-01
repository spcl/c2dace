int main(int argc, char** argv) {

	int size = 3;
	double* arr = malloc(size * sizeof(double));
	double* arr2 = arr;

	arr++;
	setArr(arr);

	arr++;

	return arr[0];
}

void setArr(double* arr) {
	arr[1] = 3.0;
}