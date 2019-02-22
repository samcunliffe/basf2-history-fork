import subprocess
import os
import sys
import tempfile
from glob import glob

import basf2
import generators
from simulation import add_simulation
from rawdata import add_packers
from L1trigger import add_tsim
from softwaretrigger import constants
from softwaretrigger.constants import DEFAULT_EXPRESSRECO_COMPONENTS, RAWDATA_OBJECTS, DEFAULT_HLT_COMPONENTS
from ROOT import Belle2
find_file = Belle2.FileSystem.findFile


class CheckForCorrectHLTResults(basf2.Module):
    """Test module for assuring correct data store content"""

    def event(self):
        """reimplementation of Module::event()."""
        sft_trigger = Belle2.PyStoreObj("SoftwareTriggerResult")

        if not sft_trigger.isValid():
            basf2.B2FATAL("SoftwareTriggerResult object not created")
        elif len(sft_trigger.getResults()) == 0:
            basf2.B2FATAL("SoftwareTriggerResult exists but has no entries")

        if not Belle2.PyStoreArray("ROIs").isValid():
            basf2.B2FATAL("ROIs are not present")


def generate_input_file(run_type, location, output_file_name, exp_number):
    """
    Generate an input file for usage in the test.
    Simulate uubar for "beam" and two muons for "cosmic" setting.

    Only raw data will be stored to the given output file.
    :param run_type: Whether to simulate cosmic or beam
    :param location: Whether to simulate expressreco (with ROIs) or hlt (no PXD)
    :param output_file_name: where to store the result file
    :param exp_number: which experiment number to simulate
    """
    if os.path.exists(output_file_name):
        return

    basf2.set_random_seed(12345)

    path = basf2.Path()
    path.add_module('EventInfoSetter', evtNumList=[1], expList=[exp_number])

    if run_type == constants.RunTypes.beam:
        generators.add_continuum_generator(path, finalstate="uubar")
    elif run_type == constants.RunTypes.cosmic:
        # add something which looks a tiny bit like a cosmic generator. We
        # cannot use the normal cosmic generator as that needs a bigger
        # simulation top volume than the default geometry from the database.
        path.add_module("ParticleGun", pdgCodes=[-13, 13], momentumParams=[10, 200])

    add_simulation(path, usePXDDataReduction=(location == constants.Location.expressreco))
    add_tsim(path)

    if location == constants.Location.hlt:
        components = DEFAULT_HLT_COMPONENTS
    elif location == constants.Location.expressreco:
        components = DEFAULT_EXPRESSRECO_COMPONENTS
    else:
        basf2.B2FATAL("Location {} for test is not supported".format(location.name))

    components.append("TRG")

    add_packers(path, components=components)

    # remove everything but HLT input raw objects
    branch_names = RAWDATA_OBJECTS + ["EventMetaData", "TRGSummary"]
    if location == constants.Location.hlt:
        branch_names.remove("RawPXDs")
        branch_names.remove("ROIs")

    # There us no packer for these objects :-(
    branch_names.remove("RawTRGs")
    branch_names.remove("RawFTSWs")

    path.add_module("RootOutput", outputFileName=output_file_name, branchNames=branch_names)

    basf2.process(path)


def test_script(script_location, input_file_name, temp_dir):
    """
    Test a script with the given file path using the given input file.
    Raises an exception if the execution fails or if the needed output is not in
    the output file.
    The results are stored in the temporary directory location.

    :param script_location: the script to test
    :param input_file_name: the file path of the input file
    :param temp_dir: where to store and run
    """
    input_buffer = "UNUSED"   # unused
    output_buffer = "UNUSED"  # unused
    histo_port = 6666         # unused

    histos_file_name = os.path.join(temp_dir, "histos.root")
    output_file_name = os.path.join(temp_dir, "output.root")
    # TODO: should we use the default global tag here?
    central_database = basf2.get_default_global_tags()
    num_processes = 1

    cmd = [sys.executable, script_location,
           "--central-db-tag", central_database,
           "--input-file", input_file_name,
           "--histo-output-file", histos_file_name,
           "--output-file", output_file_name,
           input_buffer, output_buffer, str(histo_port), str(num_processes)]

    subprocess.check_call(cmd)

    test_path = basf2.Path()
    test_path.add_module("RootInput", inputFileName=output_file_name)
    test_path.add_module(CheckForCorrectHLTResults())

    if "beam_reco" in script_location:
        basf2.process(test_path)


def test_folder(location, run_type, exp_number, phase):
    """
    Run all hlt operation scripts in a given folder
    and test the outputs of the files.

    Will call the test_script function on all files in the folder given
    by the location after having created a suitable input file with the given
    experiment number.

    :param location: hlt or expressreco, depending on which operation files to run
                     and which input to simulate
    :param run_type: cosmic or beam, depending on which operation files to run
                     and which input to simulate
    :param exp_number: which experiment number to simulate
    :param phase:    where to look for the operation files (will search in the folder
                     hlt/operation/{phase}/global/{location}/evp_scripts/)
    """
    temp_dir = tempfile.mkdtemp()
    output_file_name = os.path.join(temp_dir, f"{location.name}_{run_type.name}.root")
    generate_input_file(run_type=run_type, location=location,
                        output_file_name=output_file_name, exp_number=exp_number)

    script_dir = find_file(f"hlt/operation/{phase}/global/{location.name}/evp_scripts/")
    run_at_least_one = False
    for script_location in glob(os.path.join(script_dir, f"{run_type.name}_*.py")):
        run_at_least_one = True
        test_script(script_location, input_file_name=output_file_name, temp_dir=temp_dir)

    assert run_at_least_one
