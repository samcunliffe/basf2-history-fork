#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
You can run ``basf2 variables.py`` to list all available variables.
"""
from terminal_utils import Pager
import argparse
from variables import getCommandLineOptions
from variables import printVars

if __name__ == "__main__":
    args = getCommandLineOptions()

    if args.pager:
        with Pager(r'Available variables in Variable\:\:Manager'):
            printVars()
    else:
        printVars()
