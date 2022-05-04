int main(int argc, char** argv) {

	double addition = 5.0;

	double value[1];
	value[0] = 0;

	setI(value);

	return addition + (*value);
}

void setI(double k[]) {
	setK(k);
}

void setK(double w[]) {
	w[0] = 3.0;
}