import dace

@dace.program
def set_array(x, y, alpha):
    i = 2
    if alpha == 1:
        y[i] = x[i]
    else:
        y[i] = x[i]


@dace.program
def main():
    size = 10
    a = [0*size]
    b = [0*size]

    set_array(a, b, 1.0)

    return b[2]

sdfg = main.to_sdfg()
sdfg.simplify()
sdfg.save("tmp/python.sdfg")
for codeobj in sdfg.generate_code():
    if codeobj.title == 'Frame':
        with open("tmp/py.cc", 'w') as fp:
            fp.write(codeobj.clean_code)