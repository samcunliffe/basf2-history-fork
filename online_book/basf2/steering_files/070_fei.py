#!/usr/bin/env python3

import sys
import basf2 as b2
import fei
import modularAnalysis as ma
from variables import variables as vm

# get input file number from the command line
filenumber = sys.argv[1]

# create path
main = b2.Path()

# load input data from mdst/udst file
filedirectory = "/group/belle2/users/tenchini/prerelease-05-00-00a/charged"
ma.inputMdst(
    environmentType="default",
    filename=f"{filedirectory}/charged_eph3_BGx0_{filenumber}.root",
    path=main,
)

# Add the database with the classifier weight files for the FEI
# You can use the command b2conditionsdb-recommend
b2.conditions.globaltags = ["analysis_tools_release-04-02"]

# Get FEI default channels.
# Utilise the arguments to toggle on and off certain channels
particles = fei.get_default_channels(
    chargedB=True,
    neutralB=False,
    hadronic=True,
    semileptonic=False,
    baryonic=True,
)

# Set up FEI configuration specifying the FEI prefix
configuration = fei.FeiConfiguration(
    prefix="FEIv4_2020_MC13_release_04_01_01", monitor=False
)

# Get FEI path
feistate = fei.get_path(particles, configuration)

# Add FEI path to the path to be processed
main.add_path(feistate.path)

# Add MC matching when applying to MC.
# This is required for variables like isSignal and mcErrors below
ma.matchMCTruth(list_name="B+:generic", path=main)

# Rank B+ candidates by signal classifier output
ma.rankByHighest(
    particleList="B+:generic",
    variable="extraInfo(SignalProbability)",
    outputVariable="FEIProbabilityRank",
    path=main,
)
vm.addAlias("FEIProbRank", "extraInfo(FEIProbabilityRank)")

vm.addAlias("SigProb", "extraInfo(SignalProbability)")
vm.addAlias("decayModeID", "extraInfo(decayModeID)")

# Store tag-side variables of interest.
ma.variablesToNtuple(
    "B+:generic",
    [
        "Mbc",
        "deltaE",
        "mcErrors",
        "SigProb",
        "decayModeID",
        "FEIProbRank",
        "isSignal",
    ],
    filename="B_charged_hadronic.root",
    path=main,
)

# Process events
b2.process(main)
print(b2.statistics)
