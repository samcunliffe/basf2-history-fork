import json
import subprocess
import validation_gt as vgt
import b2test_utils
import basf2

calibration_path = b2test_utils.require_file('calibration', 'validation')

with b2test_utils.clean_working_directory():

    # Prepare the configuration dictionary for CAF and dump it into a JSON file.
    config = {
        "caf_script": "caf_klm_channel_status.py",
        "database_chain": [globaltag for globaltag in reversed(vgt.get_validation_globaltags())],
        "requested_iov": [0, 0, -1, -1]
    }
    with open("config.json", "w") as config_json:
        json.dump(config, config_json)

    # Prepare the input files for the calibration and dump them into a JSON file.
    input_files = {
        "raw_physics": [
            [f"{calibration_path}/cdst_e0008_r3121_hadronSkim/", [8, 3121]],
            [f"{calibration_path}/cdst_e0010_r5095_hadronSkim/", [10, 5095]]
        ],
        "raw_cosmic": [
            [f"{calibration_path}/cdst_e0008_r3121_cosmicSkim/", [8, 3121]],
            [f"{calibration_path}/cdst_e0010_r5095_cosmicSkim/", [10, 5095]]
        ],
        "raw_beam": []
    }
    with open("input_files.json", "w") as input_files_json:
        json.dump(input_files, input_files_json)

    # Now simply run the calibration locally (on our CI/CD servers we can not test different backends).
    try:
        subprocess.check_call(['b2caf-prompt-run', 'Local', 'config.json', 'input_files.json', '--heartbeat', '20'])
    except subprocess.CalledProcessError as e:
        # FIXME: this is a temporary workaround for debugging the failures on the buildbot...
        log_name = basf2.find_file('calibration_results/KLMChannelStatus/0/collector_output/raw/0/stdout', '', True)
        if log_name:
            with open(log_name) as log_file:
                print(log_file.read())
        basf2.B2FATAL(
            f'The test failed because an exception was raised ({e}). Please re-run the build if this failure happened on bamboo.')
