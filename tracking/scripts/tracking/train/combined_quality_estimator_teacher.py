#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
combined_module_quality_estimator_teacher
-----------------------------------------

Information on the MVA Track Quality Indicator / Estimator can be found
on `Confluence
<https://confluence.desy.de/display/BI/MVA+Track+Quality+Indicator>`_.

Purpose of this script
~~~~~~~~~~~~~~~~~~~~~~

This python script is used for the combined training and validation of three
classifiers, the actual final MVA track quality estimator and the two quality
estimators for the intermediate standalone track finders that it depends on.

    - Final MVA track quality estimator:
      The final quality estimator for fully merged and fitted tracks. Its
      classifier uses features from the track fitting, merger, hit pattern, ...
      But it also uses the outputs from respective intermediate quality
      estimators for the VXD and the CDC track finding as inputs. It provides
      the final quality indicator (QI) exported to the track objects.

    - VXDTF2 track quality estimator:
      MVA quality estimator for the VXD standalone track finding.

    - CDC track quality estimator:
      MVA quality estimator for the CDC standalone track finding.

Each classifier requires for its training a different training data set and they
need to be validated on a separate testing data set. Further, the final quality
estimator can only be trained, when the trained weights for the intermediate
quality estimators are available. To avoid mistakes, b2luigi is used to create a
task chain for a combined training and validation of all classifiers.

b2luigi: Understanding the steering file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All trainings and validations are done in the correct order in this steering
file. For the purpose of creating a dependency graph, the `b2luigi
<https://b2luigi.readthedocs.io>`_ python package is used, which extends the
`luigi <https://luigi.readthedocs.io>`_ packag developed by spotify.

Each task that has to be done is represented by a special class, which defines
which defines parameters, output files and which other tasks with which
parameters it depends on.  For example a teacher task, which runs
``basf2_mva_teacher.py`` to train the classifier, depends on a data collection
task which runs a reconstruction and writes out track-wise variables into a root
file for training.  An evaluation/validation task for testing the classifier
requires both the teacher task, as it needs the weightfile to be present, and
also a data collection task, because it needs a dataset for testing classifier.

The final task that defines which tasks need to be done for the steering file to
finish is the ``MasterTask``. When you only want to run parts of the
training/validation pipeline, you can comment out requirements in the Master
task or replace them by lower-level tasks during debugging.

Requirements
~~~~~~~~~~~~

This steering file relies on b2luigi_ for task scheduling and `uncertain_panda
<https://github.com/nils-braun/uncertain_panda>`_ for uncertainty calculations.
uncertain_panda is not in the externals and b2luigi is not upto v01-07-01. Both
can be installed via pip::

    python3 -m pip install [--user] b2luigi uncertain_panda

Use the ``--user`` option if you have not rights to install python packages into
your externals (e.g. because you are using cvmfs) and install them in
``$HOME/.local`` instead.

Configuration
~~~~~~~~~~~~~

Instead of command line arguments, the b2luigi script is configured via a
``settings.json`` file. Open it in your favorite text editor and modify it to
fit to your requirements.

Usage
~~~~~

You can test the b2luigi without running it via::

    python3 combined_quality_estimator_teacher.py --dry-run
    python3 combined_quality_estimator_teacher.py --show-output

This will show the outputs and show potential errors in the definitions of the
luigi task dependencies.  To run the the steering file in normal (local) mode,
run::

    python3 combined_quality_estimator_teacher.py

I usually use the interactive luigi web interface via the central scheduler
which visualizes the task graph while it is running. Therefore, the scheduler
daemon ``luigid`` has to run in the background, which is located in
``~/.local/bin/luigid`` in case b2luigi had been installed with ``--user``. For
example, run::

    luigid --port 8886

Then, execute your steering (e.g. in another terminal) with::

    python3 combined_quality_estimator_teacher.py --scheduler-port 8886

To view the web interface, open your webbrowser enter into the url bar::

    localhost:8886

If you don't run the steering file on the same machine on which you run your web
browser, you have two options:

    1. Run both the steering file and ``luigid`` remotely and use
       ssh-port-forwarding to your local host. Therefore, run on your local
       machine::

           ssh -N -f -L 8886:localhost:8886 <remote_user>@<remote_host>

    2. Run the ``luigid`` scheduler locally and use the ``--scheduler-host <your
       local host>`` argument when calling the steering file

Accessing the results / output files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All output files are stored in a directory structure in the ``result_path``. The
directory tree encodes the used b2luigi parameters. This ensures reproducability
and makes parameter searches easy. Sometimes, it is hard to find the relevant
output files. You can view the whole directory structure by running ``tree
<result_path>``. Ise the unix ``find`` command to find the files that interest
you, e.g.::

    find <result_path> -name "*.pdf" # find all validation plot files
    find <result_path> -name "*.root" # find all ROOT files
"""

import itertools
import os
from pathlib import Path
import shutil
import subprocess
import textwrap
from datetime import datetime
from typing import Iterable

import matplotlib.pyplot as plt
import numpy as np
import root_pandas
from matplotlib.backends.backend_pdf import PdfPages

import basf2
import basf2_mva
from packaging import version
import background
import simulation
import tracking
import tracking.root_utils as root_utils
from tracking.harvesting_validation.combined_module import CombinedTrackingValidationModule

# wrap python modules that are used here but not in the externals into a try except block
install_helpstring_formatter = ("\nCould not find {module} python module.Try installing it via\n"
                                "  python3 -m pip install [--user] {module}\n")
try:
    import b2luigi
    from b2luigi.core.utils import get_serialized_parameters, get_log_file_dir
    from b2luigi.basf2_helper import Basf2PathTask, Basf2Task
    from b2luigi.core.task import Task
    from b2luigi.basf2_helper.utils import get_basf2_git_hash
except ModuleNotFoundError:
    print(install_helpstring_formatter.format(module="b2luigi"))
    raise
try:
    from uncertain_panda import pandas as upd
except ModuleNotFoundError:
    print(install_helpstring_formatter.format(module="uncertain_panda"))
    raise

# If b2luigi version 0.3.2 or older, it relies on $BELLE2_RELEASE being "head",
# which is not the case in the new externals. A fix has been merged into b2luigi
# via https://github.com/nils-braun/b2luigi/pull/17 and thus should be available
# in future releases.
if (
    version.parse(b2luigi.__version__) <= version.parse("0.3.2") and
    get_basf2_git_hash() is None and
    os.getenv("BELLE2_LOCAL_DIR") is not None
):
    print(f"b2luigi version could not obtain git hash because of a bug not yet fixed in version {b2luigi.__version__}\n"
          "Please install the latest version of b2luigi from github via\n\n"
          "  python3 -m pip install --upgrade [--user] git+https://github.com/nils-braun/b2luigi.git\n")
    raise ImportError

# Utility functions


def create_fbdt_option_string(fast_bdt_option):
    return "_nTrees" + str(fast_bdt_option[0]) + "_nCuts" + str(fast_bdt_option[1]) + "_nLevels" + \
                           str(fast_bdt_option[2]) + "_shrin" + str(int(round(100*fast_bdt_option[3], 0)))


def my_basf2_mva_teacher(
    records_files,
    tree_name,
    weightfile_identifier,
    target_variable="truth",
    exclude_variables=[],
    fast_bdt_option=[200, 8, 3, 0.1]  # nTrees, nCuts, nLevels, shrinkage
):
    """
    My custom wrapper for basf2 mva teacher.  Adapted from code in ``trackfindingcdc_teacher``.

    :param records_files: List of files with collected ("recorded") variables to use as training data for the MVA.
    :param tree_name: Name of the TTree in the ROOT file from the ``data_collection_task``
           that contains the training data for the MVA teacher.
    :param weightfile_identifier: Name of the weightfile that is created.
           Should either end in ".xml" for local weightfiles or in ".root", when
           the weightfile needs later to be uploaded as a payload to the conditions
           database.
    :param target_variable: Feature/variable to use as truth label in the quality estimator MVA classifier.
    :param exclude_variables: List of collected variables to not use in the training of the QE MVA classifier.
           In addition to variables containing the "truth" substring, which are excluded by default.
    """

    weightfile_extension = Path(weightfile_identifier).suffix
    if weightfile_extension not in {".xml", ".root"}:
        raise ValueError(f"Weightfile Identifier should end in .xml or .root, but ends in {weightfile_extension}")

    # extract names of all variables from one record file
    with root_utils.root_open(records_files[0]) as records_tfile:
        input_tree = records_tfile.Get(tree_name)
        feature_names = [leave.GetName() for leave in input_tree.GetListOfLeaves()]

    # get list of variables to use for training without MC truth
    truth_free_variable_names = [
        name
        for name in feature_names
        if (
            ("truth" not in name) and
            (name != target_variable) and
            (name not in exclude_variables)
        )
    ]
    if "weight" in truth_free_variable_names:
        truth_free_variable_names.remove("weight")
        weight_variable = "weight"
    elif "__weight__" in truth_free_variable_names:
        truth_free_variable_names.remove("__weight__")
        weight_variable = "__weight__"
    else:
        weight_variable = ""

    # Set options for MVA trainihng
    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(*records_files)
    general_options.m_treename = tree_name
    general_options.m_weight_variable = weight_variable
    general_options.m_identifier = weightfile_identifier
    general_options.m_variables = basf2_mva.vector(*truth_free_variable_names)
    general_options.m_target_variable = target_variable
    fastbdt_options = basf2_mva.FastBDTOptions()

    fastbdt_options.m_nTrees = fast_bdt_option[0]
    fastbdt_options.m_nCuts = fast_bdt_option[1]
    fastbdt_options.m_nLevels = fast_bdt_option[2]
    fastbdt_options.m_shrinkage = fast_bdt_option[3]
    # Train a MVA method and store the weightfile (MVAFastBDT.root) locally.
    basf2_mva.teacher(general_options, fastbdt_options)


def _my_uncertain_mean(series: upd.Series):
    """
    Temporary Workaround bug in ``uncertain_panda`` where a ``ValueError`` is
    thrown for ``Series.unc.mean`` if the series is empty.  Can be replaced by
    .unc.mean when the issue is fixed.
    https://github.com/nils-braun/uncertain_panda/issues/2
    """
    try:
        return series.unc.mean()
    except ValueError:
        if series.empty:
            return np.nan
        else:
            raise


def get_uncertain_means_for_qi_cuts(df: upd.DataFrame, column: str, qi_cuts: Iterable[float]):
    """
    Return a pandas series with an mean of the dataframe column and
    uncertainty for each quality indicator cut.

    :param df: Pandas dataframe with at least ``quality_indicator``
        and another numeric ``column``.
    :param column: Column of which we want to aggregate the means
        and uncertainties for different QI cuts
    :param qi_cuts: Iterable of quality indicator minimal thresholds.
    :returns: Series of of means and uncertainties with ``qi_cuts`` as index
    """

    uncertain_means = (_my_uncertain_mean(df.query(f"quality_indicator > {qi_cut}")[column])
                       for qi_cut in qi_cuts)
    uncertain_means_series = upd.Series(data=uncertain_means, index=qi_cuts)
    return uncertain_means_series


def plot_with_errobands(uncertain_series,
                        error_band_alpha=0.3,
                        plot_kwargs={},
                        fill_between_kwargs={},
                        ax=None):
    """
    Plot an uncertain series with error bands for y-errors
    """
    if ax is None:
        ax = plt.gca()
    uncertain_series = uncertain_series.dropna()
    ax.plot(uncertain_series.index.values, uncertain_series.nominal_value, **plot_kwargs)
    ax.fill_between(x=uncertain_series.index,
                    y1=uncertain_series.nominal_value - uncertain_series.std_dev,
                    y2=uncertain_series.nominal_value + uncertain_series.std_dev,
                    alpha=error_band_alpha,
                    **fill_between_kwargs)


def format_dictionary(adict, width=80, bullet="•"):
    """
    Helper function to format dictionary to string as a wrapped key-value bullet
    list.  Useful to print metadata from dictionaries.

    :param adict: Dictionary to format
    :param width: Characters after which to wrap a key-value line
    :param bullet: Character to begin a key-value line with, e.g. ``-`` for a
        yaml-like string
    """
    # It might be possible to replace this function yaml.dump, but the current
    # version in the externals does not allow to disable the sorting of the
    # dictionary yet and also I am not sure if it is wrappable
    return "\n".join(textwrap.fill(f"{bullet} {key}: {value}", width=width)
                     for (key, value) in adict.items())

# Begin definitions of b2luigi task classes


class GenerateSimTask(Basf2PathTask):
    """
    Generate simulated Monte Carlo with background overlay.

    Make sure to use different ``random_seed`` parameters for the training data
    format the classifier trainings and for the test data for the respective
    evaluation/validation tasks.
    """

    #: Number of events to generate.
    n_events = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Random basf2 seed.
    random_seed = b2luigi.Parameter()
    #: Directory with overlay background root files
    bkgfiles_dir = b2luigi.Parameter(hashed=True)
    #: Name of the ROOT output file with generated and simulated events.
    output_file_name = "generated_mc.root"

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished iff the outputs all exist.
        """
        yield self.add_to_output(self.output_file_name)

    def create_path(self):
        """
        Create basf2 path to process with event generation and simulation.
        """
        basf2.set_random_seed(self.random_seed)
        path = basf2.create_path()
        path.add_module(
            "EventInfoSetter", evtNumList=[self.n_events], runList=[0], expList=[self.experiment_number]
        )
        path.add_module("EvtGenInput")
        bkg_files = background.get_background_files(self.bkgfiles_dir)
        if self.experiment_number == 1002:
            # remove KLM because of bug in backround files with release 4
            components = ['PXD', 'SVD', 'CDC', 'ECL', 'TOP', 'ARICH', 'TRG']
        else:
            components = None
        simulation.add_simulation(path, bkgfiles=bkg_files, bkgOverlay=True, components=components)

        path.add_module(
            "RootOutput",
            outputFileName=self.get_output_file_name(self.output_file_name),
        )
        return path


class VXDQEDataCollectionTask(Basf2PathTask):
    """
    Collect variables/features from VXDTF2 tracking and write them to a ROOT
    file.

    These variables are to be used as labelled training data for the MVA
    classifier which is the VXD track quality estimator
    """
    #: Number of events to generate.
    n_events = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Random basf2 seed used by the GenerateSimTask.
    random_seed = b2luigi.Parameter()
    #: Filename of the recorded/collected data for the VXDTF2 QE MVA training.
    records_file_name = "vxd_qe_records.root"

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield GenerateSimTask(
            bkgfiles_dir=MasterTask.bkgfiles_by_exp[self.experiment_number],
            num_processes=self.num_processes,
            random_seed=self.random_seed,
            n_events=self.n_events,
            experiment_number=self.experiment_number,
        )

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished iff the outputs all exist.
        """
        yield self.add_to_output(self.records_file_name)

    def create_path(self):
        """
        Create basf2 path with VXDTF2 tracking and VXD QE data collection.
        """
        path = basf2.create_path()
        path.add_module(
            "RootInput",
            inputFileNames=self.get_input_file_names(GenerateSimTask.output_file_name),
        )
        path.add_module("Gearbox")
        tracking.add_geometry_modules(path)
        tracking.add_hit_preparation_modules(path)  # only needed for simulated hits
        tracking.add_vxd_track_finding_vxdtf2(
            path, components=["SVD"], add_mva_quality_indicator=False
        )
        path.add_module(
            "TrackFinderMCTruthRecoTracks",
            RecoTracksStoreArrayName="MCRecoTracks",
            WhichParticles=[],
            UsePXDHits=False,
            UseSVDHits=True,
            UseCDCHits=False,
        )
        path.add_module(
            "VXDQETrainingDataCollector",
            TrainingDataOutputName=self.get_output_file_name(self.records_file_name),
            SpacePointTrackCandsStoreArrayName="SPTrackCands",
            EstimationMethod="tripletFit",
            UseTimingInfo=False,
            ClusterInformation="Average",
            MCStrictQualityEstimator=True,
            mva_target=False,
        )
        return path


class CDCQEDataCollectionTask(Basf2PathTask):
    """
    Collect variables/features from CDC tracking and write them to a ROOT file.

    These variables are to be used as labelled training data for the MVA
    classifier which is the CDC track quality estimator
    """
    #: Number of events to generate.
    n_events = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Random basf2 seed used by the GenerateSimTask.
    random_seed = b2luigi.Parameter()
    #: Filename of the recorded/collected data for the CDC QE MVA training.
    records_file_name = "cdc_qe_records.root"

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield GenerateSimTask(
            bkgfiles_dir=MasterTask.bkgfiles_by_exp[self.experiment_number],
            num_processes=self.num_processes,
            random_seed=self.random_seed,
            n_events=self.n_events,
            experiment_number=self.experiment_number,
        )

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished iff the outputs all exist.
        """
        yield self.add_to_output(self.records_file_name)

    def create_path(self):
        """
        Create basf2 path with CDC standalone tracking and CDC QE with recording filter for MVA feature collection.
        """
        path = basf2.create_path()
        path.add_module(
            "RootInput",
            inputFileNames=self.get_input_file_names(GenerateSimTask.output_file_name),
        )
        path.add_module("Gearbox")
        tracking.add_geometry_modules(path)
        tracking.add_hit_preparation_modules(path)  # only needed for simulated hits
        tracking.add_cdc_track_finding(path, add_mva_quality_indicator=True)

        basf2.set_module_parameters(
            path,
            name="TFCDC_TrackQualityEstimator",
            filter="recording",
            filterParameters={
                "rootFileName": self.get_output_file_name(self.records_file_name)
            },
        )
        return path


class FullTrackQEDataCollectionTask(Basf2PathTask):
    """
    Collect variables/features from the full track reconstruction including the
    fit and write them to a ROOT file.

    These variables are to be used as labelled training data for the MVA
    classifier which is the MVA track quality estimator.  The collected
    variables include the classifier outputs from the VXD and CDC quality
    estimators, namely the CDC and VXD quality indicators, combined with fit,
    merger, timing, energy loss information etc.  This task requires the
    subdetector quality estimators to be trained.
    """

    #: Number of events to generate.
    n_events = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Random basf2 seed used by the GenerateSimTask.
    random_seed = b2luigi.Parameter()
    #: Filename of the recorded/collected data for the final QE MVA training.
    records_file_name = "fulltrack_qe_records.root"
    #: Feature/variable to use as truth label for the CDC track quality estimator.
    cdc_training_target = b2luigi.Parameter()

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield GenerateSimTask(
            bkgfiles_dir=MasterTask.bkgfiles_by_exp[self.experiment_number],
            num_processes=MasterTask.num_processes,
            random_seed=self.random_seed,
            n_events=self.n_events,
            experiment_number=self.experiment_number,
        )
        yield CDCQETeacherTask(
            n_events_training=MasterTask.n_events_training,
            experiment_number=self.experiment_number,
            training_target=self.cdc_training_target,
        )
        yield VXDQETeacherTask(
            n_events_training=MasterTask.n_events_training,
            experiment_number=self.experiment_number,
        )

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished iff the outputs all exist.
        """
        yield self.add_to_output(self.records_file_name)

    def create_path(self):
        """
        Create basf2 reconstruction path that should mirror the default path
        from ``add_tracking_reconstruction()``, but with modules for the VXD QE
        and CDC QE application and for collection of variables for the full
        track quality estimator.
        """
        path = basf2.create_path()
        path.add_module(
            "RootInput",
            inputFileNames=self.get_input_file_names(GenerateSimTask.output_file_name),
        )
        path.add_module("Gearbox")

        # First add tracking reconstruction with default quality estimation modules
        tracking.add_tracking_reconstruction(path, add_cdcTrack_QI=True, add_vxdTrack_QI=True, add_recoTrack_QI=True)

        # Replace weightfile identifiers from defaults (CDB payloads) to new
        # weightfiles created by this b2luigi script
        cdc_qe_mva_filter_parameters = {
            "identifier":
            self.get_input_file_names(CDCQETeacherTask.get_weightfile_xml_identifier(CDCQETeacherTask))[0]
        }
        basf2.set_module_parameters(
            path,
            name="TFCDC_TrackQualityEstimator",
            filterParameters=cdc_qe_mva_filter_parameters,
        )
        basf2.set_module_parameters(
            path,
            name="VXDQualityEstimatorMVA",
            WeightFileIdentifier=self.get_input_file_names(
                VXDQETeacherTask.get_weightfile_xml_identifier(VXDQETeacherTask)
            )[0],
        )

        # Replace final quality estimator module by training data collector module
        track_qe_module_name = "TrackQualityEstimatorMVA"
        module_found = False
        new_path = basf2.create_path()
        for module in path.modules():
            if module.name() != track_qe_module_name:
                new_path.add_module(module)
            else:
                new_path.add_module(
                    "TrackQETrainingDataCollector",
                    TrainingDataOutputName=self.get_output_file_name(self.records_file_name),
                    collectEventFeatures=True,
                    SVDPlusCDCStandaloneRecoTracksStoreArrayName="SVDPlusCDCStandaloneRecoTracks",
                )
                module_found = True
        if not module_found:
            raise KeyError(f"No module {track_qe_module_name} found in path")
        path = new_path
        return path


class TrackQETeacherBaseTask(Basf2Task):
    """
    A teacher task runs the basf2 mva teacher on the training data provided by a
    data collection task.

    Since teacher tasks are needed for all quality estimators covered by this
    steering file and the only thing that changes is the required data
    collection task and some training parameters, I decided to use inheritance
    and have the basic functionality in this base class/interface and have the
    specific teacher tasks inherit from it.
    """
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Feature/variable to use as truth label in the quality estimator MVA classifier.
    training_target = b2luigi.Parameter(default="truth")
    #: List of collected variables to not use in the training of the QE MVA classifier.
    # In addition to variables containing the "truth" substring, which are excluded by default.
    exclude_variables = b2luigi.ListParameter(hashed=True, default=[])
    #: Hyperparameter option of the FastBDT algorithm. default are the FastBDT default values.
    fast_bdt_option = b2luigi.ListParameter(hashed=True, default=[200, 8, 3, 0.1])

    @property
    def weightfile_identifier_basename(self):
        """
        Property defining the basename for the .xml and .root weightfiles that are created.
        Has to be implemented by the inheriting teacher task class.
        """
        raise NotImplementedError(
            "Teacher Task must define a static weightfile_identifier"
        )

    def get_weightfile_xml_identifier(self, fbdt_option):
        """
        Name of the xml weightfile that is created by the teacher task.
        It is subsequently used as a local weightfile in the following validation tasks.
        """

        weightfile_details = create_fbdt_option_string(fbdt_option)
        return self.weightfile_identifier_basename + weightfile_details + ".weights.xml"

    @property
    def tree_name(self):
        """
        Property defining the name of the tree in the ROOT file from the
        ``data_collection_task`` that contains the recorded training data.  Must
        implemented by the inheriting specific teacher task class.
        """
        raise NotImplementedError("Teacher Task must define a static tree_name")

    @property
    def random_seed(self):
        """
        Property defining random seed to be used by the ``GenerateSimTask``.
        Should differ from the random seed in the test data samples.  Must
        implemented by the inheriting specific teacher task class.
        """
        raise NotImplementedError("Teacher Task must define a static random seed")

    @property
    def data_collection_task(self) -> Basf2PathTask:
        """
        Property defining the specific ``DataCollectionTask`` to require.  Must
        implemented by the inheriting specific teacher task class.
        """
        raise NotImplementedError(
            "Teacher Task must define a data collection task to require "
        )

    # def requires(self):
    #    """
    #    Generate list of luigi Tasks that this Task depends on.
    #    """
    #    yield self.data_collection_task(
    #        num_processes=MasterTask.num_processes,
    #        n_events=self.n_events_training,
    #        experiment_number=self.experiment_number,
    #        random_seed=self.random_seed,
    #    )

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished iff the outputs all exist.
        """
        yield self.add_to_output(self.get_weightfile_xml_identifier(self.fast_bdt_option))

    def process(self):
        """
        Use basf2_mva teacher to create MVA weightfile from collected training
        data variables.

        This is the main process that is dispatched by the ``run`` method that
        is inherited from ``Basf2Task``.
        """
        # records_files = self.get_input_file_names(
        #    self.data_collection_task.records_file_name
        # )

        records_files = ['datafiles/10k_events/cdc_qe_records.root']

        my_basf2_mva_teacher(
            records_files=records_files,
            tree_name=self.tree_name,
            weightfile_identifier=self.get_output_file_name(self.get_weightfile_xml_identifier(self.fast_bdt_option)),
            target_variable=self.training_target,
            exclude_variables=self.exclude_variables,
            fast_bdt_option=self.fast_bdt_option,
        )


class VXDQETeacherTask(TrackQETeacherBaseTask):
    """
    Task to run basf2 mva teacher on collected data for VXDTF2 track quality estimator
    """
    #: Name of the weightfile that is created.
    weightfile_identifier_basename = "vxdtf2_mva_qe_weightFile_noTiming"
    #: Name of the TTree in the ROOT file from the ``data_collection_task`` that
    # contains the training data for the MVA teacher.
    tree_name = "tree"
    #: Random basf2 seed used to create the training data set.
    random_seed = "trainvxd_0"
    #: Defines DataCollectionTask to require by tha base class to collect
    # features for the MVA training.
    data_collection_task = VXDQEDataCollectionTask


class CDCQETeacherTask(TrackQETeacherBaseTask):
    """
    Task to run basf2 mva teacher on collected data for CDC track quality estimator
    """
    #: Name of the weightfile that is created.
    weightfile_identifier_basename = "trackfindingcdc_TrackQualityIndicator"
    #: Name of the TTree in the ROOT file from the ``data_collection_task`` that
    # contains the training data for the MVA teacher.
    tree_name = "records"
    #: Random basf2 seed used to create the training data set.
    random_seed = "traincdc_0"
    #: Defines DataCollectionTask to require by tha base class to collect
    # features for the MVA training.
    data_collection_task = CDCQEDataCollectionTask


class FullTrackQETeacherTask(TrackQETeacherBaseTask):
    """
    Task to run basf2 mva teacher on collected data for the final, combined
    track quality estimator
    """
    #: Name of the weightfile that is created.
    weightfile_identifier_basename = "trackfitting_MVATrackQualityIndicator"
    #: Name of the TTree in the ROOT file from the ``data_collection_task`` that
    # contains the training data for the MVA teacher.
    tree_name = "tree"
    #: Random basf2 seed used to create the training data set.
    random_seed = "trainingdata_0"
    #: Defines DataCollectionTask to require by tha base class to collect
    # features for the MVA training.
    data_collection_task = FullTrackQEDataCollectionTask
    #: Feature/variable to use as truth label for the CDC track quality estimator.
    cdc_training_target = b2luigi.Parameter()

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield self.data_collection_task(
            cdc_training_target=self.cdc_training_target,
            num_processes=MasterTask.num_processes,
            n_events=self.n_events_training,
            experiment_number=self.experiment_number,
            random_seed=self.random_seed,
        )


class HarvestingValidationBaseTask(Basf2PathTask):
    """
    Run track reconstruction with MVA quality estimator and write out
    (="harvest") a root file with variables useful for the validation.
    """

    #: Number of events to generate for the test data set.
    n_events_testing = b2luigi.IntParameter()
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Name of the "harvested" ROOT output file with variables that can be used for validation.
    validation_output_file_name = "harvesting_validation.root"
    #: Name of the output of the RootOutput module with reconstructed events.
    reco_output_file_name = "reconstruction.root"
    #: Components for the MC matching and track fit creation.
    components = None

    @property
    def teacher_task(self) -> TrackQETeacherBaseTask:
        """
        Teacher task to require to provide a quality estimator weightfile for ``add_tracking_with_quality_estimation``
        """
        raise NotImplementedError()

    def add_tracking_with_quality_estimation(self, path: basf2.Path) -> None:
        """
        Add modules for track reconstruction to basf2 path that are to be
        validated.  Besides track finding it should include MC matching, fitted
        track creation and a quality estimator module.
        """
        raise NotImplementedError()

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield self.teacher_task(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
        )
        yield GenerateSimTask(
            bkgfiles_dir=MasterTask.bkgfiles_by_exp[self.experiment_number],
            num_processes=MasterTask.num_processes,
            n_events=self.n_events_testing,
            experiment_number=self.experiment_number,
            random_seed="testdata_0",
        )

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished iff the outputs all exist.
        """
        yield self.add_to_output(self.validation_output_file_name)
        yield self.add_to_output(self.reco_output_file_name)

    def create_path(self):
        """
        Create a basf2 path that uses ``add_tracking_with_quality_estimation()``
        and adds the ``CombinedTrackingValidationModule`` to write out variables
        for validation.
        """
        # prepare track finding
        path = basf2.create_path()
        path.add_module(
            "RootInput",
            inputFileNames=self.get_input_file_names(GenerateSimTask.output_file_name),
        )
        path.add_module("Gearbox")
        tracking.add_geometry_modules(path)
        tracking.add_hit_preparation_modules(path)  # only needed for simulated hits
        # add track finding module that needs to be validated
        self.add_tracking_with_quality_estimation(path)
        # add modules for validation
        path.add_module(
            CombinedTrackingValidationModule(
                name=None,
                contact=None,
                expert_level=200,
                output_file_name=self.get_output_file_name(
                    self.validation_output_file_name
                ),
            )
        )
        path.add_module(
            "RootOutput",
            outputFileName=self.get_output_file_name(self.reco_output_file_name),
        )
        return path


class VXDQEHarvestingValidationTask(HarvestingValidationBaseTask):
    """
    Run VXDTF2 track reconstruction and write out (="harvest") a root file with
    variables useful for validation of the VXD Quality Estimator.
    """

    #: Name of the "harvested" ROOT output file with variables that can be used for validation.
    validation_output_file_name = "vxd_qe_harvesting_validation.root"
    #: Name of the output of the RootOutput module with reconstructed events.
    reco_output_file_name = "vxd_qe_reconstruction.root"
    #: Teacher task to require to provide a quality estimator weightfile for ``add_tracking_with_quality_estimation``
    teacher_task = VXDQETeacherTask

    def add_tracking_with_quality_estimation(self, path):
        """
        Add modules for VXDTF2 tracking with VXD quality estimator to basf2 path.
        """
        tracking.add_vxd_track_finding_vxdtf2(
            path,
            components=["SVD"],
            reco_tracks="RecoTracks",
            add_mva_quality_indicator=True,
        )
        # Replace the weightfiles of all quality estimator module by those
        # produced in this training by b2luigi
        basf2.set_module_parameters(
            path,
            name="VXDQualityEstimatorMVA",
            WeightFileIdentifier=self.get_input_file_names(
                self.teacher_task.get_weightfile_xml_identifier(self.teacher_task)
            )[0],
        )
        tracking.add_mc_matcher(path, components=["SVD"])
        tracking.add_track_fit_and_track_creator(path, components=["SVD"])


class CDCQEHarvestingValidationTask(HarvestingValidationBaseTask):
    """
    Run CDC reconstruction and write out (="harvest") a root file with variables
    useful for validation of the CDC Quality Estimator.
    """
    #: Feature/variable to use as truth label in the quality estimator MVA classifier.
    training_target = b2luigi.Parameter()
    #: Name of the "harvested" ROOT output file with variables that can be used for validation.
    validation_output_file_name = "cdc_qe_harvesting_validation.root"
    #: Name of the output of the RootOutput module with reconstructed events.
    reco_output_file_name = "cdc_qe_reconstruction.root"
    #: Teacher task to require to provide a quality estimator weightfile for ``add_tracking_with_quality_estimation``
    teacher_task = CDCQETeacherTask

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield self.teacher_task(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            training_target=self.training_target,
        )
        yield GenerateSimTask(
            bkgfiles_dir=MasterTask.bkgfiles_by_exp[self.experiment_number],
            num_processes=MasterTask.num_processes,
            n_events=self.n_events_testing,
            experiment_number=self.experiment_number,
            random_seed="testdata_0",
        )

    def add_tracking_with_quality_estimation(self, path):
        """
        Add modules for CDC standalone tracking with CDC quality estimator to basf2 path.
        """
        tracking.add_cdc_track_finding(
            path,
            output_reco_tracks="RecoTracks",
            add_mva_quality_indicator=True,
        )
        # change weightfile of quality estimator to the one produced by this training script
        cdc_qe_mva_filter_parameters = {
            "identifier":
            self.get_input_file_names(CDCQETeacherTask.get_weightfile_xml_identifier(CDCQETeacherTask))[0]

        }
        basf2.set_module_parameters(
            path,
            name="TFCDC_TrackQualityEstimator",
            filterParameters=cdc_qe_mva_filter_parameters,
        )
        tracking.add_mc_matcher(path, components=["CDC"])
        tracking.add_track_fit_and_track_creator(path, components=["CDC"])


class FullTrackQEHarvestingValidationTask(HarvestingValidationBaseTask):
    """
    Run track reconstruction and write out (="harvest") a root file with variables
    useful for validation of the MVA track Quality Estimator.
    """
    #: Number of events to generate for the test data set.
    n_events_testing = b2luigi.IntParameter()
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Feature/variable to use as truth label for the CDC track quality estimator.
    cdc_training_target = b2luigi.Parameter()
    #: List of collected variables to not use in the training of the QE MVA classifier.
    # In addition to variables containing the "truth" substring, which are excluded by default.
    exclude_variables = b2luigi.ListParameter(hashed=True)
    #: Name of the "harvested" ROOT output file with variables that can be used for validation.
    validation_output_file_name = "full_qe_harvesting_validation.root"
    #: Name of the output of the RootOutput module with reconstructed events.
    reco_output_file_name = "full_qe_reconstruction.root"
    #: Teacher task to require to provide a quality estimator weightfile for ``add_tracking_with_quality_estimation``
    teacher_task = FullTrackQETeacherTask

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield CDCQETeacherTask(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            training_target=self.cdc_training_target,
        )
        yield VXDQETeacherTask(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
        )
        yield self.teacher_task(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            exclude_variables=self.exclude_variables,
            cdc_training_target=self.cdc_training_target,
        )
        yield GenerateSimTask(
            bkgfiles_dir=MasterTask.bkgfiles_by_exp[self.experiment_number],
            experiment_number=self.experiment_number,
            num_processes=MasterTask.num_processes,
            n_events=self.n_events_testing,
            random_seed="testdata_0",
        )

    def add_tracking_with_quality_estimation(self, path):
        """
        Add modules for full tracking with all track quality estimators to basf2 path.
        """

        # add tracking recontonstruction with quality estimator modules added
        tracking.add_tracking_reconstruction(
            path,
            add_cdcTrack_QI=True,
            add_vxdTrack_QI=True,
            add_recoTrack_QI=True,
            skipGeometryAdding=True,
            skipHitPreparerAdding=False,
        )

        # Replace the weightfiles of all quality estimator modules by those
        # produced in the training by b2luigi
        cdc_qe_mva_filter_parameters = {
            "identifier":
            self.get_input_file_names(CDCQETeacherTask.get_weightfile_xml_identifier(CDCQETeacherTask))[0]
        }
        basf2.set_module_parameters(
            path,
            name="TFCDC_TrackQualityEstimator",
            filterParameters=cdc_qe_mva_filter_parameters,
        )
        basf2.set_module_parameters(
            path,
            name="VXDQualityEstimatorMVA",
            WeightFileIdentifier=self.get_input_file_names(
                VXDQETeacherTask.get_weightfile_xml_identifier(VXDQETeacherTask)
            )[0],
        )
        basf2.set_module_parameters(
            path,
            name="TrackQualityEstimatorMVA",
            WeightFileIdentifier=self.get_input_file_names(
                FullTrackQETeacherTask.get_weightfile_xml_identifier(FullTrackQETeacherTask)
            )[0],
        )


class TrackQEEvaluationBaseTask(Task):
    """
    Base class for evaluating a quality estimator ``basf2_mva_evaluate.py`` on a
    separate test data set.

    Evaluation tasks for VXD, CDC and combined QE can inherit from it.
    """

    #: Use git hash / release of basf2 version as additional luigi parameter.
    # This parameter is already set in all other tasks that inherit from
    # ``Basf2Task``. For this task, I decided against inheriting from
    # ``Basf2Task`` because it already calls a subprocess and therefore does not
    # need a dispatchable ``process`` method.
    git_hash = b2luigi.Parameter(default=get_basf2_git_hash())
    #: Number of events to generate for the test data set.
    n_events_testing = b2luigi.IntParameter()
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Feature/variable to use as truth label in the quality estimator MVA classifier.
    training_target = b2luigi.Parameter(default="truth")
    #: Hyperparameter options for the FastBDT algorithm.
    fast_bdt_option = b2luigi.ListParameter(hashed=True, default=[200, 8, 3, 0.1])

    @property
    def teacher_task(self) -> TrackQETeacherBaseTask:
        """
        Property defining specific teacher task to require.
        """
        raise NotImplementedError(
            "Evaluation Tasks must define a teacher task to require "
        )

    @property
    def data_collection_task(self) -> Basf2PathTask:
        """
        Property defining the specific ``DataCollectionTask`` to require.  Must
        implemented by the inheriting specific teacher task class.
        """
        raise NotImplementedError(
            "Evaluation Tasks must define a data collection task to require "
        )

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield self.teacher_task(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            training_target=self.training_target,
            fast_bdt_option=self.fast_bdt_option,
        )
        # yield self.data_collection_task(
        #    num_processes=MasterTask.num_processes,
        #    n_events=self.n_events_testing,
        #    experiment_number=self.experiment_number,
        #    random_seed="testdata_0",
        # )

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished iff the outputs all exist.
        """
        weightfile_details = create_fbdt_option_string(self.fast_bdt_option)
        evaluation_pdf_output = self.teacher_task.weightfile_identifier_basename + weightfile_details + ".pdf"
        yield self.add_to_output(evaluation_pdf_output)

    @b2luigi.on_temporary_files
    def run(self):
        """
        Run ``basf2_mva_evaluate.py`` subprocess to evaluate QE MVA.

        The MVA weight file created from training on the training data set is
        evaluated on separate test data.
        """
        weightfile_details = create_fbdt_option_string(self.fast_bdt_option)
        evaluation_pdf_output_basename = self.teacher_task.weightfile_identifier_basename + weightfile_details + ".pdf"

        evaluation_pdf_output_path = self.get_output_file_name(evaluation_pdf_output_basename)

        cmd = [
            "basf2_mva_evaluate.py",
            "--identifiers",
            self.get_input_file_names(
                self.teacher_task.get_weightfile_xml_identifier(
                    self.teacher_task,
                    fbdt_option=self.fast_bdt_option))[0],
            # self.get_input_file_names(self.teacher_task.get_weightfile_xml_identifier(self.teacher_task))[0],
            "--datafiles",
            # self.get_input_file_names(self.data_collection_task.records_file_name)[0],
            "datafiles/1k_events/cdc_qe_records.root",
            "--treename",
            self.teacher_task.tree_name,
            "--outputfile",
            evaluation_pdf_output_path,
        ]

        # Prepare log files
        log_file_dir = get_log_file_dir(self)
        # check if directory already exists, if not, create it. I think this is necessary as this task does not
        # inherit properly from b2luigi and thus does not do it automatically??
        try:
            os.makedirs(log_file_dir, exist_ok=True)
        # the following should be unnecessary as exist_ok=True should take care that no FileExistError rises. I
        # might ask about a permission error...
        except FileExistsError:
            print('Directory ' + log_file_dir + 'already exists.')
        stderr_log_file_path = log_file_dir + "stderr"
        stdout_log_file_path = log_file_dir + "stdout"
        with open(stdout_log_file_path, "w") as stdout_file:
            stdout_file.write("stdout output of the command:\n{}\n\n".format(" ".join(cmd)))
        if os.path.exists(stderr_log_file_path):
            # remove stderr file if it already exists b/c in the following it will be opened in appending mode
            os.remove(stderr_log_file_path)

        # Run evaluation via subprocess and write output into logfiles
        with open(stdout_log_file_path, "a") as stdout_file:
            with open(stderr_log_file_path, "a") as stderr_file:
                try:
                    subprocess.run(cmd, check=True, stdin=stdout_file, stderr=stderr_file)
                except subprocess.CalledProcessError as err:
                    stderr_file.write(f"Evaluation failed with error:\n{err}")
                    raise err


class VXDTrackQEEvaluationTask(TrackQEEvaluationBaseTask):
    """
    Run ``basf2_mva_evaluate.py`` for the VXD quality estimator on separate test data
    """
    #: Task that is required by the evaluation base class to create the MVA
    # weightfile that needs to be evaluated.
    teacher_task = VXDQETeacherTask
    #: Task that is required by the evaluation base class to collect the test
    # data for the evaluation.
    data_collection_task = VXDQEDataCollectionTask


class CDCTrackQEEvaluationTask(TrackQEEvaluationBaseTask):
    """
    Run ``basf2_mva_evaluate.py`` for the CDC quality estimator on separate test data
    """
    #: Task that is required by the evaluation base class to create the MVA
    # weightfile that needs to be evaluated.
    teacher_task = CDCQETeacherTask
    #: Task that is required by the evaluation base class to collect the test
    # data for the evaluation.
    data_collection_task = CDCQEDataCollectionTask


class FullTrackQEEvaluationTask(TrackQEEvaluationBaseTask):
    """
    Run ``basf2_mva_evaluate.py`` for the final, combined quality estimator on
    separate test data
    """
    #: Task that is required by the evaluation base class to create the MVA
    # weightfile that needs to be evaluated.
    teacher_task = FullTrackQETeacherTask
    #: Task that is required by the evaluation base class to collect the test
    # data for the evaluation.
    data_collection_task = FullTrackQEDataCollectionTask

    #: List of collected variables to not use in the training of the QE MVA classifier.
    # In addition to variables containing the "truth" substring, which are excluded by default.
    exclude_variables = b2luigi.ListParameter(hashed=True)
    #: Feature/variable to use as truth label for the CDC track quality estimator.

    cdc_training_target = b2luigi.Parameter()

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield self.teacher_task(
            exclude_variables=self.exclude_variables,
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            training_target=self.training_target,
            cdc_training_target=self.cdc_training_target,
        )
        yield self.data_collection_task(
            num_processes=MasterTask.num_processes,
            n_events=self.n_events_testing,
            experiment_number=self.experiment_number,
            random_seed="testdata_0",
            cdc_training_target=self.cdc_training_target,
        )


class PlotsFromHarvestingValidationBaseTask(Basf2Task):
    """
    Create a PDF file with validation plots for a quality estimator produced
    from the ROOT ntuples produced by a harvesting validation task
    """
    #: Whether to normalize the track finding efficiencies to primary particles only.
    primaries_only = b2luigi.BoolParameter(default=True)  # normalize finding efficiencies to primary MC-tracks

    @property
    def harvesting_validation_task_instance(self) -> HarvestingValidationBaseTask:
        """
        Specifies related harvesting validation task which produces the ROOT
        files with the data that is plotted by this task.
        """
        raise NotImplementedError("Must define a QI harvesting validation task for which to do the plots")

    @property
    def output_pdf_file_basename(self):
        """
        Name of the output PDF file containing the validation plots
        """
        validation_harvest_basename = self.harvesting_validation_task_instance.validation_output_file_name
        return validation_harvest_basename.replace(".root", "_plots.pdf")

    def requires(self):
        """
        Generate list of luigi Tasks that this Task depends on.
        """
        yield self.harvesting_validation_task_instance

    def output(self):
        """
        Generate list of output files that the task should produce.
        The task is considered finished iff the outputs all exist.
        """
        yield self.add_to_output(self.output_pdf_file_basename)

    @b2luigi.on_temporary_files
    def process(self):
        """
        Use basf2_mva teacher to create MVA weightfile from collected training
        data variables.

        Main process that is dispatched by the ``run`` method that is inherited
        from ``Basf2Task``.
        """
        # get the validation "harvest", which is the ROOT file with ntuples for validation
        validation_harvest_basename = self.harvesting_validation_task_instance.validation_output_file_name
        validation_harvest_path = self.get_input_file_names(validation_harvest_basename)[0]

        # Load "harvested" validation data from root files into dataframes (requires enough memory to hold data)
        pr_columns = [  # Restrict memory usage by only reading in columns that are used in the steering file
            'is_fake', 'is_clone', 'is_matched', 'quality_indicator',
            'experiment_number', 'run_number', 'event_number', 'pr_store_array_number',
            'pt_estimate', 'z0_estimate', 'd0_estimate', 'tan_lambda_estimate',
            'phi0_estimate', 'pt_truth', 'z0_truth', 'd0_truth', 'tan_lambda_truth',
            'phi0_truth',
        ]
        # In ``pr_df`` each row corresponds to a track from Pattern Recognition
        pr_df = root_pandas.read_root(validation_harvest_path, key='pr_tree/pr_tree', columns=pr_columns)
        mc_columns = [  # restrict mc_df to these columns
            'experiment_number',
            'run_number',
            'event_number',
            'pr_store_array_number',
            'is_missing',
            'is_primary',
        ]
        # In ``mc_df`` each row corresponds to an MC track
        mc_df = root_pandas.read_root(validation_harvest_path, key='mc_tree/mc_tree', columns=mc_columns)
        if self.primaries_only:
            mc_df = mc_df[mc_df.is_primary.eq(True)]

        # Define QI thresholds for the FOM plots and the ROC curves
        qi_cuts = np.linspace(0., 1, 20, endpoint=False)
        # # Add more points at the very end between the previous maximum and 1
        # qi_cuts = np.append(qi_cuts, np.linspace(np.max(qi_cuts), 1, 20, endpoint=False))

        # Create plots and append them to single output pdf

        output_pdf_file_path = self.get_output_file_name(self.output_pdf_file_basename)
        with PdfPages(output_pdf_file_path, keep_empty=False) as pdf:

            # Add a title page to validation plot PDF with some metadata
            # Remember that most metadata is in the xml file of the weightfile
            # and in the b2luigi directory structure
            titlepage_fig, titlepage_ax = plt.subplots()
            titlepage_ax.axis("off")
            title = f"Quality Estimator validation plots from {self.__class__.__name__}"
            titlepage_ax.set_title(title)
            teacher_task = self.harvesting_validation_task_instance.teacher_task
            weightfile_identifier = teacher_task.get_weightfile_xml_identifier(teacher_task)
            meta_data = {
                "Date": datetime.today().strftime("%Y-%m-%d %H:%M"),
                "Created by steering file": os.path.realpath(__file__),
                "Created from data in": validation_harvest_path,
                "Background directory": MasterTask.bkgfiles_by_exp[self.experiment_number],
                "weight file": weightfile_identifier,
            }
            if hasattr(self, 'exclude_variables'):
                meta_data["Excluded variables"] = ", ".join(self.exclude_variables)
            meta_data_string = (format_dictionary(meta_data) +
                                "\n\n(For all MVA training parameters look into the produced weight file)")
            luigi_params = get_serialized_parameters(self)
            luigi_param_string = (f"\n\nb2luigi parameters for {self.__class__.__name__}\n" +
                                  format_dictionary(luigi_params))
            title_page_text = meta_data_string + luigi_param_string
            titlepage_ax.text(0, 1, title_page_text, ha="left", va="top", wrap=True, fontsize=8)
            pdf.savefig(titlepage_fig)
            plt.close(titlepage_fig)

            fake_rates = get_uncertain_means_for_qi_cuts(pr_df, "is_fake", qi_cuts)
            fake_fig, fake_ax = plt.subplots()
            fake_ax.set_title("Fake rate")
            plot_with_errobands(fake_rates, ax=fake_ax)
            fake_ax.set_ylabel("fake rate")
            fake_ax.set_xlabel("quality indicator requirement")
            pdf.savefig(fake_fig, bbox_inches="tight")
            plt.close(fake_fig)

            # Plot clone rates
            clone_rates = get_uncertain_means_for_qi_cuts(pr_df, "is_clone", qi_cuts)
            clone_fig, clone_ax = plt.subplots()
            clone_ax.set_title("Clone rate")
            plot_with_errobands(clone_rates, ax=clone_ax)
            clone_ax.set_ylabel("clone rate")
            clone_ax.set_xlabel("quality indicator requirement")
            pdf.savefig(clone_fig, bbox_inches="tight")
            plt.close(clone_fig)

            # Plot finding efficieny

            # The Quality Indicator is only avaiable in pr_tree and thus the
            # PR-track dataframe. To get the QI of the related PR track for an MC
            # track, merge the PR dataframe into the MC dataframe
            pr_track_identifiers = ['experiment_number', 'run_number', 'event_number', 'pr_store_array_number']
            mc_df = upd.merge(
                left=mc_df, right=pr_df[pr_track_identifiers + ['quality_indicator']],
                how='left',
                on=pr_track_identifiers
            )

            missing_fractions = (
                _my_uncertain_mean(mc_df[
                    mc_df.quality_indicator.isnull() | (mc_df.quality_indicator > qi_cut)]['is_missing'])
                for qi_cut in qi_cuts
            )

            findeff_fig, findeff_ax = plt.subplots()
            findeff_ax.set_title("Finding efficiency")
            finding_efficiencies = 1.0 - upd.Series(data=missing_fractions, index=qi_cuts)
            plot_with_errobands(finding_efficiencies, ax=findeff_ax)
            findeff_ax.set_ylabel("finding efficiency")
            findeff_ax.set_xlabel("quality indicator requirement")
            pdf.savefig(findeff_fig, bbox_inches="tight")
            plt.close(findeff_fig)

            # Plot ROC curves

            # Fake rate vs. finding efficiency ROC curve
            fake_roc_fig, fake_roc_ax = plt.subplots()
            fake_roc_ax.set_title("Fake rate vs. finding efficiency ROC curve")
            fake_roc_ax.errorbar(x=finding_efficiencies.nominal_value, y=fake_rates.nominal_value,
                                 xerr=finding_efficiencies.std_dev, yerr=fake_rates.std_dev, elinewidth=0.8)
            fake_roc_ax.set_xlabel('finding efficiency')
            fake_roc_ax.set_ylabel('fake rate')
            pdf.savefig(fake_roc_fig, bbox_inches="tight")
            plt.close(fake_roc_fig)

            # Clone rate vs. finding efficiency ROC curve
            clone_roc_fig, clone_roc_ax = plt.subplots()
            clone_roc_ax.set_title("Clone rate vs. finding efficiency ROC curve")
            clone_roc_ax.errorbar(x=finding_efficiencies.nominal_value, y=clone_rates.nominal_value,
                                  xerr=finding_efficiencies.std_dev, yerr=clone_rates.std_dev, elinewidth=0.8)
            clone_roc_ax.set_xlabel('finding efficiency')
            clone_roc_ax.set_ylabel('clone rate')
            pdf.savefig(clone_roc_fig, bbox_inches="tight")
            plt.close(clone_roc_fig)

            # Plot kinematic distributions

            # use fewer qi cuts as each cut will be it's own subplot now and not a point
            kinematic_qi_cuts = [0, 0.5, 0.9]

            # Define kinematic parameters which we want to histogram and define
            # dictionaries relating them to latex labels, units and binnings
            params = ['d0', 'z0', 'pt', 'tan_lambda', 'phi0']
            label_by_param = {
                "pt": "$p_T$",
                "z0": "$z_0$",
                "d0": "$d_0$",
                "tan_lambda": r"$\tan{\lambda}$",
                "phi0": r"$\phi_0$"
            }
            unit_by_param = {
                "pt": "GeV",
                "z0": "cm",
                "d0": "cm",
                "tan_lambda": "rad",
                "phi0": "rad"
            }
            n_kinematic_bins = 75  # number of bins per kinematic variable
            bins_by_param = {
                "pt": np.linspace(0, np.percentile(pr_df['pt_truth'].dropna(), 95), n_kinematic_bins),
                "z0": np.linspace(-0.1, 0.1, n_kinematic_bins),
                "d0": np.linspace(0, 0.01, n_kinematic_bins),
                "tan_lambda": np.linspace(-2, 3, n_kinematic_bins),
                "phi0": np.linspace(0, 2 * np.pi, n_kinematic_bins)
            }

            # Iterate over each parameter and for each make stacked histograms for different QI cuts
            kinematic_qi_cuts = [0, 0.5, 0.8]
            blue, yellow, green = plt.get_cmap("tab10").colors[0:3]
            for param in params:
                fig, axarr = plt.subplots(ncols=len(kinematic_qi_cuts), sharey=True, sharex=True, figsize=(14, 6))
                fig.suptitle(f"{label_by_param[param]}  distributions")
                for i, qi in enumerate(kinematic_qi_cuts):
                    ax = axarr[i]
                    ax.set_title(f"QI > {qi}")
                    incut = pr_df[(pr_df['quality_indicator'] > qi)]
                    incut_matched = incut[incut.is_matched.eq(True)]
                    incut_clones = incut[incut.is_clone.eq(True)]
                    incut_fake = incut[incut.is_fake.eq(True)]

                    # if any series is empty, break ouf loop and don't draw try to draw a stacked histogram
                    if any(series.empty for series in (incut, incut_matched, incut_clones, incut_fake)):
                        ax.text(0.5, 0.5, "Not enough data in bin", ha="center", va="center", transform=ax.transAxes)
                        continue

                    bins = bins_by_param[param]
                    stacked_histogram_series_tuple = (
                        incut_matched[f'{param}_estimate'],
                        incut_clones[f'{param}_estimate'],
                        incut_fake[f'{param}_estimate'],
                    )
                    histvals, _, _ = ax.hist(stacked_histogram_series_tuple,
                                             stacked=True,
                                             bins=bins, range=(bins.min(), bins.max()),
                                             color=(blue, green, yellow),
                                             label=("matched", "clones", "fakes"))
                    ax.set_xlabel(f'{label_by_param[param]} estimate / ({unit_by_param[param]})')
                    ax.set_ylabel('# tracks')
                axarr[0].legend(loc="upper center", bbox_to_anchor=(0, -0.15))
                pdf.savefig(fig, bbox_inches="tight")
                plt.close(fig)


class VXDQEValidationPlotsTask(PlotsFromHarvestingValidationBaseTask):
    """
    Create a PDF file with validation plots for the VXDTF2 track quality
    estimator produced from the ROOT ntuples produced by a VXDTF2 track QE
    harvesting validation task
    """
    #: Number of events to generate for the test data set.
    n_events_testing = b2luigi.IntParameter()
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()

    @property
    def harvesting_validation_task_instance(self):
        """
        Harvesting validation task to require, which produces the ROOT files
        with variables to produce the VXD QE validation plots.
        """
        return VXDQEHarvestingValidationTask(
            n_events_testing=self.n_events_testing,
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            num_processes=MasterTask.num_processes,
        )


class CDCQEValidationPlotsTask(PlotsFromHarvestingValidationBaseTask):
    """
    Create a PDF file with validation plots for the CDC track quality estimator
    produced from the ROOT ntuples produced by a CDC track QE harvesting
    validation task
    """
    #: Number of events to generate for the test data set.
    n_events_testing = b2luigi.IntParameter()
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Feature/variable to use as truth label in the quality estimator MVA classifier.
    training_target = b2luigi.Parameter()

    @property
    def harvesting_validation_task_instance(self):
        """
        Harvesting validation task to require, which produces the ROOT files
        with variables to produce the CDC QE validation plots.
        """
        return CDCQEHarvestingValidationTask(
            n_events_testing=self.n_events_testing,
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            training_target=self.training_target,
            num_processes=MasterTask.num_processes,
        )


class FullTrackQEValidationPlotsTask(PlotsFromHarvestingValidationBaseTask):
    """
    Create a PDF file with validation plots for the full MVA track quality
    estimator produced from the ROOT ntuples produced by a full track QE
    harvesting validation task
    """
    #: Number of events to generate for the test data set.
    n_events_testing = b2luigi.IntParameter()
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: Feature/variable to use as truth label for the CDC track quality estimator.
    cdc_training_target = b2luigi.Parameter()
    #: List of collected variables to not use in the training of the QE MVA classifier.
    # In addition to variables containing the "truth" substring, which are excluded by default.
    exclude_variables = b2luigi.ListParameter(hashed=True)

    @property
    def harvesting_validation_task_instance(self):
        """
        Harvesting validation task to require, which produces the ROOT files
        with variables to produce the final MVA track QE validation plots.
        """
        return FullTrackQEHarvestingValidationTask(
            n_events_testing=self.n_events_testing,
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            cdc_training_target=self.cdc_training_target,
            exclude_variables=self.exclude_variables,
            num_processes=MasterTask.num_processes,
        )


class QEWeightsLocalDBCreatorTask(Basf2Task):
    """
    Collect weightfile identifiers from different teacher tasks and merge them
    into a local database for testing.
    """
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.IntParameter()
    #: Experiment number of the conditions database, e.g. defines simulation geometry
    experiment_number = b2luigi.IntParameter()
    #: List of collected variables to not use in the training of the QE MVA classifier.
    # In addition to variables containing the "truth" substring, which are excluded by default.
    exclude_variables = b2luigi.ListParameter(hashed=True, default=[])
    #: Feature/vaiable to use as truth label for the CDC track quality estimator.
    cdc_training_target = b2luigi.Parameter()

    def requires(self):
        """
        Required teacher tasks
        """
        yield VXDQETeacherTask(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
        )
        yield CDCQETeacherTask(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            training_target=self.cdc_training_target,
        )
        yield FullTrackQETeacherTask(
            n_events_training=self.n_events_training,
            experiment_number=self.experiment_number,
            cdc_training_target=self.cdc_training_target,
            exclude_variables=self.exclude_variables,
        )

    def output(self):
        """
        Local database
        """
        yield self.add_to_output("localdb.tar")

    def process(self):
        """
        Create local database
        """
        current_path = Path.cwd()
        localdb_archive_path = Path(self.get_output_file_name("localdb.tar")).absolute()
        output_dir = localdb_archive_path.parent

        # remove existing local databases in output directories
        self._clean()
        # "Upload" the weightfiles of all 3 teacher tasks into the same localdb
        for task in (VXDQETeacherTask, CDCQETeacherTask, FullTrackQETeacherTask):
            # Extract xml identifier input file name before switching working directories, as it returns relative paths
            weightfile_xml_identifier_path = os.path.abspath(self.get_input_file_names(
                task.get_weightfile_xml_identifier(task))[0])
            # As localdb is created in working directory, chdir into desired output path
            try:
                os.chdir(output_dir)
                # Same as basf2_mva_upload on the command line, creates localdb directory in current working dir
                basf2_mva.upload(
                    weightfile_xml_identifier_path,
                    task.weightfile_identifier_basename,
                    self.experiment_number, 0,
                    self.experiment_number, -1,
                )
            finally:  # Switch back to working directory of b2luigi, even if upload failed
                os.chdir(current_path)

        # Pack localdb into tar archive, so that we can have on single output file instead
        shutil.make_archive(
            base_name=localdb_archive_path.as_posix().split('.')[0],
            format="tar",
            root_dir=output_dir,
            base_dir="localdb",
            verbose=True,
        )

    def _clean(self):
        """
        Remove local database and tar archives in output directory
        """
        localdb_archive_path = Path(self.get_output_file_name("localdb.tar"))
        localdb_path = localdb_archive_path.parent / "localdb"

        if localdb_path.exists():
            print(f"Deleting localdb\n{localdb_path}\nwith contents\n ",
                  "\n  ".join(f.name for f in localdb_path.iterdir()))
            shutil.rmtree(localdb_path, ignore_errors=False)  # recursively delete localdb

        if localdb_archive_path.is_file():
            print(f"Deleting {localdb_archive_path}")
            os.remove(localdb_archive_path)

    def on_failure(self, exception):
        """
        Cleanup: Remove local database to prevent existing outputs when task did not finish successfully
        """
        self._clean()
        # Run existing on_failure from parent class
        super().on_failure(exception)


class MasterTask(b2luigi.WrapperTask):
    """
    Wrapper task that needs to finish for b2luigi to finish running this steering file.

    It is done if the outputs of all required subtasks exist.  It is thus at the
    top of the luigi task graph.  Edit the ``requires`` method to steer which
    tasks and with which parameters you want to run.
    """
    #: Number of events to generate for the training data set.
    n_events_training = b2luigi.get_setting("n_events_training", default=3000)
    #: Number of events to generate for the test data set.
    n_events_testing = b2luigi.get_setting("n_events_testing", default=1000)
    #: Number of basf2 processes to use in Basf2PathTasks
    num_processes = b2luigi.get_setting("basf2_processes_per_worker", default=0)
    #: Dictionary with experiment numbers as keys and background directory paths as values
    bkgfiles_by_exp = b2luigi.get_setting("bkgfiles_by_exp")
    #: Transform dictionary keys (exp. numbers) from strings to int
    bkgfiles_by_exp = {int(key): val for (key, val) in bkgfiles_by_exp.items()}

    def requires(self):
        """
        Generate list of tasks that needs to be done for luigi to finish running
        this steering file.
        """

        # eventwise n_track_variables should not be used by teacher tasks in training
        ntrack_variables = [
            "N_RecoTracks",
            "N_PXDRecoTracks",
            "N_SVDRecoTracks",
            "N_CDCRecoTracks",
            "N_diff_PXD_SVD_RecoTracks",
            "N_diff_SVD_CDC_RecoTracks",
        ]
        exclude_variables_combinations = [ntrack_variables]

        cdc_training_targets = [
            "truth",  # treats clones as signal
            # "truth_track_is_matched"  # treats clones as backround, only best matched CDC tracks are true
        ]

        fast_bdt_options = []
        for i in range(0, 300, 100):  # 1100, 100):
            for j in range(0, 1):  # 5):
                for k in range(0, 1):  # 4):
                    fast_bdt_options.append([100 + i, 8, 3+j, 0.05+k*0.05])

        experiment_numbers = b2luigi.get_setting("experiment_numbers")

        # iterate over all possible combinations of parameters from the above defined parameter lists
        for experiment_number, exclude_variables, cdc_training_target, fast_bdt_option in itertools.product(
                experiment_numbers, exclude_variables_combinations, cdc_training_targets, fast_bdt_options
        ):
            # yield QEWeightsLocalDBCreatorTask(
            #    n_events_training=self.n_events_training,
            #    experiment_number=experiment_number,
            #    exclude_variables=exclude_variables,
            #    cdc_training_target=cdc_training_target,
            # )

            if b2luigi.get_setting("run_validation_tasks", default=True):
                yield FullTrackQEValidationPlotsTask(
                    cdc_training_target=cdc_training_target,
                    exclude_variables=exclude_variables,
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                    experiment_number=experiment_number,
                )
                yield CDCQEValidationPlotsTask(
                    training_target=cdc_training_target,
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                    experiment_number=experiment_number,
                )
                yield VXDQEValidationPlotsTask(
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                    experiment_number=experiment_number,
                )

            if b2luigi.get_setting("run_mva_evaluate", default=True):
                # Evaluate trained weightfiles via basf2_mva_evaluate.py on separate testdatasets
                # requires a latex installation to work
                # yield FullTrackQEEvaluationTask(
                #    exclude_variables=exclude_variables,
                #    cdc_training_target=cdc_training_target,
                #    n_events_training=self.n_events_training,
                #    n_events_testing=self.n_events_testing,
                #    experiment_number=experiment_number,
                # )
                yield CDCTrackQEEvaluationTask(
                    training_target=cdc_training_target,
                    n_events_training=self.n_events_training,
                    n_events_testing=self.n_events_testing,
                    fast_bdt_option=fast_bdt_option,
                    experiment_number=experiment_number,
                )
                # yield VXDTrackQEEvaluationTask(
                #    n_events_training=self.n_events_training,
                #    n_events_testing=self.n_events_testing,
                #    experiment_number=experiment_number,
                # )


if __name__ == "__main__":
    workers = b2luigi.get_setting("workers", default=1)
    b2luigi.process(MasterTask(), workers=workers)
