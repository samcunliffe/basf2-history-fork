#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

import beamparameters as beam
import simulation as sim
import reconstruction as reco
import modularAnalysis as ana

reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
use_local_database('localdb/database.txt')

main = create_path()

main.add_module("RootInput")
main.add_module('Gearbox')
main.add_module('Geometry')
reco.add_reconstruction(main, pruneTracks=False)

ana.fillParticleList('mu+:bbmu', 'muid > 0.1 and useLabFrame(p) > 0.5', True, main)

main.add_module("MillepedeCollector", minPValue=0., particles=['mu+:bbmu'], tracks=[], components=['VXDAlignment'])

main.add_module("Progress")
main.add_module("RootOutput", branchNames=['EventMetaData'])


# generate events
process(main)

# show call statistics
print(statistics)
