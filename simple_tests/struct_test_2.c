struct example_struct
{
	double start;
	double end;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	example *ex = malloc(sizeof(example));

	fillStruct(ex);

	return 0;
}

void fillStruct(example* ex) {
	ex->start = 4;
	ex->end = 3;
}