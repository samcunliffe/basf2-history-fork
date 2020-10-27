from itertools import islice
from tracking import add_tracking_reconstruction
import rawdata as raw
from caf.strategies import SequentialBoundaries
import modularAnalysis as ana
import svd as svd
from prompt.utils import events_in_basf2_file
from caf.utils import ExpRun, IoV
from caf import strategies
from caf import backends
from prompt import CalibrationSettings
from caf.framework import Calibration, CAF, Collection, LocalDatabase, CentralDatabase
from ROOT.Belle2 import SVD3SampleELSTimeCalibrationAlgorithm
from ROOT.Belle2 import SVD3SampleCoGTimeCalibrationAlgorithm
from ROOT.Belle2 import SVDCoGTimeCalibrationAlgorithm
from ROOT import Belle2, TFile
import ROOT
from random import choice, seed
import glob
import datetime
import multiprocessing
import sys
import os
import basf2
from basf2 import *
set_log_level(LogLevel.INFO)


now = datetime.datetime.now()
isMC = False

settings = CalibrationSettings(name="caf_svd_time",
                               expert_username="gdujany",
                               description=__doc__,
                               input_data_formats=["raw"],
                               input_data_names=["hlt_hadron"],
                               depends_on=[],
                               expert_config={
                                   "max_files_per_run": 20,
                                   "min_events_per_run": 10000,
                                   "min_events_per_file": 2000,
                                   "max_events_per_file": 5000
                               })

##################################################################
# Remove Module from the Path


def remove_module(path, name):

    new_path = create_path()
    for m in path.modules():
        if name != m.name():
            new_path.add_module(m)
    return new_path
##########################################################################


NEW_RECO_DIGITS_NAME = "SVDRecoDigitsFromTracks"
NEW_SHAPER_DIGITS_NAME = "SVDShaperDigitsFromTracks"


def create_collector(name="SVDTimeCalibrationCollector",
                     clusters="SVDClustersFromTracks",
                     event_info="SVDEventInfo",
                     event_t0="EventT0",
                     granularity="run"):
    """
    Simply creates a SVDTimeCalibrationCollector module with some options.

    Returns:
        pybasf2.Module
    """

    collector = register_module("SVDTimeCalibrationCollector")
    collector.set_name(name)
    collector.param("SVDClustersFromTracksName", clusters)
    collector.param("SVDEventInfoName", event_info)
    collector.param("EventT0Name", event_t0)
    collector.param("granularity", granularity)

    return collector


def create_algorithm(unique_id, prefix="", min_entries=10000):
    """
    Simply creates a SVDCoGTimeCalibrationAlgorithm class with some options.

    Returns:
        ROOT.Belle2.SVDCoGTimeCalibrationAlgorithm
    """
    if "CoG6" in prefix:
        algorithm = SVDCoGTimeCalibrationAlgorithm(unique_id)
    if "CoG3" in prefix:
        algorithm = SVD3SampleCoGTimeCalibrationAlgorithm(unique_id)
    if "ELS3" in prefix:
        algorithm = SVD3SampleELSTimeCalibrationAlgorithm(unique_id)
    if prefix:
        algorithm.setPrefix(prefix)
    algorithm.setMinEntries(10000)

    return algorithm


def create_svd_clusterizer(name="ClusterReconstruction",
                           clusters="SVDClustersFromTracks",
                           reco_digits=NEW_RECO_DIGITS_NAME,
                           shaper_digits=NEW_SHAPER_DIGITS_NAME,
                           time_algorithm=0):
    """
    Simply creates a SVDSimpleClusterizer module with some options.

    Returns:
        pybasf2.Module
    """

    cluster = register_module("SVDSimpleClusterizer")
    cluster.set_name(name)
    cluster.param("Clusters", clusters)
    cluster.param("RecoDigits", reco_digits)
    cluster.param("ShaperDigits", shaper_digits)
    cluster.param("timeAlgorithm", time_algorithm)
    return cluster


def select_files(reduced_file_to_iov, min_events, max_events_per_each_file):
    """
    Selects the input files per run to use for the calibration, starting from the minimum number of events per run that we need.
    It takes as input:
    1) an object that cointains the list of all input files and their IoV
    2) the minimum number of events per run we need
    3) the maximum number of events per file that at least we want, but there can be more or less events in a selected file.
       This number is useful to select enough files from each run as input.

    It loops on all input files, read the experiment and the run number,
    and splits the list in more lists depending on the run and experiment numbers.
    Then it loops on the lists obtained for each run and select the number of needed files per run,
    depending on the minimun number of events (min_events) and max_events_per_each_file we set.

    It is absolutely necessary to have enough events per run,
    since we need to check the distributions of each run to find the right IoVs for the payloads.

    Returns a list of file:
        selected_files
    """
    all_input_files = list(reduced_file_to_iov.keys())
    exp_list = [i.exp_low for i in reduced_file_to_iov.values()]
    run_list = [i.run_low for i in reduced_file_to_iov.values()]
    check_run = 0
    check_exp = 0
    size_for_run = []
    size_list = 0
    for item, nRun in enumerate(run_list):
        if item == 0:
            check_run = nRun
            check_exp = exp_list[item]
            size_list += 1
        elif item > 0:
            nExp = exp_list[item]
            if nRun == check_run and nExp == check_exp:
                size_list += 1
            else:
                check_run = nRun
                check_exp = nExp
                size_for_run.append(size_list)
                size_list = 1
        if item == len(run_list) - 1:
            size_for_run.append(size_list)
    iter_all_input_files = iter(all_input_files)
    iter_exp_list = iter(exp_list)
    iter_run_list = iter(run_list)
    splitted_all_input_files = [
        list(
            islice(
                iter_all_input_files,
                elem)) for elem in size_for_run]
    splitted_exp_list = [list(islice(iter_exp_list, elem))
                         for elem in size_for_run]
    splitted_run_list = [list(islice(iter_run_list, elem))
                         for elem in size_for_run]

    selected_files = []
    selected_files_per_run = []
    total_events = 0
    total_events_per_run = 0
    for item2, list_of_file in enumerate(splitted_all_input_files):
        while total_events_per_run < min_events:
            if not all_input_files:
                B2INFO(f"No Input files found.")
                break
            # Randomly selects one file from all_input_files list
            this_file = choice(list_of_file)
            # Removes the file from the list to not be choosen again
            list_of_file.remove(this_file)
            # Returns the number of events in the file
            events_in_file = events_in_basf2_file(this_file)
            if not events_in_file:
                continue
            events_counter = 0
            # Append the file to the list of selected files
            selected_files.append(this_file)
            selected_files_per_run.append(this_file)
            if events_in_file < max_events_per_each_file:
                events_counter = events_in_file
            else:
                events_counter = max_events_per_each_file
            total_events += events_counter
            total_events_per_run += events_counter
        basf2.B2INFO(
            f"(Exp,Run) = ({splitted_exp_list[item2][0]},{splitted_run_list[item2][0]})")
        basf2.B2INFO(
            f"Total chosen files per run = {len(selected_files_per_run)}")
        basf2.B2INFO(
            f"Total events in chosen files per run = {total_events_per_run}")
        if total_events < min_events:
            basf2.B2ERROR(
                f"Not enough files for the calibration when max_events_per_each_file={max_events_per_each_file}.")
        selected_files_per_run.clear()
        total_events_per_run = 0
    basf2.B2INFO(f"Total chosen files = {len(selected_files)}")
    basf2.B2INFO(f"Total events in chosen files = {total_events}")

    return selected_files


def create_pre_collector_path(clusterizers):
    """
    Create a basf2 path that runs a common reconstruction path and also runs several SVDSimpleClusterizer
    modules with different configurations. This way they re-use the same reconstructed objects.

    Parameters:
        clusterizers (list[pybasf2.Module]): All the differently configured SVDSimpleClusterizer modules.
        They should output to different datastore objects.

    returns:
        pybasf2.Path
    """
    # Set-up re-processing path
    path = create_path()

    # unpack raw data to do the tracking
    if not isMC:
        raw.add_unpackers(path, components=['PXD', 'SVD', 'CDC'])
    else:
        path.add_module("Gearbox")
        path.add_module("Geometry")

    # proceed only if we acquired 6-sample strips
    skim6SampleEvents = register_module("SVD6SampleEventSkim")
    path.add_module(skim6SampleEvents)
    emptypath = create_path()
    skim6SampleEvents.if_false(emptypath)

    # run tracking reconstruction
    add_tracking_reconstruction(path)
    path = remove_module(path, "V0Finder")

    for moda in path.modules():
        if moda.name() == "SVDCoGTimeEstimator":
            moda.param("CalibrationWithEventT0", False)

    path.add_module("SVDShaperDigitsFromTracks")

    # repeat svd reconstruction using only SVDShaperDigitsFromTracks
    cog = register_module("SVDCoGTimeEstimator")
    cog.set_name("CoGReconstruction")
    path.add_module(cog)

    # Debugging misconfigured Datastore names
    #    path.add_module("PrintCollections")

    for cluster in clusterizers:
        path.add_module(cluster)

    for moda in path.modules():
        if moda.name() == "CoGReconstruction":
            moda.param("ShaperDigits", NEW_SHAPER_DIGITS_NAME)
            moda.param("RecoDigits", NEW_RECO_DIGITS_NAME)
            moda.param("CalibrationWithEventT0", False)

    path = remove_module(path, "SVDMissingAPVsClusterCreator")

    return path


def get_calibrations(input_data, **kwargs):

    file_to_iov_physics = input_data["hlt_hadron"]
    expert_config = kwargs.get("expert_config")
    # This number should be setted from the biginning
    max_files_per_run = expert_config["max_files_per_run"]
    # Minimum number of events selected per each run
    min_events_per_run = expert_config["min_events_per_run"]
    # Files with less events will be discarded
    min_events_per_file = expert_config["min_events_per_file"]
    # Nominal max number of events selected per file. The events in the file
    # can be more.
    max_events_selected_per_file = expert_config["max_events_per_file"]

    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_physics = filter_by_max_files_per_run(
        file_to_iov_physics, max_files_per_run, min_events_per_file)
    good_input_files = select_files(
        reduced_file_to_iov_physics,
        min_events_per_run,
        max_events_selected_per_file)

    basf2.B2INFO(
        f"Total number of files before selection = {max_files_per_run}")
    basf2.B2INFO(
        f"Total number of files actually used as input = {len(good_input_files)}")

    exps = [i.exp_low for i in reduced_file_to_iov_physics.values()]
    runs = sorted([i.run_low for i in reduced_file_to_iov_physics.values()])

    firstRun = runs[0]
    lastRun = runs[-1]
    expNum = exps[0]

    if not len(good_input_files):
        print("No good input files found! Check that the input files have entries != 0!")
        sys.exit(1)

    cog6_suffix = "_CoG6"
    cog3_suffix = "_CoG3"
    els3_suffix = "_ELS3"

    calType = "Prompt"
    if isMC:
        calType = "MC"
    unique_id_cog6 = f"SVDCoGTimeCalibrations_{calType}_{now.isoformat()}_INFO:_3rdOrderPol_TBindep_" \
                     f"Exp{expNum}_runsFrom{firstRun}to{lastRun}"
    print(f"\nUniqueID_CoG6:\n{unique_id_cog6}")

    unique_id_cog3 = f"SVD3SampleCoGTimeCalibrations_{calType}_{now.isoformat()}_INFO:_3rdOrderPol_TBindep_" \
                     f"Exp{expNum}_runsFrom{firstRun}to{lastRun}"
    print(f"\nUniqueID_CoG3:\n{unique_id_cog3}")

    unique_id_els3 = f"SVD3SampleELSTimeCalibrations_{calType}_{now.isoformat()}_INFO:_TBindep_" \
                     f"Exp{expNum}_runsFrom{firstRun}to{lastRun}"
    print(f"\nUniqueID_ELS3:\n{unique_id_els3}")

    requested_iov = kwargs.get("requested_iov", None)
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    ####
    # Build the clusterizers with the different options.
    ####

    cog6 = create_svd_clusterizer(
        name=f"ClusterReconstruction{cog6_suffix}",
        clusters=f"SVDClustersFromTracks{cog6_suffix}",
        reco_digits=NEW_RECO_DIGITS_NAME,
        shaper_digits=NEW_SHAPER_DIGITS_NAME,
        time_algorithm=0)

    cog3 = create_svd_clusterizer(
        name=f"ClusterReconstruction{cog3_suffix}",
        clusters=f"SVDClustersFromTracks{cog3_suffix}",
        reco_digits=NEW_RECO_DIGITS_NAME,
        shaper_digits=NEW_SHAPER_DIGITS_NAME,
        time_algorithm=1)

    els3 = create_svd_clusterizer(
        name=f"ClusterReconstruction{els3_suffix}",
        clusters=f"SVDClustersFromTracks{els3_suffix}",
        reco_digits=NEW_RECO_DIGITS_NAME,
        shaper_digits=NEW_SHAPER_DIGITS_NAME,
        time_algorithm=2)

    ####
    # Build the Collectors and Algorithms with the different (but matching) options
    ####
    eventInfo = "SVDEventInfo"
    if isMC:
        eventInfo = "SVDEventInfoSim"

    coll_cog6 = create_collector(
        name=f"SVDTimeCalibrationCollector{cog6_suffix}",
        clusters=f"SVDClustersFromTracks{cog6_suffix}",
        event_info=eventInfo,
        event_t0="EventT0")

    algo_cog6 = create_algorithm(
        unique_id_cog6,
        prefix=coll_cog6.name(),
        min_entries=10000)

    coll_cog3 = create_collector(
        name=f"SVDTimeCalibrationCollector{cog3_suffix}",
        clusters=f"SVDClustersFromTracks{cog3_suffix}",
        event_info=eventInfo,
        event_t0="EventT0")

    algo_cog3 = create_algorithm(
        unique_id_cog3,
        prefix=coll_cog3.name(),
        min_entries=10000)

    coll_els3 = create_collector(
        name=f"SVDTimeCalibrationCollector{els3_suffix}",
        clusters=f"SVDClustersFromTracks{els3_suffix}",
        event_info=eventInfo,
        event_t0="EventT0")

    algo_els3 = create_algorithm(
        unique_id_els3,
        prefix=coll_els3.name(),
        min_entries=10000)

    ####
    # Build the pre_collector_path for reconstruction BUT we also sneakily
    # add the two cog collectors to it.
    ####

    pre_collector_path = create_pre_collector_path(
        clusterizers=[cog6, cog3, els3])
    pre_collector_path.add_module(coll_cog6)
    pre_collector_path.add_module(coll_cog3)
    # We leave the coll_els3 to be the one "managed" by the CAF

    # calibration setup
    calibration = Calibration("SVDTime",
                              collector=coll_els3,   # The other collectors are in the pre_collector_path itself
                              algorithms=[algo_cog6, algo_cog3, algo_els3],
                              input_files=good_input_files,
                              pre_collector_path=pre_collector_path)

    # calibration.strategies = strategies.SequentialRunByRun
    # calibration.strategies = strategies.SingleIOV
    calibration.strategies = strategies.SequentialBoundaries

    for algorithm in calibration.algorithms:
        algorithm.params = {"iov_coverage": output_iov}

    return [calibration]
