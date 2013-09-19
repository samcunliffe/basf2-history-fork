#!/usr/bin/env python
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

# Modules

receiver = register_module('DeSerializerPC')
# dump = register_module('PrintCollections')
dump = register_module('RootOutput')
perf = register_module('DAQPerf')

# RxSocket
receiver.param('NodeID', 3)
receiver.param('NumConn', 1)
receiver.param('HostNameFrom', ['localhost', 'cpr007'])
# receiver.param('HostNameFrom', ['cpr006', 'cpr007'])

receiver.param('PortFrom', [35000, 36000])
# receiver.param('PortFrom', [33000, 33000])

receiver.param('EventDataBufferWords', 4801)
receiver.param('MaxTime', 30.)
# receiver.param('MaxEventNum', 30.)
dump.param('outputFileName', 'root_output.root')
dump.param('compressionLevel', 0)
# Compression Level: 0 for no, 1 for low, 9 for high compression. Level 1 usually reduces size by 50%, higher levels have no noticeable effect.

# receiver.param('DumpFileName', 'ROPC01dump.dat' )

# Perf
perf.param('Cycle', 100000)
perf.param('MonitorSize', True)

# Create main path
main = create_path()

# Add modules to main path
main.add_module(receiver)
main.add_module(dump)
# main.add_module(perf)

# Process all events
process(main)
