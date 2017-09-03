#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''
An example script to reconstruct osmics events with standalone CDC.
Usage :
basf2 runReconstruction.py <input> <output>
input: Input root file (after CDC unpacker).
       These data are usually stored in
       /ghi/fs01/belle2/bdata/group/detector/CDC/unpacked/
output : Output root file, which contains helix parameters.
         N.B. this is not the basf2 root file!
         To see the helix parameters.
         Please use compare2Tracks.C for example.
'''
import basf2
from basf2 import *
import ROOT
import os
import os.path
import argparse
from tracking import add_cdc_cr_track_finding
from ROOT import Belle2
from cdc.cr import *

reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
# use_local_database("cdc_crt/database.txt", "cdc_crt")
# use_local_database("localDB/database.txt", "localDB")
use_central_database("GT_gen_data_002.11_gcr2017-07", LogLevel.WARNING)


def rec(input, output, topInCounter=False, magneticField=True):

    main_path = basf2.create_path()
    logging.log_level = LogLevel.INFO

    data_period = 'gcr2017'

    # print(data_period)
    if os.path.exists('output') is False:
        os.mkdir('output')

    # RootInput
    main_path.add_module('RootInput',
                         inputFileNames=input)

    # gearbox & geometry needs to be registered any way
    #    main_path.add_module('Gearbox',
    #                         override=[
    #                             ("/DetectorComponent[@name='CDC']//GlobalPhiRotation", str(globalPhiRotation), "deg")
    #                         ])
    main_path.add_module('Gearbox')
    #
    if magneticField is False:
        main_path.add_module('Geometry',
                             components=['CDC'])
    else:
        #        main_path.add_module('Geometry')
        main_path.add_module('Geometry', excludedComponents=['SVD', 'PXD', 'ARICH', 'BeamPipe', 'EKLM'])
        #        main_path.add_module('Geometry',
        #                             components=['CDC', 'MagneticFieldConstant4LimitedRCDC'])

    main_path.add_module('Progress')

    # Add CDC CR reconstruction.
    set_cdc_cr_parameters(data_period)
    add_cdc_cr_reconstruction(main_path)

    # Simple analysi module.
    output = "/".join(['output', output])
    main_path.add_module('CDCCosmicAnalysis',
                         noBFit=not magneticField,
                         Output=output)

    #    main_path.add_module("RootOutput", outputFileName='full.root')
    basf2.print_path(main_path)
    basf2.process(main_path)
    print(basf2.statistics)


if __name__ == "__main__":
    # Make the parameters accessible form the outside.

    parser = argparse.ArgumentParser()
    parser.add_argument('input', help='Input file to be processed (unpacked CDC data).')
    parser.add_argument('output', help='Output file you want to store the results.')
    args = parser.parse_args()
    rec(args.input, args.output, topInCounter=False, magneticField=True)
