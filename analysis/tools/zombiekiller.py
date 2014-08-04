#!/usr/bin/env python
# Removes brokek or empty root files in the current directory
# Thomas Keck 2014

import ROOT
import os

if __name__ == '__main__':
    for filename in [f for f in os.listdir('.') if os.path.isfile(f) and f.endswith('.root')]:
        rootfile = ROOT.TFile(filename)
        n = len(rootfile.GetListOfKeys())
        z = rootfile.IsZombie()
        rootfile.Close()
        if n == 0 or z:
            print 'Remove broken or empty ROOT file {f} (y/n)?'.format(f=filename)
            if raw_input() == 'y':
                os.remove(filename)
