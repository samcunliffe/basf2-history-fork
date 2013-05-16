#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
from basf2 import *

# The Belle II logo graphics
logo = \
    """                                                 
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
  eeee             eeee                eeee      
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

logolist = logo.splitlines()

CSI = "\x1B["
reset = CSI + 'm'
for ii in range(len(logolist)):
    print CSI + '93;44m' + (logolist[ii])[:-1] + CSI + '0m'

print ''
print basf2label.center(48)
print basf2copyright.center(48)
print ('Version ' + basf2version).center(48)
print ''
print '-------------------------------------------------'
print 'BELLE2_RELEASE:      ', os.environ.get('BELLE2_RELEASE', '')
print 'BELLE2_RELEASE_DIR:  ', envarReleaseDir
print 'BELLE2_LOCAL_DIR:    ', envarLocalDir
print 'BELLE2_SUBDIR:       ', envarSubDir
print 'Machine architecture:', os.environ.get('BELLE2_ARCH', '')
print 'Kernel version:      ', os.uname()[2]
print 'Operating system:    ', os.uname()[3]
print ''
print 'basf2 modules directory:', ':'.join(basf2moddir)
print '-------------------------------------------------'
