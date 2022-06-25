DACE_INCLUDE = ~/.local/lib/python3.9/site-packages/dace/runtime/include

FILENAME := fft
FILE := bots_preprocessed/$(FILENAME).c

run:
	python3 c2dace/c2d.py -f $(FILE)

clean:
	rm -Rf tmp/* .dacecache _dacegraphs a.out orig

compile:
	gcc .dacecache/_$(FILENAME)/sample/_$(FILENAME)_main.cpp .dacecache/_$(FILENAME)/src/cpu/_$(FILENAME).cpp -I $(DACE_INCLUDE) -lstdc++ -lm -ggdb

test:
	python3 testing/harness.py
