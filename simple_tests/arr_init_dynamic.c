int main(int argc, char** argv) {

	double arr_size = 3;
	double* value = malloc(sizeof(double) * arr_size);

	setI(value);

	return value[0] + value[2];
}

void setI(double* k) {
	k[0] = 5.0;
	k[1] = 7.0;
	k[2] = 9.0;
}