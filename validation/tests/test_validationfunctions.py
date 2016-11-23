#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import os
import validationfunctions
import tempfile

call_iteration = 0


class ValidationFunctionstTest(unittest.TestCase):

    """
    Various test cases for the validation utility functions
    """

    def test_git_hash(self):
        """
        Test if the extraction of the git hash works and if not, if the
        functions returns the expected result
        """

        cwd = os.getcwd()

        with tempfile.TemporaryDirectory() as td:
            # switch to this folder
            os.chdir(str(td))
            ret = validationfunctions.get_compact_git_hash(os.getcwd())
            self.assertTrue(ret is None)

        os.chdir(cwd)

        # in the source folder, we should get a sensible output
        ret = validationfunctions.get_compact_git_hash(os.getcwd())
        self.assertTrue(ret is not None)
        print("This source code has git hash {}".format(ret))


if __name__ == "__main__":
    unittest.main()
