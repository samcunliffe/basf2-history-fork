#!/usr/bin/env python
# -*- coding: utf-8 -*-

from DataSetUtil import *
import sys


def main():

    dsparam = DSParam(sys.argv[0])
    dsparam.register_param()
    from DIRAC.Core.Base import Script
    Script.parseCommandLine(ignoreErrors=True)
    User = dsparam.getUser()
    Project = dsparam.getProject()
    dstse = dsparam.getDstSE()
    dspath = '/belle2/user/belle/' + User + '/' + Project
    s = \
        'This operation will remove the dataset %s completely,\nand it is not recoverable, are you sure to go ahead?(yes or no?)' \
        % dspath
    a = raw_input(s)
    while a != 'yes' and a != 'no':
        a = raw_input(s)
    if a == 'yes':
        from DataSetOp import ds_del, ds_current_user
        cuser = ds_current_user()
        if cuser and cuser == User:
            from AmgaClient import AmgaClient
            aclient = AmgaClient()
            entries = aclient.getAttributesValues(dspath, ['lfn'])
            if entries is False:
                print 'dataset %s does not exisit' % dspath
                sys.exit(-1)
            ds_del(entries, aclient)
        else:
            print 'You are not allowed to delete dataset belonging to ', User
            sys.exit(-1)
    else:
        sys.exit(0)


if __name__ == '__main__':
    main()

