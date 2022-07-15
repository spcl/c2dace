struct example_struct
{
	double* start;
	double end;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	example* ex = malloc(sizeof(example));
	ex->start = malloc(sizeof(double) * 3);

	fillStruct(ex);

	printf("%f\n", (ex->start)[1]);
	return (ex->start)[1];
}

void fillStruct(example *ex) {
	ex->end = 3;
	for (int i = 0; i < 3; i++) {
		(ex->start)[i] = i;
	}
}