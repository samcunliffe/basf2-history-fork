#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
from basf2 import *
from ROOT import Belle2


class VXDMisalignment:
    """ Class to generate VXDAlignment object in Database
    Can be used to train misalignment by storing wrong alignment
    and trying to reach nominal values (zeros)
    """

    def __init__(self, name):
        """ COnstructor """

        #: name of the object
        self.name = name
        #: alignment data = list of VxdID mask, param id, generation distribution and value
        self.alignment = []

    def getName(self):
        """ get object name """
        return self.name

    def getAlignment(self):
        """ get alignment data """
        return self.alignment

    def alignSensors(self, vxdid, param_id, distribution, value):
        """ align sensors by VxdID pattern """
        self.alignment.append(str(vxdid + ',' + str(param_id) + ',' + distribution + ',' + str(value)))

    def genSensorU(self, vxdid, value):
        """ generate u coordinate shift for VxdID mask with gaussian distribution of given width """
        self.alignSensors(vxdid, 1, 'gaus', value)

    def genSensorV(self, vxdid, value):
        """ generate v coordinate shift for VxdID mask with gaussian distribution of given width """
        self.alignSensors(vxdid, 2, 'gaus', value)

    def genSensorW(self, vxdid, value):
        """ generate w coordinate shift for VxdID mask with gaussian distribution of given width """
        self.alignSensors(vxdid, 3, 'gaus', value)

    def genSensorAlpha(self, vxdid, value):
        """ generate alpha rotation for VxdID mask with gaussian distribution of given width """
        self.alignSensors(vxdid, 4, 'gaus', value)

    def genSensorBeta(self, vxdid, value):
        """ generate beta rotation for VxdID mask with gaussian distribution of given width """
        self.alignSensors(vxdid, 5, 'gaus', value)

    def genSensorGamma(self, vxdid, value):
        """ generate gamma rotation coordinate shift for VxdID mask with gaussian distribution of given width """
        self.alignSensors(vxdid, 6, 'gaus', value)

    def setSensorU(self, vxdid, value):
        """ set u coordinate shift for VxdID """
        self.alignSensors(vxdid, 1, 'fix', value)

    def setSensorV(self, vxdid, value):
        """ set v coordinate shift for VxdID """
        self.alignSensors(vxdid, 2, 'fix', value)

    def setSensorW(self, vxdid, value):
        """ set w coordinate shift for VxdID """
        self.alignSensors(vxdid, 3, 'fix', value)

    def setSensorAlpha(self, vxdid, value):
        """ set alpha angle for VxdID mask """
        self.alignSensors(vxdid, 4, 'fix', value)

    def setSensorBeta(self, vxdid, value):
        """ set beta angle for VxdID mask  """
        self.alignSensors(vxdid, 5, 'fix', value)

    def setSensorGamma(self, vxdid, value):
        """ set gamma angle for VxdID mask """
        self.alignSensors(vxdid, 6, 'fix', value)

    def setSensorAll(self, vxdid, value):
        """ Set all shifts/rotation to given value (usually 0.) for VxdID mask """
        self.setSensorU(vxdid, value)
        self.setSensorV(vxdid, value)
        self.setSensorW(vxdid, value)
        self.setSensorAlpha(vxdid, value)
        self.setSensorBeta(vxdid, value)
        self.setSensorGamma(vxdid, value)

main = create_path()

main.add_module('EventInfoSetter')
main.add_module('Gearbox')
main.add_module('Geometry', components=['MagneticFieldConstant4LimitedRCDC', 'PXD', 'SVD'])

misalignment = VXDMisalignment('VXDAlignment')
# misalign everything

misalignment.genSensorU('0.0.0', 0.01)
misalignment.genSensorV('0.0.0', 0.01)
misalignment.genSensorW('0.0.0', 0.01)

main.add_module(
    'AlignmentGenerator',
    experimentLow=0,
    runLow=0,
    experimentHigh=-1,
    runHigh=-1,
    name=misalignment.getName(),
    data=misalignment.getAlignment())

main.add_module('Progress')

process(main)
print(statistics)
