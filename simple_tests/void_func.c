int main(int argc, char** argv) {

	double* i = malloc(sizeof(double));

	setI(i);

	return 0;
}

void setI(double* i) {
	(*i) = 5.0;
}