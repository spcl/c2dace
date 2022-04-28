struct example_struct
{
	double* data;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	example *ex = malloc(sizeof(example));

	fillStruct(ex);

	double result[3];

	for (int i=0; i<3; i++) {
		result[i] = ex->data[i];
	}

	return result[0] + result[2];
}

void fillStruct(example *ex) {
	ex->data = malloc(sizeof(double) * 3);
	for (int i=0; i<3; i++) {
		(ex->data)[i] = (double) i;
	}
}