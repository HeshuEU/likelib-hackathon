#!/usr/bin/python3

import sys
import os
import subprocess

PATH_TO_LOGS = "/tmp/logs"
LOG_NAME = "clang-tidy-output.txt"

l = sys.argv
l[0] = "/usr/bin/clang-tidy"

with open(os.path.join(PATH_TO_LOGS, "run-clang-tidy.sh"), "at") as script:
    for i in l:
        script.write(i + " ")
    script.write("\n")

try:
    output = subprocess.check_output(l, stderr=subprocess.STDOUT)
    with open(os.path.join(PATH_TO_LOGS, LOG_NAME), "at") as f:
        f.write('\n'.join(output))
except Exception as e:
    with open(os.path.join(PATH_TO_LOGS, LOG_NAME), "at", encoding='utf8') as f:
        # Print output with correct \n
        f.write(str(e.output.decode("utf-8")))
    open(os.path.join(PATH_TO_LOGS, ".error-flag"), "w").close()
    # Set error flag, and continue check other files with exit(0)
    exit(0)
