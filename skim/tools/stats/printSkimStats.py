#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""

"""


__author__ = "Phil Grace, Racha Cheaib"
__email__ = "philip.grace@adelaide.edu.au, rachac@mail.ubc.ca"


import argparse
from functools import lru_cache
import json
import pandas as pd
from pathlib import Path
import re
import sys
from textwrap import wrap

from ROOT import PyConfig
from tabulate import tabulate

# Importing ROOT in skimExpertFunctions has the side-effect of hijacking argument parsing
PyConfig.IgnoreCommandLineOptions = True
from skim.registry import skim_registry, combined_skims
from skimExpertFunctions import get_test_file, get_eventN, get_total_infiles, get_events_per_file


# TODO: put these into a function for documentation purposes
beamBackgroundWeights = {
    'BGx1': 0.8,
    'BGx0': 0.2
}

mcSampleCrossSections = {
    'mixed': 0.555,
    'charged': 0.555,
    'ccbar': 1.3,
    'uubar': 1.61,
    'ddbar': 0.40,
    'ssbar': 0.38,
    'taupair': 0.91
}

# TODO: put these into a function for documentation purposes
mcCampaign = 'MC12'

mcSamples = {
    f'{mcCampaign}_mixedBGx1': f'{mcCampaign}: mixed BGx1',
    f'{mcCampaign}_chargedBGx1': f'{mcCampaign}: charged BGx1',
    f'{mcCampaign}_ccbarBGx1': f'{mcCampaign}: ccbar BGx1',
    f'{mcCampaign}_uubarBGx1': f'{mcCampaign}: uubar BGx1',
    f'{mcCampaign}_ddbarBGx1': f'{mcCampaign}: ddbar BGx1',
    f'{mcCampaign}_ssbarBGx1': f'{mcCampaign}: ssbar BGx1',
    f'{mcCampaign}_taupairBGx1': f'{mcCampaign}: taupair BGx1',
    f'{mcCampaign}_mixedBGx0': f'{mcCampaign}: mixed BGx0',
    f'{mcCampaign}_chargedBGx0': f'{mcCampaign}: charged BGx0',
    f'{mcCampaign}_ccbarBGx0': f'{mcCampaign}: ccbar BGx0',
    f'{mcCampaign}_uubarBGx0': f'{mcCampaign}: uubar BGx0',
    f'{mcCampaign}_ddbarBGx0': f'{mcCampaign}: ddbar BGx0',
    f'{mcCampaign}_ssbarBGx0': f'{mcCampaign}: ssbar BGx0',
    f'{mcCampaign}_taupairBGx0': f'{mcCampaign}: taupair BGx0'
}

dataSamples = {
    'proc9_exp3': 'Data: proc9 exp. 3',
    'proc9_exp7': 'Data: proc9 exp. 7',
    'proc9_exp8': 'Data: proc9 exp. 8',
    'bucket7_exp8': 'Data: bucket7 exp. 8'
}

samples = list(mcSamples.keys()) + list(dataSamples.keys())


class SkimNotRunException(Exception):
    """An exception to be raised whenever an error occurs that is likely due to
    a skim not being run properly by ``runSkimsForStats.py``.
    """
    pass


def getArgumentParser():
    """

    Returns:
        parser (argparse.ArgumentParser):
    """
    allStandaloneSkims = [skim for _, skim in skim_registry]
    allCombinedSkims = list(combined_skims.keys())

    parser = argparse.ArgumentParser(description='A script to print tables of statistics for skims ' +
                                     'which have been run by runSkimsForStats.py. One or more standalone or combined ' +
                                     'skim names must be provided.')
    parser.add_argument('-s', '--standalone', nargs='+', default=[],
                        choices=['all']+allStandaloneSkims, metavar='SKIM',
                        help='List of standalone skims to run. Valid options are: ' +
                        ', '.join([f'``{s}``' for s in allStandaloneSkims]) +
                        ', or ``all`` to run all standalone skims.')
    parser.add_argument('-c', '--combined', nargs='+', default=[],
                        choices=['all']+allCombinedSkims, metavar='SKIM',
                        help='List of combined skims to run. Valid options are: ' +
                        ', '.join([f'``{s}``' for s in allCombinedSkims]) +
                        ', or ``all`` to run all combined skims.')

    return parser


def getStatFromLog(statisticName, logFileContents):
    """Search for a given statistic in the "Resource usage summary" section of the log file.

    Args:
        statisticName (str): The name of the value as it appears in the log file.
        logFileContents (str): A string containing the contents of the log file
            of a skim script.

    Returns:
        statFromLog (float):
    """
    floatRegexp = r'\s*:\s+(\d+(\.(\d+)?)?)'
    statFromLog = re.findall(f'{statisticName}{floatRegexp}', logFileContents)[0][0]

    return float(statFromLog)


def nInputEvents(jsonFileContents):
    """

    Args:
        jsonFileContents (dict): A dict read from the JobInformation JSON output
            of a skim script.

    Returns:
        nInputEvents (int):
    """
    return jsonFileContents['basf2_status']['total_events']


def nSkimmedEvents(jsonFileContents):
    """

    Args:
        jsonFileContents (dict): A dict read from the JobInformation JSON output
            of a skim script.

    Returns:
        nSkimmedEvents (int):
    """
    return jsonFileContents['output_files'][0]['stats']['events']


def udstSize(jsonFileContents):
    """Return the size of the output uDST file in KB, read from Job Information JSON file.

    Args:
        jsonFileContents (dict): A dict read from the JobInformation JSON output
            of a skim script.

    Returns:
        udstSize (float):
    """
    return jsonFileContents['output_files'][0]['stats']['filesize_kib']


def logSize(logFileContents):
    """Return the size of the log file in kB.

    Args:
        logFileContents (str): A string containing the contents of the log file
            of a skim script.

    Returns:
        logSize (float):
    """
    return len(logFileContents) / 1024


def cpuTime(logFileContents):
    """

    Args:
        logFileContents (str): A string containing the contents of the log file
            of a skim script.

    Returns:
        cpuTime (float):
    """
    return getStatFromLog('CPU time', logFileContents)


def averageCandidateMultiplicity(logFileContents):
    """

    Args:
        logFileContents (str): A string containing the contents of the log file
            of a skim script.

    Returns:
        averageCandidateMultiplicity (float):
    """
    floatRegexp = r'\d+\.\d+'

    multiplicityBlock = re.findall('(Average Candidate Multiplicity(.*\n)+?.*INFO)', logFileContents)[0][0]
    multiplicityLines = [line for line in multiplicityBlock.split('\n') if re.findall(floatRegexp, line)]
    multiplicities = [float(re.findall(floatRegexp, line)[1]) for line in multiplicityLines]

    averageMultiplicity = sum(multiplicities)/len(multiplicities)

    return averageMultiplicity


def memoryAverage(logFileContents):
    """

    Args:
        logFileContents (str): A string containing the contents of the log file
            of a skim script.

    Returns:
       memoryAverage (float):
    """
    return getStatFromLog('Average Memory', logFileContents)


def memoryMaximum(logFileContents):
    """

    Args:
        logFileContents (str): A string containing the contents of the log file
            of a skim script.

    Returns:
        memoryMaximum (float):
    """
    return getStatFromLog('Max Memory', logFileContents)


@lru_cache()
def nEventsPerFile(sample):
    """

    Args:
        sample (str): The label of the sample being tested.

    Returns:
        nEventsPerFile (int):
    """
    parentFile = get_test_file(sample)
    return get_eventN(parentFile)


def nTotalFiles(sample):
    """

    Args:
        sample (str): The label of the sample being tested.

    Returns:
        nTotalFiles (int):
    """
    return get_total_infiles(sample)


def nTotalEvents(sample):
    """

    Args:
        sample (str): The label of the sample being tested.

    Returns:
        nTotalEvents (int):
    """
    return nEventsPerFile(sample)*nTotalFiles(sample)


def getJobOutput(skim, sample):
    """

    Args:
        skim (str): The name of the skim being tested.
        sample (str): The label of the sample being tested.

    Returns:
        logFileContents (str): The contents of the log file.
        jsonFileContents (dict): A dict read from the JobInformation JSON output
            of a skim script.

    Raises:
        SkimNotRunException: Raised if any of the log or JSON files cannot be opened.
    """
    try:
        logFileName = Path('log', f'{skim}_{sample}.out')
        with open(logFileName) as logFile:
            logFileContents = logFile.read()

        jsonFileName = Path('log', f'JobInformation_{skim}_{sample}.json')
        with open(jsonFileName) as jsonFile:
            jsonFileContents = json.load(jsonFile)
    except FileNotFoundError:
        raise SkimNotRunException(f'    Failed to open output files for {skim} skim on {sample} sample.\n' +
                                  '    Perhaps you forgot to run the skim with runSkimsForStats.py?')

    return logFileContents, jsonFileContents


def testLogContents(logFileContents, jsonFileContents, skim, sample):
    """

    Args:
        logFileContents (str):  A string containing the contents of the log file
            of a skim script.
        jsonFileContents (dict): A dict read from the JobInformation JSON output
            of a skim script.
        skim (str): The name of the skim being tested.
        sample (str): The label of the sample being tested.

    Raises:
        SkimNotRunException: Raised if the log file indicates the job did not finish
            successfully, or the JSON file indicates the output files were not
            written correctly.
    """
    logTests = ['Successfully completed' in logFileContents]
    jsonTests = [jsonFileContents['basf2_status']['finished'],
                 jsonFileContents['basf2_status']['success'],
                 jsonFileContents['basf2_status']['errors'] == 0,
                 jsonFileContents['basf2_status']['fatals'] == 0,
                 all(check for check in jsonFileContents['output_files'][0]['checks_passed'].values())
                 ]

    if not all(logTests) and all(jsonTests):
        raise SkimNotRunException(f'    Error found in log files of {skim} skim on {sample} sample.\n' +
                                  '    Please check the .out, .err, and .json files in log/ directory.')


def getSkimsToRun(parser, standaloneList, combinedList):
    """

    Returns:
        skimsToRun (list):
    """
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

    return standaloneSkims + combinedSkims


def getSkimStatsDict(skims, samples, statSpecifier):
    """
    If ``SkimNotRunException`` is raised, this is caught and the dict entries for
    the skim are removed. An error messsage is printed, and execution continues.

    Args:
        skims (list): List of skims to run.
        samples (list): List of sample labels to test on.
        statSpecifier (dict): A nested dict specifying how each statistic should
            be calculated and printed, as returned by `getStatSpecifier`.

    Returns:
        skimStatsDict (dict): A nested dict indexed as ``skim name::statistic
            label::sample label``.
    """
    allSkimStats = {skim: {stat: {} for stat in statSpecifier.keys()} for skim in skims}

    for skim in skims:
        try:
            for sample in samples:
                logFileContents, jsonFileContents = getJobOutput(skim, sample)

                testLogContents(logFileContents, jsonFileContents, skim, sample)

                for statName, statInfo in statSpecifier.items():
                    statFunction = statInfo['Calculate']

                    try:
                        allSkimStats[skim][statName][sample] = statFunction(jsonFileContents, logFileContents, sample)
                    except TypeError:
                        allSkimStats[skim][statName][sample] = None
        except SkimNotRunException as e:
            del allSkimStats[skim]

            print(f'Error! Could not get stats for {skim}. Details:', file=sys.stderr)
            print(e, '\n', file=sys.stderr)

    return allSkimStats


def mcWeightedAverage(statsPerSample):
    """

    Args:
        statsPerSample (dict):

    Returns:
        mcWeightedAverage (float):
    """
    totalCrossSection = sum(mcSampleCrossSections.values())

    weightedAverage = 0
    for mcSample, crossSection in mcSampleCrossSections.items():
        for beamBackground, beamBackgroundWeight in beamBackgroundWeights.items():
            sample = f'{mcCampaign}_{mcSample}{beamBackground}'
            weightedAverage += statsPerSample[sample]*beamBackgroundWeight*crossSection/totalCrossSection

    return weightedAverage


def addWeightedMC(allSkimStats, statSpecifier):
    """

    Args:
        allSkimStats (dict):
        statSpecifier (dict):

    Returns:
        allSkimStatsWithWeightedMC (dict):
    """
    for skimStats in allSkimStats.values():
        for statName, statInfo in statSpecifier.items():
            try:
                combiningFunction = statInfo['CombineMC']
                skimStats[statName]['Combined MC'] = combiningFunction(skimStats[statName])
            except TypeError:
                skimStats[statName]['Combined MC'] = None

    return allSkimStats


def printToJson(allSkimStats):
    """

    Args:
        allSkimStats (dict):
    """
    outputJsonName = 'skimStats.json'
    with open(outputJsonName, 'w') as outputJson:
        json.dump(allSkimStats, outputJson, indent=4)
    print(f'Wrote stats to JSON file {outputJsonName}.')


def printToScreen(allSkimStats):
    """

    Args:
        allSkimStats (dict):
    """
    for skimName, skimStats in allSkimStats.items():
        print(f'Performance statistics for {skimName} skim:')

        # Only print some stats to screen
        selectedStats = [stat for (stat, statInfo) in statSpecifier.items() if statInfo['PrintToScreen']]
        df = pd.DataFrame(skimStats, columns=selectedStats)

        df = df.reindex([*dataSamples.keys(), 'Combined MC', *mcSamples.keys()])
        df = df.rename(index={**dataSamples, **mcSamples})

        headers = ['\n'.join(wrap(statSpecifier[stat]['LongName'], 12)) for stat in selectedStats]
        floatFormat = [''] + [statSpecifier[stat]['FloatFormat'] for stat in selectedStats]

        table = tabulate(df[selectedStats],
                         headers=headers, tablefmt="fancy_grid",
                         numalign='right', stralign='left',
                         floatfmt=floatFormat)
        table = table.replace(' nan ', ' -   ')
        print(table)


def printToConfluence(allSkimStats):
    """

    Args:
        allSkimStats (dict):
    """
    confluenceStrings = []
    for skimName, skimStats in allSkimStats.items():
        confluenceStrings += [f'h1. Performance statistics for {skimName} skim']

        selectedStats = [stat for (stat, statInfo) in statSpecifier.items() if statInfo['PrintToConfluence']]
        df = pd.DataFrame(skimStats, columns=selectedStats)

        # Set up row ordering and naming
        df = df.reindex([*dataSamples.keys(), 'Combined MC', *mcSamples.keys()])
        df = df.rename(index={**dataSamples, **mcSamples})

        headers = [statSpecifier[stat]['LongName'] for stat in selectedStats]
        floatFormat = [''] + [statSpecifier[stat]['FloatFormat'] for stat in selectedStats]

        table = tabulate(df, headers=headers, tablefmt="jira", floatfmt=floatFormat)

        # Make the first column (the sample label) bold on Confluence
        table = re.sub(r'^\| ', '|| ', table, flags=re.MULTILINE)
        table = table.replace(' nan ', ' --  ')

        confluenceStrings += [table]

    confluenceString = '\n'.join(confluenceStrings)

    confluenceFileName = 'skimStats_confluence.txt'
    with open(confluenceFileName, 'w') as confluenceFile:
        confluenceFile.write(confluenceString)
    print(f'Wrote tables to {confluenceFileName}. The contents of this file can be copied directly to Confluence.')


def getStatSpecifier():
    """Returns a nested dict containing the specifications for how each
    statistic should be calculated and printed. The keys of the dict are used
    internally by this script for constructing the tables of stats. The entries
    of the dicts are as follows:

    * ``'LongName'`` (`str`): The label to use in the Confluence table. Should
      include units of the statistic.
    * ``'FloatFormat'`` (`str`): A printf string for how the number should be printed.
    * ``'PrintToScreen'`` (`bool`): Determines whether the statistic is included
      in the table in terminal output.
    * ``'PrintToConfluence'`` (`bool`): Determines whether the statistic is
      included in the Confluence table.
    * ``'Calculate'`` (`function`): An anonymous function for how to calculate
      the statistic. The arguments of this function must be a dict constructed
      from the JobInformation JSON output, a string containing the log file
      contents, and the name of the sample being used. Not all of these
      arguments must be used by the function, but they must all be supplied.
    * ``'CombineMC'`` (`function`): An anonymous function for how this
      statistics should be combined across samples to obtain an estimate for a
      cross-section weighted MC sample. Can be `None` if there is no sensible way
      to combine this statistic for different MC samples.

    Returns:
        statSpecifier (dict): A nested dict specifying how each
        statistic should be handled.
    """
    statSpecifier = {
        'RetentionRate': {
            'LongName': 'Retention rate (%)',
            'FloatFormat': '.2f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda json, *_: 100 * nSkimmedEvents(json) / nInputEvents(json),
            'CombineMC': lambda statDict: mcWeightedAverage(statDict)
        },
        'nInputEvents': {
            'LongName': 'Number of input events of test',
            'FloatFormat': '.0f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda json, *_: nInputEvents(json),
            'CombineMC': None
        },
        'nSkimmedEvents': {
            'LongName': 'Number of skimmed events',
            'FloatFormat': '.0f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda json, *_: nSkimmedEvents(json),
            'CombineMC': None
        },
        'cpuTime': {
            'LongName': 'CPU time of test on KEKCC (s)',
            'FloatFormat': '.1f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda _, log, __: cpuTime(log),
            'CombineMC': None
        },
        'cpuTimePerEvent': {
            'LongName': 'CPU time per event on KEKCC (s)',
            'FloatFormat': '.3f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda json, log, *_: cpuTime(log) / nInputEvents(json),
            'CombineMC': lambda statDict: mcWeightedAverage(statDict)
        },
        'udstSize': {
            'LongName': 'uDST size of test (MB)',
            'FloatFormat': '.2f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda json, *_: udstSize(json) / 1024,
            'CombineMC': None
        },
        'udstSizePerEvent': {
            'LongName': 'uDST size per event (kB)',
            'FloatFormat': '.3f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda json, *_: udstSize(json) / nInputEvents(json),
            'CombineMC': lambda statDict: mcWeightedAverage(statDict)
        },
        'udstSizePerFile': {
            'LongName': 'Estimated average uDST size per file (MB)',
            'FloatFormat': '.2f',
            'PrintToScreen': False,
            'PrintToConfluence': False,
            'Calculate': lambda json, _, sample: udstSize(json) / nInputEvents(json) * get_events_per_file(sample) / 1024,
            'CombineMC': lambda statDict: mcWeightedAverage(statDict)
        },
        'logSize': {
            'LongName': 'Log size of test (kB)',
            'FloatFormat': '.1f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda _, log, __: logSize(log),
            'CombineMC': None
        },
        'logSizePerEvent': {
            'LongName': 'Log size per event (B)',
            'FloatFormat': '.2f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda json, log, __: logSize(log) / nInputEvents(json) * 1024,
            'CombineMC': lambda statDict: mcWeightedAverage(statDict)
        },
        'averageCandidateMultiplicity': {
            'LongName': 'Average candidate multiplicity of passed events',
            'FloatFormat': '.2f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda _, log, __: averageCandidateMultiplicity(log),
            'CombineMC': lambda statDict: mcWeightedAverage(statDict)
        },
        'memoryAverage': {
            'LongName': 'Average memory usage (MB)',
            'FloatFormat': '.0f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda _, log, __: memoryAverage(log),
            'CombineMC': lambda statDict: mcWeightedAverage(statDict)
        },
        'memoryMaximum': {
            'LongName': 'Maximum memory usage (MB)',
            'FloatFormat': '.0f',
            'PrintToScreen': True,
            'PrintToConfluence': True,
            'Calculate': lambda _, log, __: memoryMaximum(log),
            'CombineMC': lambda statDict: max(statDict.values())
        },
        'udstSizePerEntireSample': {
            'LongName': 'Estimated uDST size for entire sample (GB)',
            'FloatFormat': '.2f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda json, _, sample: udstSize(json) * nTotalEvents(sample) / nInputEvents(json) / 1024 / 1024,
            'CombineMC': lambda statDict: sum(statDict.values())
        },
        'logSizePerEntireSample': {
            'LongName': 'Estimated log size for entire sample (GB)',
            'FloatFormat': '.2f',
            'PrintToScreen': False,
            'PrintToConfluence': True,
            'Calculate': lambda json, log, sample: logSize(log) * nTotalEvents(sample) / nInputEvents(json) / 1024 / 1024,
            'CombineMC': lambda statDict: sum(statDict.values())
        }
    }

    return statSpecifier

if __name__ == '__main__':
    parser = getArgumentParser()
    args = parser.parse_args()

    skims = getSkimsToRun(parser, args.standalone, args.combined)

    statSpecifier = getStatSpecifier()
    allSkimStats = getSkimStatsDict(skims, samples, statSpecifier)
    allSkimStats = addWeightedMC(allSkimStats, statSpecifier)

    printToScreen(allSkimStats)
    printToJson(allSkimStats)
    printToConfluence(allSkimStats)
