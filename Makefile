#FILE := HPCCG_preprocessed/merged.c
#FILE := simple_tests/struct_null_init.c
FILE := simple_tests/void_func.c
#FILE := simple_tests/ret_func.c

run:
	python3 c2dace/c2d.py -f $(FILE)

clean:
	rm -Rf tmp/* .dacecache
