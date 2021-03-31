#!/usr/bin/env python3
# -- analysis/tests/fitting_examples.py

import b2test_utils as b2tu
import b2test_utils_analysis as b2tua

#: extend the examples test class to actually run over the fitting examples:


class ExamplesTest(b2tua.ExamplesTest):
    def test_fitting_examples(self):
        """
        Test supported fitting examples.
        """

        self._test_examples_dir('analysis/examples/fitting/')


if __name__ == '__main__':
    with b2tu.clean_working_directory():
        b2tua.unittest.main()
