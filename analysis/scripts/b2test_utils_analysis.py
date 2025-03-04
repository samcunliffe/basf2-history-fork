#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import sys
import subprocess
import unittest
import glob
from basf2 import find_file


class ExamplesTest(unittest.TestCase):
    """Test to run all example scripts."""

    @unittest.skipIf(not os.getenv('BELLE2_EXAMPLES_DATA_DIR'),
                     "$BELLE2_EXAMPLES_DATA_DIR not found.")
    @unittest.skipIf(not os.getenv('BELLE2_VALIDATION_DATA_DIR'),
                     "$BELLE2_VALIDATION_DATA_DIR not found.")
    def _test_examples_dir(self, path_to_glob, broken=None):
        """
        Internal function to test a directory full of example scripts with an optional list of broken scripts to be skipped.

        Parameters:
            path_to_glob (str): the path to search for scripts
            broken (list(str)): (optional) scripts that are known to be broken and can be skipped
        """
        if broken is None:
            broken = []
        all_egs = sorted(glob.glob(find_file(path_to_glob) + "/*.py"))
        for eg in all_egs:
            filename = os.path.basename(eg)
            if filename not in broken:
                with self.subTest(msg=filename):
                    result = subprocess.run(['basf2', '-n1', eg], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                    if result.returncode != 0:
                        # failure running example so let's print the output
                        # on stderr so it's not split from output of unittest
                        # done like this since we don't want to decode/encode utf8
                        sys.stdout.buffer.write(result.stdout)
                    self.assertEqual(result.returncode, 0)
