struct example_struct
{
	double* data;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	example *ex = malloc(sizeof(example));
	ex->data = malloc(sizeof(double) * 3);

	fillStruct(ex);

	double* result = malloc(3 * sizeof(double));

	for (int i=0; i<3; i++) {
		result[i] = ex->data[i];
	}

	printf("%f\n", result[0] + result[2]);
	return result[0] + result[2];
}

void fillStruct(example *ex) {
	for (int i=0; i<3; i++) {
		(ex->data)[i] = (double) i;
	}
}