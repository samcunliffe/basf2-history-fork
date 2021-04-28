#!/usr/bin/env python3
# -- analysis/tests/mva_examples.py

import b2test_utils as b2tu
import b2test_utils_analysis as b2tua

#: extend the examples test class to actually run over the mva examples:


class ExamplesTest(b2tua.ExamplesTest):
    def test_mva_examples(self):
        """
        Test supported mva examples.
        """
        # list of the broken examples (to be removed when they are individually fixed)
        broken_mva_egs = ['B2A711-DeepContinuumSuppression_Input.py',  # BII-4279
                          'B2A713-DeepContinuumSuppression_MVAExpert.py',  # BII-4279
                          'B2A712-DeepContinuumSuppression_MVATrain.py',  # BII-4279
                          'B2A714-DeepContinuumSuppression_MVAModel.py'  # BII-4279
                          ]

        self._test_examples_dir('analysis/examples/mva/', broken_mva_egs)


if __name__ == '__main__':
    with b2tu.clean_working_directory():
        b2tua.unittest.main()
