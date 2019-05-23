#!/bin/env python3
# -*- coding: utf-8 -*-

"""
Script to get the integrated luminosity of a set of runs

Based on a bash script by Mikhail Remnev mikhail.remnev@desy.de
https://confluence.desy.de/display/BI/ECL+LM+Integrated+luminosity+in+Phase+3

Usage:
    b2integrated_luminosity_online.py --exp <E> --runs <Range> [--verbose] [--db db_path]

<E> experiment number (mandatory)
<Range> run range (mandatory)
    syntax is: xx-yy,zz
    all runs between xx and yy (included) and run zz. Can have multiple ranges xx-yy,zz-ww
--verbose print lumi for each run
--db path of database to use

"""

__author__ = "Stefano Lacaprara"
__email__ = "stefano.lacaprara@pd.infn.it"


import argparse
import re
from argparse import ArgumentParser, ArgumentTypeError
import sqlite3
import time
import os
import basf2 as b2


def parseNumRange(string):
    """
    Parse range of integers like xx-yy
    """
    m = re.match(r'(\d+)(?:-(\d+))?$', string)
    if not m:
        raise ArgumentTypeError("'" + string + "' is not a range of number. Expected forms like '0-5' or '2'.")
    start = m.group(1)
    end = m.group(2) or start
    return set(range(int(start, 10), int(end, 10) + 1))


def parseNumList(string):
    """
    Parse range of integers like xx-yy,zz
    """
    result = set()
    for rr in [x.strip() for x in string.split(',')]:
        result |= parseNumRange(rr)

    return list(result)


def argparser():
    """
    Parse options as command-line arguments.
    """

    description = "Script to get the integrated luminosity for a range or runs"

    # Re-use the base argument parser, and add options on top.
    parser = argparse.ArgumentParser(description=description, usage=__doc__, add_help=True)

    parser.add_argument("--exp",
                        dest="exp",
                        action="store",
                        type=int,
                        required=True,
                        help="Experiment number")

    parser.add_argument("--runs",
                        dest="runs",
                        action="store",
                        type=parseNumList,
                        required=True,
                        default=None,
                        help="Process only the selected run range. Syntax is xx-yy,zz")

    parser.add_argument("--verbose",
                        dest="verb",
                        action="store_true",
                        default=False,
                        help="Print each run luminosity")

    parser.add_argument("--db",
                        dest="db",
                        action="store",
                        default="/gpfs/group/belle2/group/detector/ECL/ecl_lom.db",
                        help="EC: DataBase path")

    return parser


if __name__ == '__main__':

    args = argparser().parse_args()

    reco = "lum_det_shift"
    DB = args.db
    if not os.path.exists(DB):
        b2.B2FATAL(f"DB {DB} not found. The live luminosity is only available on KEKCC (sorry)")

    L = 0
    conn = sqlite3.connect(DB)
    conn.row_factory = sqlite3.Row
    cursor = conn.cursor()
    cmd = f"SELECT SUM({reco}) as lumi, start_time, end_time FROM prev_bhacnt_det_run WHERE exp = ? and run = ?"
    if(args.verb):
        print('-' * 61)
        print(f"Run No   : Delivererd Lumi  | Run start [JST]")
        print('-' * 61)
    for run in args.runs:
        cursor.execute(cmd, (args.exp, run))
        row = cursor.fetchone()

        if (row['lumi']):
            if(args.verb):
                runDate = time.strftime("%a, %d %b %Y %H:%M:%S +0000", time.localtime(row['start_time']))
                print(f"Run {run:5d}: L={row['lumi']:8.2f} nb^{-1} | {runDate}")
            L += float(row['lumi'])

    conn.close()
    if(args.verb):
        print('-' * 61)

    print(f"TOTAL    : L={L:5.2f} /nb = {L/1E3:5.2f} /pb = {L/1E6:5.3f} /fb = {L/1E9:5.4f} /ab")
