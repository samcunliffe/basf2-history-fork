import sys
import basf2
import argparse
import itertools
from softwaretrigger import (
    SOFTWARE_TRIGGER_GLOBAL_TAG_NAME
)
import ROOT
import softwaretrigger.hltdqm as hltdqm

import reconstruction
from softwaretrigger import add_fast_reco_software_trigger, add_hlt_software_trigger, \
    add_calibration_software_trigger, add_calcROIs_software_trigger

from daqdqm.collisiondqm import add_collision_dqm
from daqdqm.cosmicdqm import add_cosmic_dqm

from rawdata import add_unpackers

# Objects to be left on output
ALWAYS_SAVE_OBJECTS = ["EventMetaData", "SoftwareTriggerResult", "TRGSummary", "ROIpayload"]
RAWDATA_OBJECTS = ["RawCDCs", "RawSVDs", "RawPXDs", "RawTOPs", "RawARICHs", "RawKLMs", "RawECLs", "RawFTSWs", "RawTRGs", "ROIs"]
# Objects which will be kept after the ExpressReconstruction, for example for the Event Display
PROCESSED_OBJECTS = ['Tracks', 'TrackFitResults',
                     'SVDClusters', 'PXDClusters',
                     'CDCHits', 'TOPDigits', 'ARICHHits',
                     'ECLClusters',
                     'BKLMHit1ds', 'BKLMHit2ds',
                     'EKLMHit1ds', 'EKLMHit2ds',
                     'SoftwareTriggerResult']

# list of DataStore names that are present when data enters the HLT.
HLT_INPUT_OBJECTS = RAWDATA_OBJECTS + ["EventMetaData"]
HLT_INPUT_OBJECTS.remove("ROIs")

EXPRESSRECO_INPUT_OBJECTS = RAWDATA_OBJECTS + ALWAYS_SAVE_OBJECTS

# Detectors to be included
DEFAULT_HLT_COMPONENTS = ["CDC", "SVD", "ECL", "TOP", "ARICH", "BKLM", "EKLM", "TRG"]
DEFAULT_EXPRESSRECO_COMPONENTS = DEFAULT_HLT_COMPONENTS + ["PXD"]

# Functions


def setup_basf2_and_db(dbfile=None):
    """
    Setupl local database usage for HLT
    """

    parser = argparse.ArgumentParser(description='basf2 for online')
    parser.add_argument('input_buffer_name', type=str,
                        help='Input Ring Buffer names')
    parser.add_argument('output_buffer_name', type=str,
                        help='Output Ring Buffer name')
    parser.add_argument('histo_port', type=int,
                        help='Port of the HistoManager to connect to')
    parser.add_argument('number_processes', type=int, default=0,
                        help='Number of parallel processes to use')
    parser.add_argument('--local-db-path', type=str,
                        help="set path to the local database.txt to use for the ConditionDB",
                        default=None)
    parser.add_argument('--input-file', type=str,
                        help="Input sroot file, if set no RingBuffer input will be used",
                        default=None)
    parser.add_argument('--output-file', type=str,
                        help="Filename for SeqRoot output, if set no RingBuffer output will be used",
                        default=None)
    parser.add_argument('--histo-output-file', type=str,
                        help="Filename for histogram output",
                        default=None)
    parser.add_argument('--central-db-tag', type=str,
                        help="Use the central db with a specific tag",
                        default=None)
    parser.add_argument('--no-output',
                        help="Don't write any output files",
                        action="store_true", default=False)

    args = parser.parse_args()

    basf2.set_log_level(basf2.LogLevel.ERROR)
    ##########
    # Local DB specification
    ##########
    basf2.reset_database()
    if args.central_db_tag:
        basf2.use_central_database(args.central_db_tag)
    else:
        # use db file provided by parameter, if not set another one by command line
        if args.local_db_path is None:
            basf2.use_local_database(ROOT.Belle2.FileSystem.findFile(dbfile))
        else:
            basf2.use_local_database(ROOT.Belle2.FileSystem.findFile(args.local_db_path))

    basf2.set_nprocesses(args.number_processes)

    return args


def add_pxd_fullframe(path, min_ladders=(1, 1), max_ladders=(8, 12)):
    modules = []
    for layer in [1, 2]:
        min_ladder = min_ladders[layer - 1]
        max_ladder = max_ladders[layer - 1]
        for ladder in range(min_ladder, max_ladder + 1):
            for sensor in [1, 2]:
                modules.append((layer, ladder, sensor))

    for (layer, ladder, sensor) in modules:
        path.add_module('ROIGenerator', ROIListName='ROIs', nROIs=1, TrigDivider=1,
                        Layer=layer, Ladder=ladder, Sensor=sensor,
                        MinU=0, MaxU=249, MinV=0, MaxV=767)


def add_pxd_fullframe_phase2(path):
    add_pxd_fullframe(path, min_ladders=(1, 1), max_ladders=(1, 1))


def add_pxd_fullframe_phase3_early(path):
    add_pxd_fullframe(path, min_ladders=(1, 4), max_ladders=(8, 5))


def add_roi_payload_assembler(path, alwaysAcceptEvents=True, SendAllDownscaler=0, RejectByZeroROI=False):
    # send allDownscaler is set to two in order to get event 2nd full hit data
    # and the other half still the full frame because all 4 modules of phase 2 have
    # a Roi generated by the ROIGenerator
    path.add_module(
        'ROIPayloadAssembler',
        ROIListName='ROIs',
        ROIpayloadName='ROIpayload',
        SendAllDownscaler=SendAllDownscaler,
        SendROIsDownscaler=1,
        AcceptAll=alwaysAcceptEvents,
        NoRejectFlag=RejectByZeroROI)


def create_path_common(args, inputbuffer_module, inputfile='DAQ', dqmfile='DAQ'):
    """
    Create and return a path used for HLT and ExpressReco running
    """
    path = basf2.create_path()

    ##########
    # Input
    ##########
    if inputfile == 'DAQ' and not args.input_file:
        # Input from ringbuffer (for raw data)
        input = basf2.register_module(inputbuffer_module)
        input.param("RingBufferName", args.input_buffer_name)
    else:
        # Input from SeqRootInput
        input_file_name = None

        if args.input_file:
            input_file_name = args.input_file
        else:
            input_file_name = inputfile

        # automatic detection if root or sroot input module
        # is needed
        if input_file_name.endswith(".sroot"):
            input = basf2.register_module('SeqRootInput')
        else:
            input = basf2.register_module('RootInput')
        input.param('inputFileName', input_file_name)

    path.add_module(input)

    ##########
    # Histogram Handling
    ##########
    if dqmfile == 'DAQ' and not args.histo_output_file:
        # HistoManager for real HLT
        histoman = basf2.register_module('DqmHistoManager')
        histoman.param("Port", args.histo_port)
        histoman.param("DumpInterval", 1000)
    else:
        histoman = basf2.register_module('HistoManager')

        if args.histo_output_file:
            histoman.param('histoFileName', args.histo_output_file)
        else:
            histoman.param('histoFileName', dqmfile)

    path.add_module(histoman)

    return path


def create_hlt_path(args, inputfile='DAQ', dqmfile='DAQ'):
    return create_path_common(args, "Raw2Ds", inputfile, dqmfile)


def create_expressreco_path(args, inputfile='DAQ', dqmfile='DAQ'):
    return create_path_common(args, "Rbuf2Ds", inputfile, dqmfile)


def finalize_hlt_path(path, args, show_progress_bar=True, outputfile='HLT'):
    """
    Add the required output modules for HLT
    """
    ##########
    # Output
    ##########
    if outputfile == 'HLT' and not args.output_file:
        # Output to RingBuffer
        output = basf2.register_module("Ds2Rbuf")
        output.param("RingBufferName", args.output_buffer_name)
        output.param("saveObjs", ALWAYS_SAVE_OBJECTS + RAWDATA_OBJECTS)
    else:
        # Output to SeqRoot
        if args.output_file:
            outputfile = args.output_file

        if outputfile.endswith(".sroot"):
            output = basf2.register_module("SeqRootOutput")
            output.param("saveObjs", ALWAYS_SAVE_OBJECTS + RAWDATA_OBJECTS)
        else:
            output = basf2.register_module("RootOutput")
            # root output does not have 'saveObjs', everything is stored in the output file

        output.param('outputFileName', outputfile)

    if not args.no_output:
        path.add_module(output)

    ##########
    # Other utilities
    ##########
    if show_progress_bar:
        progress = basf2.register_module('Progress')
        path.add_module(progress)

#    etime = basf2.register_module('ElapsedTime')
#    path.add_module(etime)

    #########
    # Limit streaming objects for parallel processing
    #########
    basf2.set_streamobjs(ALWAYS_SAVE_OBJECTS + RAWDATA_OBJECTS + PROCESSED_OBJECTS)


def finalize_expressreco_path(path, args, show_progress_bar=True, outputfile='ERECO'):
    """
    Add the required output modules for HLT
    """
    ##########
    # PruneDataStore
    ##########
    prune = basf2.register_module("PruneDataStore")
    prune.param("matchEntries", ALWAYS_SAVE_OBJECTS + RAWDATA_OBJECTS + PROCESSED_OBJECTS)
    path.add_module(prune)

    ##########
    # Output
    ##########
    if outputfile == 'ERECO' and not args.output_file:
        output = basf2.register_module("Ds2Sample")
        output.param("RingBufferName", args.output_buffer_name)
        output.param("saveObjs", ALWAYS_SAVE_OBJECTS + RAWDATA_OBJECTS + PROCESSED_OBJECTS)
    else:
        # Output to SeqRoot
        if args.output_file:
            outputfile = args.output_file

        if outputfile.endswith(".sroot"):
            output = basf2.register_module("SeqRootOutput")
            output.param("saveObjs", ALWAYS_SAVE_OBJECTS + RAWDATA_OBJECTS)
        else:
            output = basf2.register_module("RootOutput")
            # root output does not have 'saveObjs', everything is stored in the output file

        output.param('outputFileName', outputfile)

    if not args.no_output:
        path.add_module(output)

    ##########
    # Other utilities
    ##########
    if show_progress_bar:
        progress = basf2.register_module('Progress')
        path.add_module(progress)

#    etime = basf2.register_module('ElapsedTime')
#    path.add_module(etime)

    #########
    # Limit streaming objects for parallel processing
    #########
    basf2.set_streamobjs(ALWAYS_SAVE_OBJECTS + RAWDATA_OBJECTS + PROCESSED_OBJECTS)


def add_hlt_processing(path, run_type="collision",
                       with_bfield=True,
                       pruneDataStore=True,
                       additonal_store_arrays_to_keep=[],
                       components=DEFAULT_HLT_COMPONENTS,
                       reco_components=None,
                       roi_take_fullframe=False,
                       calcROIs=True,
                       RejectByZeroROI=False,
                       softwaretrigger_mode='hlt_filter',
                       prune_input=True, **kwargs):
    """
    Add all modules for processing on HLT filter machines
    """

    # ensure that only DataStore content is present that we expect in
    # in the HLT configuration. If ROIpayloads or tracks are present in the
    # input file, this can be a problem and lead to crashes
    if prune_input:
        path.add_module(
            "PruneDataStore",
            matchEntries=HLT_INPUT_OBJECTS)

    # manually adds Gearbox for phase2, as long as the correct phase 2 geometry is not
    # in the conditions db for data experiment runs
    add_geometry_if_not_present(path)

    # todo: currently, the add_unpackers script will add the geometry with
    # all components. This is actually important, as also the PXD geomtery is
    # needed on the HLT (but not in DEFAULT_HLT_COMPONENTS) because the
    # RoiPayloadAssembler might look things up in the PXD Geometry
    # This needs to be properly solved in some future refactoring of this file
    add_unpackers(path, components=components)

    # if not set, just assume to reuse the normal compontents list
    if reco_components is None:
        reco_components = components

    if run_type == "collision":
        # todo: forward the the mag field and run_type mode into this method call
        add_softwaretrigger_reconstruction(path,
                                           components=reco_components,
                                           softwaretrigger_mode=softwaretrigger_mode,
                                           run_type=run_type,
                                           roi_take_fullframe=roi_take_fullframe,
                                           calcROIs=calcROIs,
                                           RejectByZeroROI=RejectByZeroROI,
                                           addDqmModules=True, **kwargs)
    elif run_type == "cosmics":
        # no filtering, don't prune RecoTracks so the Tracking DQM module has access to all hits
        sendAllDS = 0
        reconstruction.add_cosmics_reconstruction(path, components=reco_components, pruneTracks=False, **kwargs)
        if roi_take_fullframe:
            add_pxd_fullframe_phase3_early(path)
            sendAllDS = 2
        else:
            # this will generate ROIs using the output of CDC and SVD track finder
            add_calcROIs_software_trigger(path)
            sendAllDS = 0

        # always accept all events, because there is no software trigger result in cosmics
        add_roi_payload_assembler(path, alwaysAcceptEvents=True, SendAllDownscaler=sendAllDS)
        add_hlt_dqm(path, run_type, components=components)
        if pruneDataStore:
            path.add_module(
                "PruneDataStore",
                matchEntries=ALWAYS_SAVE_OBJECTS +
                RAWDATA_OBJECTS +
                additonal_store_arrays_to_keep)

    else:
        basf2.B2FATAL("Run Type {} not supported.".format(run_type))


def add_expressreco_processing(path, run_type="collision",
                               with_bfield=True,
                               components=DEFAULT_EXPRESSRECO_COMPONENTS,
                               reco_components=None,
                               prune_input=True,
                               do_reconstruction=True, **kwargs):
    """
    Add all modules for processing on the ExpressReco machines
    """
    # ensure that only DataStore content is present that we expect in
    # in the ExpressReco configuration. If tracks are present in the
    # input file, this can be a problem and lead to crashes
    if prune_input:
        path.add_module(
            "PruneDataStore",
            matchEntries=EXPRESSRECO_INPUT_OBJECTS)

    if not do_reconstruction:
        add_geometry_if_not_present(path)

    add_geometry_if_not_present(path)
    add_unpackers(path, components=components)

    # if not set, just assume to reuse the normal compontents list
    if reco_components is None:
        reco_components = components

    if do_reconstruction:
        # don't prune RecoTracks so the Tracking DQM module has access to all hits
        if run_type == "collision":
            reconstruction.add_reconstruction(path, components=reco_components, pruneTracks=False)
        elif run_type == "cosmics":
            reconstruction.add_cosmics_reconstruction(path, components=reco_components, pruneTracks=False, **kwargs)
        else:
            basf2.B2FATAL("Run Type {} not supported.".format(run_type))

    add_expressreco_dqm(path, run_type, components=components)


def add_softwaretrigger_reconstruction(
        path,
        store_array_debug_prescale=0,
        components=DEFAULT_HLT_COMPONENTS,
        trackFitHypotheses=[211],
        softwaretrigger_mode='hlt_filter',
        addDqmModules=False,
        run_type="collision",
        additonal_store_arrays_to_keep=[],
        pruneDataStore=True,
        calcROIs=True,
        roi_take_fullframe=False,
        RejectByZeroROI=True):
    """
    Add all modules, conditions and conditional paths to the given path, that are needed for a full
    reconstruction stack in the HLT using the software trigger modules. Several steps are performed:

    * Use FastReco to pre cut on certain types of background events
    * Use the SoftwareTriggerModule to actually calculate FastReco variables and do the cuts (the cuts are downloaded
      from the database).
    * Depending on the output of the module, either dismiss everything except the EventMetaData and the SoftwareTrigger
      results or call the remaining full reconstruction.
    * Create two particle lists pi+:HLT and gamma:HLT
    * Use the SoftwareTriggerModule again to calculate more advanced variables and do HLT cuts (again downloaded
      from the database).
    * Depending on the output of this module, dismiss everything except the meta or except the raw data from the
      data store.

    The whole setup looks like this:

                                                 -- [ Calibration ] -- [ Raw Data ] ---
                                               /                                        \
             In -- [ Fast Reco ] -- [ HLT ] --                                            -- Out
                                 \             \                                        /
                                  ----------------- [ Meta Data ] ---------------------


    Before calling this function, make sure that your database setup is suited to download software trigger cuts
    from the database (local or global) and that you unpacked raw data in your data store (e.g. call the add_unpacker
    function). After this part of the reconstruction is processes, you rather want to store the output, as you can not
    do anything sensible any more (all the information of the reconstruction is lost).

    :param path: The path to which the ST modules will be added.
    :param store_array_debug_prescale: Set to an finite value, to control for how many events the variables should
        be written out to the data store.
    :param components: the detector components
    :param softwaretrigger_mode: softwaretrigger_off: disable all software trigger activity, no reconstruction, no filter
                                 monitoring: enable reconstruction, fast_reco filter is off, hlt filter is off
                                 fast_reco_filter: enable reconstruction, fast_reco filter is on, hlt filter is off
                                 hlt_filter: default mode, enable all software activities
                                             including reconstruction, fast_reco and hlt filters.
    :param additonal_store_arrays_to_keep: StoreArray names which will kept together with the Raw objects after
                                           the HLT processing is complete. The content of the StoreArray will only
                                           be kept if the event is not filtered or the monitoring mode is used.
    :param pruneDataStore: If this is false, none of the reconstruction content will be removed from the datestore
                           after the reconstruction and software trigger is complete. Default is true.
    """
    # In the following, we will need some paths:
    # (1) A "store-metadata" path (deleting everything except the trigger tags and some metadata)
    store_only_metadata_path = basf2.create_path()
    # make sure to create the RoiPayload, even for rejected events
    add_roi_payload_assembler(store_only_metadata_path, alwaysAcceptEvents=False,
                              RejectByZeroROI=RejectByZeroROI)

    if pruneDataStore:
        store_only_metadata_path.add_path(get_store_only_metadata_path())

    # (3) A path doing the fast reco reconstruction
    fast_reco_reconstruction_path = basf2.create_path()
    # (4) A path doing the hlt reconstruction
    hlt_reconstruction_path = basf2.create_path()
    # (5) A path doing the calibration reconstruction with a "store-all" path, which deletes everything except
    # raw data, trigger tags and the meta data.
    calibration_and_store_only_rawdata_path = basf2.create_path()

    # this will be set depending on the path configuration below, the
    # payload assember must always run, but always at last
    accept_all_events_pxd = True

    # If softwaretrigger_mode is not any of the trigger mode, force the mode
    # to the default mode 'hlt_filter', and print out a warning
    if softwaretrigger_mode not in ['softwaretrigger_off', 'monitoring', 'fast_reco_filter', 'hlt_filter']:
        basf2.B2FATAL("The trigger mode " + softwaretrigger_mode +
                      " is not a supported software trigger" +
                      "mode [softwaretrigger_off, monitoring, fast_reco_filter, hlt_filter]" +
                      "the default is mode hlt_filter")

    if roi_take_fullframe:
        add_pxd_fullframe_phase3_early(fast_reco_reconstruction_path)

    # Add fast reco reconstruction
    if softwaretrigger_mode in ['monitoring', 'fast_reco_filter', 'hlt_filter']:
        reconstruction.add_reconstruction(fast_reco_reconstruction_path, trigger_mode="fast_reco", skipGeometryAdding=True,
                                          components=components, trackFitHypotheses=trackFitHypotheses)
        # Add fast reco cuts
        fast_reco_cut_module = add_fast_reco_software_trigger(fast_reco_reconstruction_path, store_array_debug_prescale)
        if softwaretrigger_mode in ['fast_reco_filter', 'hlt_filter']:
            # in this cases, we have a trigger decision only readout if PXD data is available
            accept_all_events_pxd = False
            # There are three possibilities for the output of this module
            # (1) the event is dismissed -> only store the metadata
            fast_reco_cut_module.if_value("==0", store_only_metadata_path, basf2.AfterConditionPath.CONTINUE)
            # (2) the event is accepted -> go on with the hlt reconstruction
            fast_reco_cut_module.if_value("==1", hlt_reconstruction_path, basf2.AfterConditionPath.CONTINUE)

        elif softwaretrigger_mode == 'monitoring':
            fast_reco_reconstruction_path.add_path(hlt_reconstruction_path)

        # Add hlt reconstruction
        reconstruction.add_reconstruction(hlt_reconstruction_path, trigger_mode="hlt", skipGeometryAdding=True,
                                          components=components, trackFitHypotheses=trackFitHypotheses, pruneTracks=False)

        hlt_cut_module = add_hlt_software_trigger(hlt_reconstruction_path, store_array_debug_prescale)

        # preserve the reconstruction information which is needed for ROI calculation.
        add_calcROIs_software_trigger(calibration_and_store_only_rawdata_path, calcROIs=calcROIs)
        # Fill the calibration_and_store_only_rawdata_path path
        add_calibration_software_trigger(calibration_and_store_only_rawdata_path, store_array_debug_prescale)
        # write Roi for Accepted events
        add_roi_payload_assembler(
            calibration_and_store_only_rawdata_path,
            alwaysAcceptEvents=accept_all_events_pxd,
            RejectByZeroROI=RejectByZeroROI)
        # currently, dqm plots are only shown for event accepted by the HLT filters
        add_hlt_dqm(calibration_and_store_only_rawdata_path, run_type, components=components)

        if pruneDataStore:
            calibration_and_store_only_rawdata_path.add_path(get_store_only_rawdata_path(additonal_store_arrays_to_keep))
        if softwaretrigger_mode == 'hlt_filter':
            # There are two possibilities for the output of this module
            # (1) the event is rejected -> only store the metadata
            hlt_cut_module.if_value("==0", store_only_metadata_path, basf2.AfterConditionPath.CONTINUE)
            # (2) the event is accepted -> go on with the calibration
            hlt_cut_module.if_value("==1", calibration_and_store_only_rawdata_path, basf2.AfterConditionPath.CONTINUE)
        elif softwaretrigger_mode in ['monitoring', 'fast_reco_filter']:
            hlt_reconstruction_path.add_path(calibration_and_store_only_rawdata_path)

    elif softwaretrigger_mode == 'softwaretrigger_off':
        # make sure to still add the DQM modules, they can give at least some FW runtime info
        # and some unpacked hit information
        add_hlt_dqm(path, run_type, components=components)
        # todo: empty ROIid list needs to generater so at least someone fills the store array
        add_roi_payload_assembler(path, alwaysAcceptEvents=True)
        if pruneDataStore:
            fast_reco_reconstruction_path.add_module(
                "PruneDataStore",
                matchEntries=ALWAYS_SAVE_OBJECTS +
                RAWDATA_OBJECTS + PROCESSED_OBJECTS +
                additonal_store_arrays_to_keep)

    path.add_path(fast_reco_reconstruction_path)


def add_online_dqm(path, run_type, dqm_environment, components):
    """
    Add DQM plots for a specific run type and dqm environment
    """
    if run_type == "collision":
        add_collision_dqm(path, components=components, dqm_environment=dqm_environment)
    elif run_type == "cosmics":
        add_cosmic_dqm(path, components=components, dqm_environment=dqm_environment)
    else:
        basf2.B2FATAL("Run type {} not supported.".format(run_type))
    path.add_module('DelayDQM', title=dqm_environment, histogramDirectoryName='DAQ')


def add_hlt_dqm(path, run_type, standalone=False, components=DEFAULT_HLT_COMPONENTS):
    """
    Add all the DQM modules for HLT to the path
    """
    if "HistoManager" not in path.modules() or "DqmHistoManager" not in path.modules():
        basf2.B2WARNING("I am not adding the DQM modules " +
                        "as there is neither a HistoManager nor the DqmHistoManager module present.")
        return

    if standalone:
        add_geometry_if_not_present(path)
        add_unpackers(path, components=components)

    add_online_dqm(path, run_type, "hlt", components)


def add_expressreco_dqm(path, run_type, standalone=False, components=DEFAULT_EXPRESSRECO_COMPONENTS):
    """
    Add all the DQM modules for ExpressReco to the path
    """
    if standalone:
        add_geometry_if_not_present(path)
        add_unpackers(path, components=components)

    add_online_dqm(path, run_type, "expressreco", components)


def add_geometry_if_not_present(path):
    # geometry parameter database
    if 'Gearbox' not in path:
        path.add_module('Gearbox')

    # detector geometry
    if 'Geometry' not in path:
        path.add_module('Geometry', useDB=True)


def get_store_only_metadata_path():
    """
    Helper function to create a path which deletes (prunes) everything from the data store except
    things that are really needed, e.g. the event meta data and the results of the software trigger module.

    After this path was processed, you can not use the data store content any more to do reconstruction (because
    it is more or less empty), but can only output it to a (S)ROOT file.
    :return: The created path.
    """
    store_metadata_path = basf2.create_path()
    store_metadata_path.add_module("PruneDataStore", matchEntries=ALWAYS_SAVE_OBJECTS). \
        set_name("KeepMetaData")

    return store_metadata_path


def get_store_only_rawdata_path(additonal_store_arrays_to_keep=[]):
    """
    Helper function to create a path which deletes (prunes) everything from the data store except
    raw objects from the detector and things that are really needed, e.g. the event meta data and the results of the
    software trigger module.

    After this path was processed, you can not use the data store content any more to do reconstruction (because
    it is more or less empty), but can only output it to a (S)ROOT file.
    :return: The created path.
    """
    store_rawdata_path = basf2.create_path()
    store_rawdata_path.add_module(
        "PruneDataStore",
        matchEntries=ALWAYS_SAVE_OBJECTS +
        RAWDATA_OBJECTS +
        additonal_store_arrays_to_keep) .set_name("KeepRawData")

    return store_rawdata_path


def setup_softwaretrigger_database_access(software_trigger_global_tag_name=SOFTWARE_TRIGGER_GLOBAL_TAG_NAME,
                                          production_global_tag_name="development"):
    """
    Helper function to set up the database chain, needed for typical software trigger applications. This chains
    consists of:
    * access to the local database store in localdb/database.txt in the current folder.
    * global database access with the given software trigger global tag (probably the default one).
    * global database access with the "development" tag, which is the standard global database.

    :param software_trigger_global_tag_name: controls the name of the software trigger global tag in the database.
    :param production_global_tag_name: controls the name of the general global tag in the database.
    """
    basf2.reset_database()
    basf2.use_database_chain()
    basf2.use_local_database("localdb/dbcache.txt")
    basf2.use_central_database(software_trigger_global_tag_name)
    basf2.use_central_database(production_global_tag_name)
