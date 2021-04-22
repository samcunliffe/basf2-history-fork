#!/usr/bin/env python3
# -- analysis/tests/tagging_examples.py

import b2test_utils as b2tu
import b2test_utils_analysis as b2tua

#: extend the examples test class to actually run over the tagging examples:


class ExamplesTest(b2tua.ExamplesTest):
    def test_tagging_examples(self):
        """
        Test supported tagging examples.
        """

        self._test_examples_dir('analysis/examples/tagging/')


if __name__ == '__main__':
    with b2tu.clean_working_directory():
        b2tua.unittest.main()
