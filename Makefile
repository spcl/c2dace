#FILE := HPCCG_preprocessed/merged_no_ptr.c
FILENAME := remerged
FILE := simple_tests/$(FILENAME).c

run:
	python3 c2dace/c2d.py -f $(FILE)

clean:
	rm -Rf tmp/* .dacecache _dacegraphs a.out orig

compile:
	gcc .dacecache/_$(FILENAME)/sample/_$(FILENAME)_main.cpp .dacecache/_$(FILENAME)/src/cpu/_$(FILENAME).cpp -I /home/mafaldo/.local/lib/python3.10/site-packages/dace/runtime/include -lstdc++ -lm -ggdb

patch:
	python3 patcher.py

orig:
	gcc -lm -o orig $(FILE)
