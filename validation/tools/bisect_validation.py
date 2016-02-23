#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Utility script to be used in conjuction with the git bisect functionality [1] to
search for degredations in validation variables. To use it, you can set the last known
good git commit and the first known bad commit and start the bistecting process.
Git will then checkout commits between the good and the bad and perform a binary search
to arrive at the first bad commit.

To give feed back to git, which is first bad commit, this script either returns the
exit code 0 for good commit, code 125 to ignore the current commit or code 1 for a bad commit.

Information contained within .root files produced by the official validation scripcts or your
custom scripts can be used to distingush between good and bad commits.

The syntaxn of the --check-quantity option is the following:
<root filename>:<quantity>:<compare op>:<value>

<quantity> is the name of a histogram in the given root file. For histograms, the quantity to check
           for can be given in square brackets, for regular histograms, this is one of this strings:
           "mean_x" = mean of the histogram
           "entries" = number of entries in all bins
           "mean_y" = the mean of the bin content
           "mean_y_zero_suppressed" = the mean of the bin content, only with bins with > 0 filling
<compare op> can be ">" or "<"
<value> is the reference value as a float to compare to

As an example, to identfiy the commit which made the tracking efficiency drop below 0.93, the
following snippet can be used:

git bisect good <last good release>
git bisect bad <first bad release>
git bisect run bisect_validation.py --script 13_trackingEfficiency_createPlots.py
    --check-quantity "results/current/tracking/TrackingValidation.root:hEfficiency[mean_y]:>:0.93"

 Author: The Belle II Collaboration
 Contributors: Thomas Hauth

[1]
http://git-scm.com/docs/git-bisect

"""

import os
import sys
import shutil
import subprocess
import ROOT

import argparse
from functools import reduce

import quantity_extract
import validationtools


def extract_information_from_file(file_name, results):
    """
    Read in the given root file and export the information to the results
    """

    root_file = ROOT.TFile(file_name)
    if not root_file.IsOpen():
        return None

    qe = quantity_extract.RootQuantityExtract()

    keys = root_file.GetListOfKeys()
    for graph_or_table in keys:
        name = graph_or_table.GetName()

        if name in results.keys():
            # As the naming convention is that simple in the validation output, this single line should do the job
            root_obj = root_file.Get(name)
            values = qe.extract(root_obj)

            # only fill if at least one result was found,
            # otherwise the results will stay None
            if len(values) > 0:
                results[name] = values

    root_file.Close()

    return results


def parseCheckQuantity(quantString):
    """!
    parses the quantity string, including its brackets
    """

    fragments = quantString.split(":")
    if len(fragments) == 4:
        filename = fragments[0].strip()
        quant = (fragments[1].strip('"').strip(), None)
        comp = fragments[2].strip()
        val = fragments[3].strip()

        # parse quantity further ?
        if "[" in quant[0] and "]" in quant[0]:
            splitQuant = [x for x in quant[0].replace("[", "]").split("]") if len(x) > 0]
            spiltQuant = [x.strip() for x in splitQuant]
            quant = (splitQuant[0], splitQuant[1])

    else:
        print("cannot parse quantity expression " + str(quantString))
        sys.exit(1)

    return (filename, quant, comp, val)

# setup argument parser options
parser = argparse.ArgumentParser(description='Evaluate status of the compile and validation of a '
                                             'specific basf2 revision.')
parser.add_argument('--skip-compile', action='store_true',
                    default=False,
                    help='Do not trigger a compile, useful for saving time when '
                    'setting up the quantity checks.')
parser.add_argument('--report-compile-fail', action='store_true',
                    default=False,
                    help='Report a bad revision to git if the code base did not'
                         'compile. By default, failing compiles are ignored.')
parser.add_argument(
    '--report-execution-fail',
    action='store_true',
    default=False,
    help='Report a bad revision to git if the provided script has an '
         'error code != 0. By default, this is reported to git.')
parser.add_argument('--check-quantity', action="append",
                    help='Check for a quantity in validation files')
parser.add_argument('--test-check', action='store_true', default=False,
                    help='Just check for the quantity, but do not run any git or compile commands')
parser.add_argument('--execute', action="append",
                    help='File to execute after the compile and before the quantity check')
parser.add_argument('--script', action="append",
                    help='Name of validation script to run after the compile '
                         'and before the quantity check. Only this validation script and '
                         'all scripts it depends on are executed.')
parser.add_argument('--keep', action='store_true', default=False,
                    help='Keep the result folder of each bisect step for later examination')
args = parser.parse_args()
argsVar = vars(args)

c_parsed = []
if not argsVar["check_quantity"] is None:
    for c_string in argsVar["check_quantity"]:
        c_parsed = c_parsed + [parseCheckQuantity(c_string)]

# try to get git revision
current_git_commit = None
try:
    # get git rev, decode and remove possible newline at end
    current_git_commit = subprocess.check_output(["git", "rev-parse", "HEAD"]).decode().rstrip()
except subprocess.CalledProcessError:
    current_git_commit = None
    print("Cannot get git commit hash of HEAD")

if argsVar["test_check"] is False:
    # output current git commit and svn revision
    os.system("echo -n 'git commit ' && git rev-parse HEAD")
    os.system("echo -n 'SVN revsion ' && git svn find-rev `git rev-parse HEAD`")

    # make sure the proper environment is setup. If for example
    # the externals version was changed between two commits, this is required
    validationtools.update_env()

    # see if quantities must be checked and
    # make sure the expression can be properly parsed
    if argsVar["check_quantity"] is None:
        argsVar["check_quantity"] = []

    if argsVar["skip_compile"] is False:
        print("Compiling revision ...")
        exitCode = os.system("scons -j8")
        print("Exit code of compile was " + str(exitCode))
        if exitCode > 0:
            if argsVar["report_compile_fail"]:
                # mark this as a bad revision
                sys.exit(1)
            else:
                # tell git to ignore this failed build
                sys.exit(125)

    # execute validation script
    if argsVar["script"] is None:
        argsVar["script"] = []

    validation_scripts = reduce(lambda x, y: x + " " + y, argsVar["script"], "")
    if len(validation_scripts) > 0:
        validation_call = "validate_basf2 -s " + validation_scripts
        exitCode = os.system(validation_call)

        print("Exit code of " + validation_call + " was " + str(exitCode))

        if exitCode > 0:
            if argsVar["report_execution_fail"]:
                sys.exit(1)  # tell git about this failed run ...
            else:
                sys.exit(125)  # tell git to ignore this failed validation

    # execute provided file
    if argsVar["execute"] is None:
        argsVar["execute"] = []

    for ex in argsVar["execute"]:
        print("Executing " + str(ex))
        exitCode = os.system(ex.strip('"'))
        print("Exit code of " + str(ex) + " was " + str(exitCode))

        if exitCode > 0:
            if argsVar["report_execution_fail"]:
                sys.exit(1)  # tell git about this failed run ...
            else:
                sys.exit(125)  # tell git to ignore this failed validation
else:
    print("Skipping checkout and compile, performing only check")

# preserve results folder, if requested
if argsVar["keep"] is True:
    if current_git_commit is None:
        print("Cannot retrieve git commit hash, no archival possible")
    else:
        archive_folder_name = "results_" + current_git_commit
        # remove a previously archieved folder, if present
        shutil.rmtree(archive_folder_name, True)
        shutil.copytree("results", archive_folder_name)

# perform checks
for c in c_parsed:

    print("Running check on file " + str(c[0]) + " for quantity " + str(c[1]) + " " + str(c[2]) + " " + str(c[3]))

    file_name = c[0]

    refObjKey = c[1][0]
    results = {refObjKey: None}
    results = extract_information_from_file(file_name, results)

    print("results " + str(results))
    if results is None:
        print("result file " + str(file_name) + " not found")
        print("Check failed")
        sys.exit(1)

    if results[refObjKey] is None:
        print("could not find quantity " + str(c[1]) + " in file " + str(c[0]))
        print("Check failed")
        sys.exit(1)

    if c[1][1] is None:
        valValue = results[refObjKey]
    else:
        keyName = c[1][1]
        valValue = results[refObjKey][keyName]
    print("Retrieved value " + str(valValue))

    compareResult = None
    if c[2] is ">":
        compareResult = float(valValue) > float(c[3])
    elif c[2] is "<":
        compareResult = float(valValue) < float(c[3])
    else:
        print("Compare operation " + c[2] + " not supported")
        sys.exit(0)

    if not compareResult:
        print("Check failed")
        sys.exit(1)
    else:
        print("Check successful")
        sys.exit(0)

sys.exit(0)
