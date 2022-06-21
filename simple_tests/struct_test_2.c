struct example_struct
{
	double start;
	double end;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	example *ex = malloc(sizeof(example));

	example loc_ex = {2, 3};
	loc_ex.start = 1;

	fillStruct(ex);

	double out = ex->end + loc_ex.start;

	printf("%f\n", out);
	return out;
}

void fillStruct(example* ex) {
	ex->start = 4;
	ex->end = 3;
}