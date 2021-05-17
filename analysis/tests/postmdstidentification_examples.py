#!/usr/bin/env python3
# -- analysis/tests/postmdstidentification_examples.py

import b2test_utils as b2tu
import b2test_utils_analysis as b2tua

#: extend the examples test class to actually run over the post mdst identification examples:


class ExamplesTest(b2tua.ExamplesTest):
    def test_postmdstidentification_examples(self):
        """
        Test supported PostMdstIdentification examples.
        """

        self._test_examples_dir('analysis/examples/PostMdstIdentification/')


if __name__ == '__main__':
    with b2tu.clean_working_directory():
        b2tua.unittest.main()
