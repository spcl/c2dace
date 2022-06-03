#FILE := HPCCG_preprocessed/merged.c
FILE := simple_tests/matrix_init.c

run:
	python3 c2dace/c2d.py -f $(FILE)

clean:
	rm -Rf tmp/* .dacecache _dacegraphs

compile:
	gcc .dacecache/_pointer_arith_3/src/cpu/_pointer_arith_3.cpp -I ~/.local/lib/python3.10/site-packages/dace/runtime/include -lm -lstdc++ -ggdb
