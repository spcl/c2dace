FILE := HPCCG_preprocessed/merged.c
#FILE := simple_tests/merged_test.c

run:
	python3 c2dace/c2d.py -f $(FILE)

clean:
	rm -Rf tmp/* .dacecache _dacegraphs
