struct example_struct
{
	double size;
	double** data;
};

typedef struct example_struct example;


int main(int argc, char** argv) {

	double arr_size = 3;
	example *ex = malloc(sizeof(example));
	ex->data = malloc(sizeof(double) * arr_size);
	(ex->data)[0] = malloc(sizeof(double) * 2);

	for (int i=0; i<3; i++) {
		double* tmp_arr = malloc(sizeof(double)*2);
		tmp_arr[0] = i;
		tmp_arr[1] = i*2;
		(ex->data)[i] = tmp_arr;
	}

	return (ex->data)[0][0] + (ex->data)[1][1];
}