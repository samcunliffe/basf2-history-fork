#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from DataSetUtil import DSParam


def main():

    dsparam = DSParam(sys.argv[0])
    dsparam.register_param()
    from DIRAC.Core.Base import Script
    Script.parseCommandLine(ignoreErrors=True)
    Project = dsparam.getProject()
    User = dsparam.getUser()
    dstse = dsparam.getDstSE()
    dspath = '/belle2/user/belle/' + User + '/' + Project
    # print "ds_path is ",dspath, "dstse is ",dstse
    from DataSetOp import ds_generate

    ds_generate(dspath, dstse)


if __name__ == '__main__':
    main()
