#!/usr/bin/env python
# -*- coding: utf-8 -*-

from DataSetUtil import *
import sys


def main():
    dsparam = DSParam(sys.argv[0])
    dsparam.register_param()
    from DIRAC.Core.Base import Script
    Script.parseCommandLine(ignoreErrors=True)
    Project = dsparam.getProject()
    User = dsparam.getUser()
    dstse = dsparam.getDstSE()
    dspath = '/belle2/user/belle/' + User + '/' + Project
    from DataSetOp import ds_sync, ds_list_replica
    from AmgaClient import AmgaClient

    aclient = AmgaClient()
    entries = aclient.getAttributesValues(dspath, ['lfn'])
    if entries is False:
        print 'dataset %s does not exisit' % dspath
        sys.exit(-1)

    ds_sync(entries)
    ds_list_replica(entries, 1, 0)


if __name__ == '__main__':
    main()

