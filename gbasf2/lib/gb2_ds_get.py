#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from DataSetUtil import *
import sys


def main():
    dsparam = DSParam(sys.argv[0])
    dsparam.register_param()
    from DIRAC.Core.Base import Script
    Script.parseCommandLine(ignoreErrors=True)
    Project = dsparam.getProject()
    User = dsparam.getUser()
    dstdir = dsparam.getDstDir()
    dspath = '/belle2/user/belle/' + User + '/' + Project
    from DataSetOp import ds_get
    from AmgaClient import AmgaClient

    aclient = AmgaClient()
    entries = aclient.getAttributesValues(dspath, ['lfn'])
    if entries is False:
        print 'dataset %s does not exisit' % dspath
        sys.exit(-1)
    if dstdir == '':
        dstdir = '/tmp/'
    a = dspath.split('/')
    ds_dir = dstdir + '/' + '_'.join(a[-2:])
    ow = ''
    if not os.path.exists(ds_dir):
        os.mkdir(ds_dir)
    else:
        s = \
            'you have downloaded this dataset before, how would you like to verify the files?By file size(s) or by file checksum(c)?: '
        ow = raw_input(s)
        while ow != 's' and ow != 'c':
            s = \
                'please input "s" to choose file size, or "c" to choose file checksum: '
            ow = raw_input(s)

    ds_get(entries, ds_dir, ow)


if __name__ == '__main__':
    main()

