#!/usr/bin/python3

import sys, subprocess

PATH_TO_LOGS="/tmp/logs/"
LOG_NAME="clang-tidy-output.txt"
#print(len(sys.argv))
#if not "-isystem" in sys.argv:
#    sys.argv.append("-isystem")
#index = sys.argv.index("-isystem")
l = sys.argv
#l = l[:index + 1] + ["/usr/lib/gcc/x86_64-linux-gnu/9/include/", "-isystem"] + l[index + 1: ]
l[0] = "/usr/bin/clang-tidy"
script = open(PATH_TO_LOGS+"run-clang-tidy.sh","at")
for i in l: script.write(i + " ")
script.write("\n")
script.close()
try:
    output = subprocess.check_output(l, stderr=subprocess.STDOUT)
except Exception as e:
    with open(PATH_TO_LOGS + LOG_NAME, "at") as f:
        f.write(e.output.decode("utf-8"))
    open(PATH_TO_LOGS + ".error-flag", "w").close()
    exit(0)
with open(PATH_TO_LOGS + LOG_NAME, "at") as f:
    f.write('\n'.join(output))


