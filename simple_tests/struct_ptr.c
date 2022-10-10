struct example_struct
{
	double* start;
	double end;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	example *ex = malloc(sizeof(example));
	ex->start = malloc(sizeof(double));

	fillStruct(ex);

	double result = (ex->start)[0] + ex->end;

	(ex->start)[0] = 23;

	printf("%f\n", result);
	return result;
}

void fillStruct(example *ex) {
	ex->end = 3;
	(ex->start)[0] = 4.0;
}