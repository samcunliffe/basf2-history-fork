#!/usr/bin/env python3
############################################################################
#
# this script is used to create ECLCrystalsShapeAndPosition payload
# from files ecl/data/crystal_{shape,position}_{forward,barrel,backward}.dat
#
############################################################################

import basf2
main = basf2.create_path()
main.add_module("EventInfoSetter")
main.add_module("Gearbox")
main.add_module("Geometry", createPayloads=True, components=["ECL"], payloadIov=[0, 0, -1, -1])
basf2.process(main)
