#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from DataSetUtil import DSParam


def main():

    dsparam = DSParam(sys.argv[0])
    dsparam.register_param()
    from DIRAC.Core.Base import Script
    Script.parseCommandLine(ignoreErrors=True)
    from DataSetOp import ds_get_se_list
    dst_se = ds_get_se_list()
    for se in dst_se:
        print se


if __name__ == '__main__':
    main()

