struct example_struct
{
	double* start;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	example *ex = malloc(sizeof(example));
  ex->start = malloc(sizeof(double));

  double* value = ex->start;

	return (*value);
}