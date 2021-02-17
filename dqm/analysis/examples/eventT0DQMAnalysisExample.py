# !/usr/bin/env python3
# -*- coding: utf-8 -*-

# illustrative run:
# basf2 eventT0DQMAnalysisEfficiency.py dqm_e0014r000921.root 100

from basf2 import *
import sys
import re
from basf2 import conditions as b2conditions

mypath = Path()
inputFile = sys.argv[1]
exp_nr = int(re.findall(r'\d+', inputFile)[0])
run_nr = int(re.findall(r'\d+', inputFile)[1])
nevt = int(sys.argv[2])  # number of events

# setup database
b2conditions.reset()
b2conditions.override_globaltags()
b2conditions.globaltags = ["online"]

inroot = register_module('DQMHistAnalysisInputRootFile')
inroot.param('FileList', inputFile)
inroot.param('SelectHistograms', ['EventT0*/*'])
inroot.param('Experiment', exp_nr)
inroot.param('RunList', [run_nr])
inroot.param('EventsList', [nevt])
mypath.add_module(inroot)

dqmEff = register_module('DQMHistAnalysisEventT0')
dqmEff.set_log_level(LogLevel.INFO)
dqmEff.param("printCanvas", True)
mypath.add_module(dqmEff)

outroot = register_module('DQMHistAnalysisOutputMonObj')
outroot.param('ProcID', 'online')  # set processing ID
outroot.param('exp', exp_nr)
outroot.param('run', run_nr)
outroot.param('nevt', nevt)
outroot.param('TreeFile', 'run_tree_eventT0.root')
mypath.add_module(outroot)

# Process the events
print_path(mypath)
process(mypath)

# print out the summary
print(statistics)
