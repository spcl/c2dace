struct example_struct
{
	double start;
	double end;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	example *ex = 0;
	ex = malloc(sizeof(example));

	fillStruct(ex);

	double result = ex->start + 4.0;

	return result;
}

void fillStruct(example *ex) {
	ex->start = 4;
	ex->end = 3;
}