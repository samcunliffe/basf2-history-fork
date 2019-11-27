#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
"""

import argparse
from pathlib import Path
import re
import subprocess
import sys

from ROOT import PyConfig

# Importing ROOT in skimExpertFunctions has the side-effect of hijacking argument parsing
PyConfig.IgnoreCommandLineOptions = True
from b2test_utils import clean_working_directory
from basf2 import find_file
from skim.registry import skim_registry, combined_skims
from skimExpertFunctions import get_test_file


nTestEvents = 10000


def getAllSamples(mcCampaign):
    mcSamples = [
        f'{mcCampaign}_mixedBGx1',
        f'{mcCampaign}_chargedBGx1',
        f'{mcCampaign}_ccbarBGx1',
        f'{mcCampaign}_uubarBGx1',
        f'{mcCampaign}_ddbarBGx1',
        f'{mcCampaign}_ssbarBGx1',
        f'{mcCampaign}_taupairBGx1',
        f'{mcCampaign}_mixedBGx0',
        f'{mcCampaign}_chargedBGx0',
        f'{mcCampaign}_ccbarBGx0',
        f'{mcCampaign}_uubarBGx0',
        f'{mcCampaign}_ddbarBGx0',
        f'{mcCampaign}_ssbarBGx0',
        f'{mcCampaign}_taupairBGx0',
    ]

    dataSamples = ['proc9_exp3', 'proc9_exp7', 'proc9_exp8', 'bucket7_exp8']

    return mcSamples, dataSamples


class CustomHelpFormatter(argparse.HelpFormatter):
    """Custom formatter for argparse, to print the valid choices for an argument
    in the help string.
    """
    def _get_help_string(self, action):
        if action.choices:
            return action.help + ' Valid options are: ' + ', '.join(action.choices)
        else:
            return action.help


def getArgumentParser():
    """

    Returns:
        parser (argparse.ArgumentParser):
    """
    allStandaloneSkims = [skim for _, skim in skim_registry]
    allCombinedSkims = list(combined_skims.keys())

    parser = argparse.ArgumentParser(description='A script to run a given set of skims, and ' +
                                     'save the output in a format to be read by ``printSkimStats.py``. ' +
                                     'One or more standalone or combined skim names must be provided.',
                                     formatter_class=CustomHelpFormatter)
    parser.add_argument('-s', '--standalone', nargs='+', default=[],
                        choices=['all']+allStandaloneSkims, metavar='SKIM',
                        help='List of standalone skims to run.')
    parser.add_argument('-c', '--combined', nargs='+', default=[],
                        choices=['all']+allCombinedSkims, metavar='SKIM',
                        help='List of combined skims to run.')
    parser.add_argument('--mccampaign', default='MC12',
                        choices=['MC9', 'MC10', 'MC11', 'MC12'],
                        help='The MC campaign to test on.')

    sampleGroup = parser.add_mutually_exclusive_group()
    sampleGroup.add_argument('--mconly', action='store_true',
                             help='Test on only MC samples.')
    sampleGroup.add_argument('--dataonly', action='store_true',
                             help='Test on only data samples.')

    return parser


def getSkimsAndScriptsToRun(parser, standaloneList, combinedList):
    """
    """
    allStandaloneSkims = [skim for _, skim in skim_registry]
    allCombinedSkims = list(combined_skims.keys())

    if not (standaloneList or combinedList):
        parser.print_help()
        sys.exit(1)

    if standaloneList == ['all']:
        standaloneSkims = allStandaloneSkims
    else:
        standaloneSkims = standaloneList

    if combinedList == ['all']:
        combinedSkims = allCombinedSkims
    else:
        combinedSkims = combinedList

    standaloneScripts = [find_file(f'skim/standalone/{skim}_Skim_Standalone.py', silent=True) for skim in standaloneSkims]
    combinedScripts = [find_file(f'skim/combined/{skim}_Skim_Standalone.py', silent=True) for skim in combinedSkims]

    skims = standaloneSkims + combinedSkims
    scripts = standaloneScripts + combinedScripts

    return skims, scripts


def getSamplesToRun(mcSamples, dataSamples, mcOnly=False, dataOnly=False):
    """
    """
    if mcOnly:
        return mcSamples
    elif dataOnly:
        return dataSamples
    else:
        return mcSamples + dataSamples


def submitJobs(skims, scripts, samples):
    """"""
    logDirectory = Path('log').resolve()
    logDirectory.mkdir(parents=True, exist_ok=True)

    for skim, script in zip(skims, scripts):
        if not script:
            print(f'Error! Could not find script for {skim} skim.', file=sys.stderr)
            continue

        jobIDs = []
        returnCodes = []

        for sample in samples:
            sampleFile = get_test_file(sample)

            logFile = Path(logDirectory, f'{skim}_{sample}.out')
            errFile = Path(logDirectory, f'{skim}_{sample}.err')
            jsonFile = Path(logDirectory, f'JobInformation_{skim}_{sample}.json')

            with clean_working_directory():
                process = subprocess.run(['bsub', '-q', 'l', '-oo', logFile, '-e', errFile,
                                          '-J', f'{skim} {sample}'
                                          'basf2', script, '--job-information', jsonFile,
                                          '-n', str(nTestEvents), '-i', sampleFile],
                                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            jobIDs.append(re.findall('\d+', str(process.stdout))[0])
            returnCodes.append(process.returncode)

        if any(returnCodes):
            print(f'An error occurred while submitting jobs for {skim} skim with script {script}.')
        else:
            print(f'Running {script} on {nTestEvents} events from test samples of {", ".join(samples)}. Job IDs:\n  ' +
                  '\n  '.join(jobIDs))

if __name__ == '__main__':
    parser = getArgumentParser()
    args = parser.parse_args()

    mcSamples, dataSamples = getAllSamples(args.mccampaign)
    skims, scripts = getSkimsAndScriptsToRun(parser, args.standalone, args.combined)
    samples = getSamplesToRun(mcSamples, dataSamples, args.mconly, args.dataonly)

    submitJobs(skims, scripts, samples)
