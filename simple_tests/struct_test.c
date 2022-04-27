struct example_struct
{
	double start;
	double end;
	double value;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	example **ex = malloc(sizeof(example*));
	(*ex) = malloc(sizeof(example));

	(*ex)->start = 4;
	(*ex)->end = 3;
	(*ex)->value = 12*0.23;

	return 0;
}