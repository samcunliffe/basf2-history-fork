#!/usr/bin/env python3
# -*- coding: utf-8 -*-

######################################################
# This steering file shows how to run the framework
# for different experiment, run and event numbers.
#
# In the example below, basf2 will run on and display
# the following experiment, run and event numbers:
#
# Experiment 71, Run  3, 4 Events
# Experiment 71, Run  4, 6 Events
# Experiment 73, Run 10, 2 Events
# Experiment 73, Run 20, 5 Events
# Experiment 73, Run 30, 3 Events
#
# Example steering file - 2011 Belle II Collaboration
######################################################

from basf2 import *

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
# set_log_level(LogLevel.INFO)

# input
# input = register_module('SeqRootInput')
# input.param('iutputFileName', 'RootOutput1.root')
input = register_module('RootInput')

# output
output = register_module('PrintData')

# Histo Module
# histo = register_module('DqmHistoManager')
histo = register_module('HistoManager')
# histo.param('HostName', 'ropc01')
# histo.param('Port', 9991)
# histo.param('DumpInterval', 10)
# histo.param('HistoFileName', 'ropc_histofile.root')

# Monitor module
monitor = register_module('MonitorDataCOPPER')

# Create main path
main = create_path()

# Add modules to main path
main.add_module(input)
main.add_module(histo)
main.add_module(monitor)
# main.add_module(output)

# Process all events
process(main)
