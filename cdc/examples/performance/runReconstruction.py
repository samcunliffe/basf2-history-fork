import basf2
from basf2 import *
import ROOT
import datetime
import os
import os.path
import argparse
from tracking import add_cdc_cr_track_finding
from ROOT import Belle2
from cdc.cr.utilities import *

reset_database()
use_database_chain()
use_local_database(Belle2.FileSystem.findFile("data/framework/database.txt"))
use_central_database("cdc_cr_test1", LogLevel.WARNING)
set_experiment_name(0, "cdc_crt_e000000")

globalPhiRotation = 1.875


def main(input, output):
    main_path = basf2.create_path()
    logging.log_level = LogLevel.INFO

    # Get run number.
    run_number = getRunNumber(input)

    # Set the peiod of data taking.
    data_period = getDataPeriod(run_number)

    if os.path.exists(data_period) is False:
        os.mkdir(data_period)

    # RootInput
    main_path.add_module('RootInput',
                         inputFileNames=input)

    # gearbox & geometry needs to be registered any way
    main_path.add_module('Gearbox',
                         override=[
                             ("/DetectorComponent[@name='CDC']//GlobalPhiRotation", str(globalPhiRotation), "deg")
                         ])
    #
    main_path.add_module('Geometry',
                         components=['CDC'])
    main_path.add_module('Progress')

    # Set CDC CR parameters.
    set_cdc_cr_parameters(data_period)

    # Add CDC CR reconstruction.
    add_cdc_cr_reconstruction(main_path)

    # Simple analysi module.
    output = "/".join([data_period, output])
    main_path.add_module('CDCCosmicAnalysis',
                         Output=output)

    basf2.print_path(main_path)
    basf2.process(main_path)
    print(basf2.statistics)


if __name__ == "__main__":
    # Make the parameters accessible form the outside.

    parser = argparse.ArgumentParser()
    parser.add_argument('input', help='Input file to be processed (unpacked CDC data).')
    parser.add_argument('output', help='Output file you want to store the results.')
    args = parser.parse_args()

    date = datetime.datetime.today()
    print(date.strftime('Start at : %d-%m-%y %H:%M:%S\n'))
    main(args.input, args.output)
    date = datetime.datetime.today()
    print(date.strftime('End at : %y-%m-%d %H:%M:%S\n'))
