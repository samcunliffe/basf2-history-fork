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
    del_se = dsparam.getDstSE()
    dspath = '/belle2/user/belle/' + User + '/' + Project

    from DataSetOp import ds_del_file_replica, ds_list_replica, \
        ds_get_file_SE, ds_current_user
    cuser = ds_current_user()
    if cuser and cuser == User:
        from AmgaClient import AmgaClient
        aclient = AmgaClient()
        entries = aclient.getAttributesValues(dspath, ['lfn'])
        if entries is False:
            print 'dataset %s does not exisit' % dspath
            sys.exit(-1)
        for file in entries.keys():
            lfn = entries[file][0].replace('belle2', 'belle')
            print lfn
            src_ses = ds_get_file_SE(lfn)
            if del_se != '' and del_se in src_ses:
                myse = del_se
            else:
                myse = src_ses[0]
                if del_se != '':
                    print 'no replica at <%s>,delete replica from <%s> instead' \
                        % (del_se, myse)

            print 'deleting replica from <%s>' % myse
            ds_del_file_replica(lfn, myse)
        ds_list_replica(entries, 1, 0)
    else:
        print 'You are not allowed to decrease replicas of dataset belonging to ', \
            User
        sys.exit(-1)


if __name__ == '__main__':
    main()

