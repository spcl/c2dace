int main(int argc, char** argv) {

	double** value = malloc(sizeof(double));

	setI(value);

	return (*value)[0] + (*value)[1];
}

void setI(double** k) {
	double* ret = malloc(sizeof(double) * 2);
	ret[0] = 5.0;
	ret[1] = 7.0;
	(*k) = ret;
}