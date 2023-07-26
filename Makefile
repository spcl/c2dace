DACE_INCLUDE = ~/.local/lib/python3.9/site-packages/dace/runtime/include

FILENAME := struct_ptr_arr
FILE := simple_tests/$(FILENAME).c
FILENAME2 := _$(FILENAME)

run:
	python3 c2dace/c2d.py -f $(FILE)

clean:
	rm -Rf tmp/* .dacecache _dacegraphs a.out orig

compile:
	gcc .dacecache/$(FILENAME2)/sample/$(FILENAME2)_main.cpp .dacecache/$(FILENAME2)/src/cpu/$(FILENAME2).cpp -I $(DACE_INCLUDE) -lstdc++ -lm -ggdb

test:
	python3 testing/harness.py
