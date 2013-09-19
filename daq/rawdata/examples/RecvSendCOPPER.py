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
import sys
argvs = sys.argv

if len(argvs) != 4:
    print 'Usage : RecvSendCOPPER.py <COPPER hostname> <COPPER nodeID> <bit flag of FINNESEs>'
    sys.exit()

# Set the log level to show only error and fatal messages
set_log_level(LogLevel.ERROR)
set_log_level(LogLevel.INFO)

# Modules
# reader = register_module('HSLBReaderArray')
reader = register_module('DeSerializerCOPPER')
sender = register_module('Serializer')

# TxSocket
reader.param('NodeID', int(argvs[2]))
reader.param('FinnesseBitFlag', int(argvs[3]))

sender.param('DestPort', 33000)
# sender.param('LocalHostName', 'cpr006')
sender.param('LocalHostName', argvs[1])
sender.param('ProcessMethod', 'COPPER')

# reader.param('DumpFileName', 'COPPERdump.dat' )

# Create main path
main = create_path()

# Add modules to main path
main.add_module(reader)
# main.add_module(sender)

# Process all events
process(main)
