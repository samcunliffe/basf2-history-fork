#!/usr/bin/env python3

"""
Test track isolation variables.
"""

__author__ = 'Marco Milesi'
__copyright__ = 'Copyright 2020 - Belle II Collaboration'
__maintainer__ = 'Marco Milesi'
__email__ = 'marco.milesi@unimelb.edu.au'


import argparse


def argparser():

    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument("--detectors",
                        type=str,
                        nargs="+",
                        default=["CDC", "PID", "ECL", "KLM"],
                        choices=["CDC", "PID", "ECL", "KLM"],
                        help="List of detectors at whose entry surface track isolation variables will be calculated.\n"
                        "Pass a space-separated list of names.")
    parser.add_argument("-d", "--debug",
                        action="store",
                        default=0,
                        type=int,
                        choices=list(range(11, 20)),
                        help="Run the TrackIsoCalculator module in debug mode. Pass the desired DEBUG level integer.")

    return parser


if __name__ == '__main__':

    args = argparser().parse_args()

    # Command line arguments are parsed before importing basf2, to avoid PyROOT hijacking them.
    # in case of overlapping option names.

    import basf2 as b2
    import modularAnalysis as ma
    from variables import variables

    # Create path. Register necessary modules to this path.
    path = b2.create_path()

    # Add input data and ParticleLoader modules to the path.
    ma.inputMdstList("default",
                     filelist=[b2.find_file("mdst13.root", "validation")],
                     path=path)

    # Fill a particle list of charged stable particles (eg. pions).
    # Apply (optionally) some quality selection.
    ma.fillParticleList("pi+:all", "", path=path)
    ma.applyCuts("pi+:all", "abs(dr) < 2.0 and abs(dz) < 5.0 and p > 0.1", path=path)

    ma.printList("pi+:all", False, path=path)
    # Get particle statistics at the end of the job: number of events with at
    # least one candidate, average number of candidates per event, etc.
    # ma.summaryOfLists(["pi+:all"], path=path)

    # # Calculate the mininmal distance at a given detector surface among all
    # # track candidates in the input particle list.

    ma.calculateTrackIsolation("pi+:all", path, *args.detectors, alias="dist3DToClosestTrkAtSurface")
    ma.calculateTrackIsolation("pi+:all", path, *args.detectors, use2DRhoPhiDist=True, alias="dist2DRhoPhiToClosestTrkAtSurface")

    ntup_vars = [f"dist3DToClosestTrkAtSurface{det}" for det in args.detectors]
    ntup_vars += [f"dist2DRhoPhiToClosestTrkAtSurface{det}" for det in args.detectors]

    # Optionally activate debug mode for the TrackIsoCalculator module(s)
    if args.debug:
        for m in path.modules():
            if "TrackIsoCalculator" in m.name():
                m.set_log_level(b2.LogLevel.DEBUG)
                m.set_debug_level(args.debug)

    # Dump isolation variables in a ntuple.
    ma.variablesToNtuple("pi+:all",
                         ntup_vars,
                         treename="pi",
                         filename="TrackIsolationVariables.root",
                         path=path)

    path.add_module("Progress")

    # Process the data
    b2.process(path)

    print(b2.statistics)
