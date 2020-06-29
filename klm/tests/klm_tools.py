#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Test the klm tools.
"""

import os
import glob
from b2test_utils import skip_test, clean_working_directory

if __name__ == "__main__":

    # Test b2klm-create-dqm (it also tests b2klm-mask-dqm and b2klm-execute-masking)
    if 'BELLE2_VALIDATION_DATA_DIR' not in os.environ:
        B2INFO('Skipping the b2klm-create-dqm test.')
    else:
        for input_file in glob.glob(os.environ['BELLE2_VALIDATION_DATA_DIR'] + '/rawdata/*HLT?.*.root'):
            with clean_working_directory() as test_klm_tools:
                command = f'b2klm-create-dqm -i {input_file} -n 100 --prepend_gt validation_2020-05-02 online_proc11'
                assert(0 == os.system(command))

    # Test b2klm-numberToIndex
    command = 'b2klm-numberToIndex 20 50020'
    assert(0 == os.system(command))

    # Test b2klm-indexToNumber
    command = 'b2klm-indexToNumber 11489 21997'
    assert(0 == os.system(command))
