#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from DataSetUtil import DSParam


def main():

    dir = '/belle2/user/belle/'
    from AmgaClient import AmgaClient

    aclient = AmgaClient()
    unames = aclient.getSubdirectories(dir, relative=True)
    if unames:
        unames.sort()
        for uname in unames:
            udir = dir + uname + '/'
            projects = aclient.getSubdirectories(udir, relative=True)
            if projects:
                projects.sort()
                for proj in projects:
                    ds = udir + proj
                    print ds
            else:
                print 'no dataset for user ' + uname
    else:
        print 'no  dataset for any user '
        DIRAC.exit(1)


if __name__ == '__main__':
    main()

