import basf2
from basf2 import *
set_log_level(LogLevel.INFO)

import os
import sys
import multiprocessing
import datetime
import glob

import ROOT
from ROOT import Belle2, TFile
from ROOT.Belle2 import SVD3SampleCoGTimeCalibrationAlgorithm

from caf.framework import Calibration, CAF, Collection, LocalDatabase, CentralDatabase
from prompt import CalibrationSettings
from caf import backends
from caf import strategies
from caf.utils import ExpRun, IoV

import svd as svd
import modularAnalysis as ana
from caf.strategies import SequentialBoundaries

now = datetime.datetime.now()

settings = CalibrationSettings(name="SVDCoGTimeCalibrationPrompt",
                               expert_username="gdujany",
                               description=__doc__,
                               input_data_formats=["cdst"],
                               input_data_names=["hlt_hadron"],
                               depends_on=[])

##################################################################
# Remove Module from the Path


def remove_module(path, name):

    new_path = create_path()
    for m in path.modules():
        if name != m.name():
            new_path.add_module(m)
    return new_path
#####################################################################################################
# Definition of the class


def get_calibrations(input_data, **kwargs):

    # Set-up re-processing path
    path = create_path()

    path.add_module("Gearbox")
    path.add_module("Geometry", useDB=True)

    # unpack raw svd data and produce: SVDEventInfo and SVDShaperDigits
    svd.add_svd_unpacker(path)

    # run SVD reconstruction, changing names of StoreArray
    svd.add_svd_reconstruction(path)

    for moda in path.modules():
        if moda.name() == 'SVDCoGTimeEstimator':
            moda.param("ShaperDigits", 'SVDShaperDigitsFromTracks')
            moda.param("RecoDigits", 'SVDRecoDigitsFromTracks')
            moda.param("CalibrationWithEventT0", False)
        if moda.name() == 'SVDSimpleClusterizer':
            moda.param("Clusters", 'SVDClustersFromTracks')
            moda.param("RecoDigits", 'SVDRecoDigitsFromTracks')
            moda.param("ShaperDigits", 'SVDShaperDigitsFromTracks')  # for 3TA
            moda.param("timeAlgorithm", 1)  # 1:CoG3, 2:ELS3
        if moda.name() == 'SVDSpacePointCreator':
            moda.param("SVDClusters", 'SVDClustersFromTracks')

    path = remove_module(path, 'SVDMissingAPVsClusterCreator')

    file_to_iov_physics = input_data["hlt_hadron"]

    max_files_per_run = 1

    from prompt.utils import filter_by_max_files_per_run

    # reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run)
    reduced_file_to_iov_physics = filter_by_max_files_per_run(file_to_iov_physics, max_files_per_run, min_events_per_file=1)
    # input_files_physics = list(reduced_file_to_iov_physics.keys())
    good_input_files = list(reduced_file_to_iov_physics.keys())
    basf2.B2INFO(f"Total number of files actually used as input = {len(good_input_files)}")

    exps = [i.exp_low for i in reduced_file_to_iov_physics.values()]
    runs = sorted([i.run_low for i in reduced_file_to_iov_physics.values()])

    firstRun = runs[0]
    lastRun = runs[-1]
    expNum = exps[0]

    if not len(good_input_files):
        print("No good input files found! Check that the input files have entries != 0!")
        sys.exit(1)

    uniqueID = "SVD3SampleCoGTimeCalibrations_Test_" + str(now.isoformat()) + "_INFO:_3rdOrderPol_TBindep_Exp" + \
        str(expNum) + "_runsFrom" + str(firstRun) + "to" + str(lastRun)
    print("")
    print("UniqueID")
    print("")
    print(str(uniqueID))
    print("")

    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    # collector setup
    collector = register_module('SVDCoGTimeCalibrationCollector')
    collector.param("SVDClustersFromTracksName", "SVDClustersFromTracks")
    collector.param("SVDEventInfoName", "SVDEventInfo")
    collector.param("EventT0Name", "EventT0")
    collector.param("granularity", "run")

    # algorithm setup
    algorithm = SVD3SampleCoGTimeCalibrationAlgorithm(uniqueID)
    algorithm.setMinEntries(10000)

    # calibration setup
    calibration = Calibration('SVD3SampleCoGTime',
                              collector=collector,
                              algorithms=algorithm,
                              input_files=good_input_files,
                              pre_collector_path=path,
                              )

    # calibration.pre_algorithms = pre_alg
    calibration.strategies = strategies.SequentialRunByRun
    # calibration.strategies = strategies.SingleIOV
    # calibration.strategies = strategies.SequentialBoundaries

    for algorithm in calibration.algorithms:
        algorithm.params = {"iov_coverage": output_iov}

    return [calibration]
