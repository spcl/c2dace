struct example_struct
{
	double* data;
	double size;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	example *ex = malloc(sizeof(example));
	ex->size = 3;
	ex->data = malloc(sizeof(double) * (ex->size));

	fillStruct(ex);

	double result[3];

	for (int i=0; i<3; i++) {
		result[i] = ex->data[i];
	}

	return result[0] + result[2];
}

void fillStruct(example *ex) {
	for (int i=0; i<3; i++) {
		(ex->data)[i] = (double) i;
	}
}