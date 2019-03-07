
# Example script to run dEdx validation module for bhabha or radbhabha or hadron skims
# configure distination of file and set variable --> var
# Commited on : Feb 21, 2019 >> Jitendra Kumar


import basf2
import os
import sys
import ROOT
from ROOT import Belle2
from modularAnalysis import *

reset_database()
use_database_chain()
use_central_database("data_reprocessing_preproc8b")
# use_local_database("localDB/database.txt", "localDB")

ROOT.gROOT.SetBatch(True)


var = 'bhabha'  # bhabha or radbhabha or hadron

if var == 'bhabha':
    input_files = [os.path.abspath(
        '/mnt/data/BelleIICalibration/data/preprod8b/electron/bhabha/cdst/skinny_cdst.preprod8b.*18*.root')]
elif var == 'radbhabha':
    input_files = [os.path.abspath(
        '/mnt/data/BelleIICalibration/data/preprod8b/electron/radbhabha/cdst/skinny_cdst.preprod8b.*18*.root')]
elif var == 'hadron':
    input_files = [os.path.abspath('/mnt/data/BelleIICalibration/data/preprod8b/hadron/cdst/skinny_cdst.preprod8b.*18*.root')]
else:
    print("Only (rad)bhabha or hadron input files are compatible")
    exit()

mypath = basf2.Path()  # create your own path (call it what you like)


moduleA = register_module("RootInput")
moduleA.param("inputFileNames", input_files)
mypath.add_module(moduleA)


moduleB = register_module("CDCDedxValidation")
moduleB.param("fnRuns", 50)
if var == 'bhabha':
    moduleB.param("SampleType", "bhabha")
    moduleB.param("outputFileName", "bhabha_outfile.root")
elif var == 'radbhabha':
    moduleB.param("SampleType", "radbhabha")
    moduleB.param("outputFileName", "radbhabha_outfile.root")
elif var == 'hadron':
    moduleB.param("SampleType", "hadron")
    moduleB.param("outputFileName", "hadron_outfile.root")
else:
    print("Only (rad)bhabha or hadron input files are compatible")
    exit()

mypath.add_module(moduleB)

basf2.process(mypath)

print(basf2.statistics)
