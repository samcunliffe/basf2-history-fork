#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import basf2_version

import os
import sys

# The Belle II logo graphics
logo = """\

                      eeeeeeee
                eeeeeeeeeeeeeeeeeeee
          eeeeeeee        eeeeeeeeeeeeeeee
        eeeeee           eeee         eeeeee
       eeee              eeee           eeee
        eeee            eeee           eeee
         eeeee          eeee          eeee
            eeee       eeeeeeeeeeeeeeeee
                       eeeeeeeeeeeee

                      eeeeeeeeeeeeee
                      eeeeeeeeeeeeeeeeeee
       eeee          eeee            eeeeee
     eeee            eeee             eeeeee
   eeee             eeee                eeeee
  eeee              eeee                eeeee
    eeee           eeee             eeeeeee
      eeeeee       eeeeeeeeeeeeeeeeeeeee
         eeeeeeeeeeeeeeeeeeeeeeeeee
             eeeeeeeeeeeeeeeee

   BBBBBBB             ll ll          2222222
   BB    BB    eeee    ll ll   eeee    22 22
   BB    BB   ee   ee  ll ll  ee   ee  22 22
   BBBBBBB   eeeeeeee  ll ll eeeeeeee  22 22
   BB    BB  ee        ll ll ee        22 22
   BB     BB  ee   ee  ll ll  ee   ee  22 22
   BBBBBBBB    eeeee   ll ll   eeeee  2222222

"""

CSI = "\x1B["
reset = CSI + 'm'
for line in logo.splitlines():
    print(CSI + '93;44m' + line.ljust(48) + CSI + '0m')

print('')
print(basf2label.center(48))
print(basf2copyright.center(48))
print(('Version ' + basf2_version.version).center(48))
print('')
print('-' * 48)
for var in ["RELEASE", "RELEASE_DIR", "LOCAL_DIR", "SUBDIR", "EXTERNALS_VERSION", "ARCH"]:
    name = "BELLE2_" + var
    print((name + ":").ljust(25), os.environ.get(name, ''))

print('Default global tags:'.ljust(25), get_default_global_tags())
print('Kernel version:'.ljust(25), os.uname()[2])
python_version = sys.version_info[:3]
print('Python version:'.ljust(25), '.'.join(str(ver) for ver in python_version))
try:
    from ROOT import gROOT
    gROOT.SetBatch()
    rootver = gROOT.GetVersion()
except:
    rootver = 'PyROOT broken, cannot get version!'
print('ROOT version:'.ljust(25), rootver)
print('')
print('basf2 module directories:'.ljust(25))
for dirname in fw.list_module_search_paths():
    print(' ', dirname)

print('-' * 48)
