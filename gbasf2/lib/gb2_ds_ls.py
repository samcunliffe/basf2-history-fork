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
    dspath = '/belle2/user/belle/' + User + '/' + Project
    ListReplica = dsparam.getListReplica()
    ListSite = dsparam.getListSite()
    print 'dspath is %s ,ListReplica is %d, listsite is %d' % (dspath,
            ListReplica, ListSite)
    from DataSetOp import ds_list_replica
    from AmgaClient import AmgaClient

    aclient = AmgaClient()
    entries = aclient.getAttributesValues(dspath, ['lfn'])
    if entries is False:
        print 'dataset %s does not exisit' % dspath
        sys.exit(-1)
    # print "entries are: ",entries
    ds_list_replica(entries, ListReplica, ListSite)


if __name__ == '__main__':
    main()

