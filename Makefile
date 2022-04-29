#FILE := HPCCG_preprocessed/merged.c
FILE := simple_tests/and_condition.c

run:
	python3 c2dace/c2d.py -f $(FILE)

clean:
	rm -Rf tmp/* .dacecache
