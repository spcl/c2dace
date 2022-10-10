int main(int argc, char** argv) {
	double* value = malloc(sizeof(double));
	setI(value);
	return value[0];
}

void setI(double* k) {
	k[0] = 5.0;
}
