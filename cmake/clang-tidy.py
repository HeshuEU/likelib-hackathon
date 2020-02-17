#!/usr/bin/python3

import sys, subprocess

PATH_TO_LOGS="/tmp/logs/"
#print(len(sys.argv))
index = sys.argv.index("-isystem")
l = sys.argv
l = l[:index + 1] + ["/usr/lib/gcc/x86_64-linux-gnu/9/include/", "-isystem"] + l[index + 1: ]
l[0] = "/usr/bin/clang-tidy"
script = open(PATH_TO_LOGS+"run-clang-tidy.sh","at")
for i in l: script.write(i)
script.write("\n")
script.close()
try:
    output = subprocess.check_output(l, stderr=subprocess.STDOUT)
except Exception as e:
    with open(PATH_TO_LOGS+"clang-error.txt", "at") as f:
        for i in l: f.write(i)
        f.write("\n\n")
        f.write(e.output.decode("utf-8"))
    exit(1)
with open(PATH_TO_LOGS+"clang-output.txt", "at") as f:
    f.write('\n'.join(output))


