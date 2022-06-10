import os

flist = os.listdir(".dacecache")
fname = flist[0]

DIR = ".dacecache/" + fname + "/src/cpu/" + fname + ".cpp"

data = []

with open(DIR, "r") as f:
    for line in f:
        data.append(line)

target = 0
dst = 0
for i in range(len(data)):
    if "delete[] dace_r_0;" in data[i]:
        begin = i

    if "delete[] dace_rtrans_0;" in data[i]:
        dst = i
        break

size = 3

print("Patching file: " + DIR)
print("Moving lines: " + str(begin) + " to " + str(dst))

data_to_move = data[begin:begin+size]

for l in data_to_move:
    data.insert(dst+1, l)

for l in range(len(data_to_move)):
    del data[begin]

with open(DIR, "w") as f:
    for l in data:
        f.write(l)