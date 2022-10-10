#include <stdlib.h>

struct example_struct {
  int size;
  double* data;
};

typedef struct example_struct example;

void foo(example* ex) {
	ex->size = 3;
}

void bar(example* ex) {
	for (int i=0; i<ex->size; i++) {
		ex->data[i] = i;
	}
}

int main(int argc, char** argv) {

	example* ex = malloc(sizeof(example));

	// init variables
	ex->data = malloc(sizeof(double) * 10);

	foo(ex);
	bar(ex);

	return ex->data[2];
}