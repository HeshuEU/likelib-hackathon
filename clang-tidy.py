#!/usr/bin/python3

import sys, subprocess

PATH_TO_LOGS="/tmp/logs/"
#print(len(sys.argv))
index = sys.argv.index("-isystem")
l = sys.argv
l = l[:index + 1] + ["/usr/lib/gcc/x86_64-linux-gnu/9/include/"] + l[index + 1: ]
l[0] = "/usr/bin/clang-tidy"
try:
    output = subprocess.check_output(l, stderr=subprocess.STDOUT)
except Exception as e:
    with open(PATH_TO_LOGS+"clang-error.txt", "w") as f:
        f.write(str(e.output))
    exit(0)
with open(PATH_TO_LOGS+"clang-output.txt", "at") as f:
    f.write('\n'.join(output))


