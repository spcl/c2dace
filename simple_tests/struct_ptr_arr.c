struct example_struct
{
	double* start;
	double end;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	example* ex = malloc(sizeof(example));
	ex->start = malloc(sizeof(double) * 3);
	double* test_arr = malloc(sizeof(double) * 3);
	test_arr[1] = 1.0;

	fillStruct(ex);

	double result = (ex->start)[1] + test_arr[1] + ex->end;

	return result;
}

void fillStruct(example *ex) {
	ex->end = 3;
	(ex->start)[0] = 3.0;
	(ex->start)[1] = 2.0;
	(ex->start)[2] = 1.0;
}