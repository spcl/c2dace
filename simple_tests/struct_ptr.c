struct example_struct
{
	double* start;
	double end;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	example *ex = malloc(sizeof(example));

	fillStruct(ex);

	double result = *(ex->start) + ex->end;

	return result;
}

void fillStruct(example *ex) {
	ex->end = 3;
	ex->start = malloc(sizeof(double));
	*(ex->start) = 3.0;
}