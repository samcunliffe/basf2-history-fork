#!/usr/bin/env python

import subprocess
import sys

OLD = "decay_hash_interpreter.py"
NEW = "b2decay-hash-interpreter.py"

print(f"""\033[0;31mAttention:\033[0m {OLD} has been renamed to {NEW}.

Some time ago the software group has decided that we should enforce a more
systematic naming convention for command line tools. The old names have been
kept for compatibility but will now be removed. The tool you tried to call is
one of those which has been renamed.

From now on please use \033[0;32m{NEW}\033[0m

\033[0;31mAttention:\033[0m ${OLD} has been renamed to ${NEW}."""
      )

command = [NEW] + sys.argv[1:]
print(f"\nRunning command {' '.join(command)}\n")
subprocess.run(command, check=True)
