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

	double sum = 0;
	for (int i = 0; i < 3; i++) {
		sum += ex->start[i];
	}

	return sum;
}

void fillStruct(example *ex) {
	ex->end = 3;
	for (int i = 0; i < 3; i++) {
		(ex->start)[i] = i;
	}
}