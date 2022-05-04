FILE := HPCCG_preprocessed/merged.c
#FILE := simple_tests/struct_ptr_init.c

run:
	python3 c2dace/c2d.py -f $(FILE)

clean:
	rm -Rf tmp/* .dacecache _dacegraphs
