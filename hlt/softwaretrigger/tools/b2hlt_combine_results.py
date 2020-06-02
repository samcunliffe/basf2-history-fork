#!/usr/bin/env python3

from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
PyConfig.StartGuiThread = False

from argparse import ArgumentParser
import os
import uproot
import root_pandas
import pandas as pd
import numpy as np

__author__ = "Sam Cunliffe"
__email__ = "sam.cunliffe@desy.de"

PRESCALE_ROW = 4


def get_parser():
    """Get the command line options

    Returns:
        argparse.ArgumentParser for this tool
    """
    parser = ArgumentParser(
        description="Combines several ``software_trigger_result`` files.")
    parser.add_argument("input", nargs='*',
                        help="Wildcard to select ``software_trigger_results`` files.")
    parser.add_argument("--output",
                        help="The combined output ``software_trigger_result`` file name.",
                        default="software_trigger_results_combined.root")
    return parser


def get_prescales(df):
    """Get prescale values from a data frame

    Returns:
        a list of the prescale values of each trigger line
    """
    prescales = []
    for col in df.columns:
        if col.find('software_trigger_cut_') >= 0 and df[col][PRESCALE_ROW] > 0:
            prescales.append(df[col][PRESCALE_ROW])
    return prescales

if __name__ == "__main__":

    args = get_parser().parse_args()

    # get input file list
    if not all([os.path.exists(f) for f in args.input]):
        raise FileNotFoundError("Could not find input files: %s" % args.input)

    # loop over SWTRs
    sum_out = pd.DataFrame()
    total_events = []  # number of total events in each data frame
    prescales = []  # prescale values of the trigger lines in each data frame
    for fi in args.input:

        # might have swtr files with no events selected: skip these
        swtr = uproot.open(fi)["software_trigger_results"].pandas.df()
        if not swtr['total_events'][0].any():
            continue

        # add up all non-zero dataframes
        if sum_out.empty:
            sum_out = swtr
        else:
            sum_out = sum_out.add(swtr)
        total_events.append(swtr['total_events'][0])
        prescales.append(get_prescales(swtr))

    # the prescale values were also added up, to get the correct prescale values back,
    # calculate the average prescale of the sum for each trigger line
    prescales = np.array(prescales)
    total_events = np.array(total_events)
    overall_total_events = np.sum(total_events)
    i = 0  # index the trigger lines

    for col in sum_out.columns:
        # loop over all trigger lines
        if col.find('software_trigger_cut_') >= 0 and sum_out[col][PRESCALE_ROW] > 0:
            average_prescale = 0
            for j, prescale in enumerate(prescales[:, i]):
                # loop over the values of each data frame and calculate an average
                # prescale value regarding the number of total events in the data frame
                average_prescale += total_events[j]/overall_total_events*prescale
            average_prescale = np.round(average_prescale, 3)
            # overwrite the wrong added up prescale value with the calculated average value
            sum_out.at[PRESCALE_ROW, col] = average_prescale
            i += 1

    root_pandas.to_root(sum_out, key='software_trigger_results', path=args.output)
    # uproot.newtree doesn't work in the current externals version but when it does this can be root free
    print("Created file %s" % args.output)
