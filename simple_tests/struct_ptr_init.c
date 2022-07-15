struct example_struct
{
	double* start;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	example *ex = malloc(sizeof(example));
	ex->start = malloc(sizeof(double));
	*ex->start = 3.0;

	double* value = ex->start;

	printf("%f\n", value[0]);
	return value[0];
}