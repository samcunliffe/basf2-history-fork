#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from DataSetUtil import DSParam


def main():

    dsparam = DSParam(sys.argv[0])
    dsparam.register_param()
    from DIRAC.Core.Base import Script
    Script.parseCommandLine(ignoreErrors=True)
    User = dsparam.getUser()
    dir = '/belle2/user/belle/' + User + '/'
    from AmgaClient import AmgaClient

    aclient = AmgaClient()
    entries = aclient.getSubdirectories(dir, relative=True)
    if entries:
        entries.sort()
        for proj in entries:
            ds_name = dir + proj
            print ds_name
    else:
        print 'no available dataset for user ', User
        DIRAC.exit(1)


if __name__ == '__main__':
    main()
