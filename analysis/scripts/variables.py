#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Makes analysis variables (which can be used in many of the functions defined
in :mod:`modularAnalysis`) available to Python.
Details can be found on https://confluence.desy.de/display/BI/Physics+VariableManager

You can also run ``basf2 variables.py`` to list all available variables.
"""

from basf2 import *
from ROOT import gSystem
import argparse
import pager
gSystem.Load('libanalysis.so')

# import everything into current namespace.
from ROOT import Belle2
variables = Belle2.Variable.Manager.Instance()

import ROOT


def std_vector(*args):
    """
    Creates an std::vector which can be passed to pyROOT
    """
    v = ROOT.std.vector(type(args[0]))()
    for x in args:
        v.push_back(x)
    return v


def getCommandLineOptions():
    """ Parses the command line options of the fei and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('--no-pager', dest='pager', default=True, action='store_false',
                        help='Use a pager to show output or print to terminal.')
    args = parser.parse_args()
    return args


def printVars():
    """
    Print list of all available variables.
    """

    print('Available variables in Variable::Manager:')
    allVars = variables.getVariables()
    vars = []
    for v in allVars:
        vars.append((v.group, v.name, v.description))

    rows = []
    current_group = ''
    for (group, name, description) in sorted(vars):
        if current_group != group:
            current_group = group
            rows.append([group])
        rows.append([name, description])
    pretty_print_description_list(rows)


if __name__ == "__main__":
    args = getCommandLineOptions()

    if args.pager:
        with pager.Pager('Available variables in Variable\:\:Manager'):
            printVars()
    else:
        printVars()
