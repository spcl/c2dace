#FILE := HPCCG_preprocessed/merged.c
FILE := simple_tests/pointer_arith.c

run:
	python3 c2dace/c2d.py -f $(FILE)

clean:
	rm -Rf tmp/* .dacecache _dacegraphs

compile:
	gcc .dacecache/_merged/src/cpu/_merged.cpp -I ~/.local/lib/python3.10/site-packages/dace/runtime/include
