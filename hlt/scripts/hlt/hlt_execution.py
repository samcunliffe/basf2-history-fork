#! /usr/bin/env python3
import os
import subprocess
import sys

from hlt.clean_execution import CleanBasf2Execution


def main(script_name):
    """Shortcut method to do the basf2 execution on expressreco and hlt"""
    os.chdir(sys.path[0])
    execution = CleanBasf2Execution(["basf2", "--no-stats", script_name, "--"] + sys.argv[1:])
    try:
        return_code = execution.run()
    finally:
        subprocess.run("framework-pcore-clear_ipcs")
    return return_code
