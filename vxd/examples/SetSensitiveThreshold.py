#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

# Import xml parser
import xml.etree.ElementTree as xml

# Load the required libraries
import ROOT
from ROOT import Belle2

logging.log_level = LogLevel.WARNING


class SetVXDSensitiveThreshold(Module):

    """A utility module to manipulate the threshold on deposited energy
     in PXD and SVD SensitiveDetector.
     """

    def __init__(self, threshold):
        """Initialize the module"""

        super(SetVXDSensitiveThreshold, self).__init__()
        ## The threshold value to be set.
        self.threshold = threshold
        ## The current pxd threshold value.
        self.old_pxd_threshold = 0.0
        ## The current svd threshold value.
        self.old_svd_threshold = 0.0
        ## Filesystem path to the .xml file
        self.path = 'data/{comp}/{COMP}.xml'
        ## XML path to the threshold setting.
        self.xmlpath = 'Content/SensitiveThreshold'

    def set_threshold(self, component, value):
        """ Set the threshold value in PXD or SVD file."""

        filename = self.path.format(comp=component.lower(),
                                    COMP=component.upper())
        tree = xml.parse(filename)
        for node in tree.getroot().findall(self.xmlpath):
            if component.lower() == 'pxd':
                self.old_pxd_threshold = float(node.text)
            else:
                self.old_svd_threshold = float(node.text)
            node.text = '{val}'.format(val=value)

        file = open(filename, 'w')
        tree.write(file, encoding='UTF-8', xml_declaration=True)
        file.close()

    def initialize(self):
        """ Set the required threshold value """

        self.set_threshold('PXD', self.threshold)
        self.set_threshold('SVD', self.threshold)

    def beginRun(self):
        """ Do nothing. """

    def event(self):
        """Do nothing."""

    def terminate(self):
        """ Set the previuos threshold value """

        self.set_threshold('PXD', self.old_pxd_threshold)
        self.set_threshold('SVD', self.old_svd_threshold)


# Particle gun module
particlegun = register_module('ParticleGun')
# Create Event information
evtmetagen = register_module('EvtMetaGen')
# Show progress of processing
progress = register_module('Progress')
# Manipulate the energy threshold in VXD SensitiveDetectors.
set_thr = SetVXDSensitiveThreshold(-0.01)
# Load parameters
gearbox = register_module('Gearbox')
# Create geometry
geometry = register_module('Geometry')
# Run simulation
simulation = register_module('FullSim')
# PXD digitization module
pxddigi = register_module('PXDDigitizer')
# PXD clustering module
pxdclust = register_module('PXDClusterizer')
# SVD digitization module
svddigi = register_module('SVDDigitizer')
# SVD clustering module
svdclust = register_module('SVDClusterizer')
# Simpleoutput
output = register_module('RootOutput')

# Specify number of events to generate
evtmetagen.param({'evtNumList': [10], 'runList': [1]})

# Set parameters for particlegun
particlegun.param({  # Generate 5 tracks on average
                     # the number of tracks is a Poisson variate
                     # Generate pi+, pi-, e+ and e-
                     # with a normal distributed transversal momentum
                     # with a center of 5 GeV and a width of 1 GeV
                     # a normal distributed phi angle,
                     # center of 180 degree and a width of 30 degree
                     # Generate theta angles uniform in cos theta
                     # between 17 and 150 degree
                     # normal distributed vertex generation
                     # around the origin with a sigma of 2cm in the xy plane
                     # and no deviation in z
                     # all tracks sharing the same vertex per event
    'nTracks': 1,
    'varyNTracks': True,
    'pdgCodes': [211, -211, 11, -11],
    'momentumGeneration': 'normalPt',
    'momentumParams': [5, 1],
    'phiGeneration': 'normal',
    'phiParams': [180, 30],
    'thetaGeneration': 'uniformCosinus',
    'thetaParams': [17, 150],
    'vertexGeneration': 'normal',
    'xVertexParams': [0, 2],
    'yVertexParams': [0, 2],
    'zVertexParams': [0, 0],
    'independentVertices': False,
    })

# Select subdetectors to be built
geometry.param('Components', ['MagneticField', 'PXD', 'SVD'])

# Set the desired SensitiveDetector threshold for the VXD.
# -1 eV means no threshold.
# set_thr.param('threshold',-1.0)

# create processing path
main = create_path()
main.add_module(evtmetagen)
main.add_module(progress)
main.add_module(particlegun)
main.add_module(set_thr)  # MUST precede gearbox!
main.add_module(gearbox)
main.add_module(geometry)
main.add_module(simulation)
main.add_module(pxddigi)
main.add_module(pxdclust)
main.add_module(svddigi)
main.add_module(svdclust)
main.add_module(output)

# generate events
process(main)

# show call statistics
print statistics
