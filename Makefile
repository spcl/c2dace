#FILE := HPCCG_preprocessed/merged.c
FILE := simple_tests/struct_test_double.c

run:
	python3 c2dace/c2d.py -f $(FILE)

clean:
	rm -Rf tmp/* .dacecache
