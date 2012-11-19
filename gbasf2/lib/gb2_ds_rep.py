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
    from DataSetOp import ds_rep, ds_list_replica, ds_current_user
    cuser = ds_current_user()
    if not cuser or cuser != User:
        print 'You are not allowed to replicate dataset belonging to ', User
        sys.exit(-1)

    from AmgaClient import AmgaClient
    aclient = AmgaClient()
    entries = aclient.getAttributesValues(dspath, ['lfn'])
    if entries is False:
        print 'dataset %s does not exisit' % dspath
        sys.exit(-1)

    ds_rep(entries, dstse)
    ds_list_replica(entries, 1, 0)


if __name__ == '__main__':
    main()

