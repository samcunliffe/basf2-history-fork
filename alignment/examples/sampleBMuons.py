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
# use_local_database('localdb/database.txt')

main = create_path()
main.add_module("EventInfoSetter")
beam.add_beamparameters(main, "Y4S")
main.add_module('Gearbox')
main.add_module('Geometry')
main.add_module('ParticleGun', pdgCodes=[13, -13])
sim.add_simulation(main)
main.add_module("Progress")
main.add_module("RootOutput")
process(main)
print(statistics)
