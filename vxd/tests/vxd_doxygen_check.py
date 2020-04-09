#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Perform code doxygen checks for every commit to the vxd package.
"""

import re
from b2test_utils import check_error_free

if __name__ == "__main__":
    ignoreme = ''
    check_error_free("b2code-doxygen-warnings", "doxygen", "vxd",
                     lambda x: re.findall(ignoreme, x) or x is "'")
