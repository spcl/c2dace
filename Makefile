run:
	python3 c2dace/c2d.py -f HPCCG_preprocessed/merged.c

clean:
	rm -Rf tmp/* .dacecache
