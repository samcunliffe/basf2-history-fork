#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import math
from basf2 import *

import ROOT
from ROOT import Belle2

# Import xml parser
import xml.etree.ElementTree as xml

paramnames = [
    'du',
    'dv',
    'dw',
    'alpha',
    'beta',
    'gamma',
    ]


def write_alignment(aligntree, file_path):
    file = open(file_path, 'w')
    aligntree.write(file, encoding='UTF-8', xml_declaration=True)
    file.close()


def get_alignment_from_txt(txt_path):
    alignment = dict()
    file = open(txt_path, 'r')
    line = file.readline()
    while True:
        line = file.readline()
        if not line:
            break
        data = line.split()
        paramid = data[0]
        if int(paramid) < 0:
            continue
        sensor = int(int(paramid) / 10.)
        param = int(paramid) - sensor * 10
        value = data[1]
    # presigma = data[2]
        vxdid = str(Belle2.VxdID(sensor).getLayerNumber()) + '.' \
            + str(Belle2.VxdID(sensor).getLadderNumber()) + '.' \
            + str(Belle2.VxdID(sensor).getSensorNumber())
        if not vxdid in alignment.keys():
            alignment[vxdid] = dict()
        alignment[vxdid][paramnames[int(param) - 1]] = value
    file.close()
    return alignment


def sum_xmltxt_alignment(alignment_xml_path, alignment_txt_path):
    aligntree = xml.parse(alignment_xml_path)
    txtdata = get_alignment_from_txt(alignment_txt_path)
    for comp in aligntree.iter('Align'):
        if not str(comp.attrib['component']) in txtdata:
            continue
        for paramname in paramnames:
            if not paramname in txtdata[comp.attrib['component']]:
                continue
            param = comp.find(paramname)
            # Text file is always in cm (until Geant4 changes units)
            factor = 1.
            if param.attrib['unit'] == 'cm':
                factor = 1.
            elif param.attrib['unit'] == 'mm':
                factor = 1. / 10.
            elif param.attrib['unit'] == 'rad':
                factor = 1.
            elif param.attrib['unit'] == 'deg':
                factor = 2. * 3.1415926 / 360.
            else:
                raise Exception('Unsupported unit in xml file '
                                + alignment_xml_path)
            param.text = str(1. / factor * (float(param.text) * factor
                             + float(txtdata[str(comp.attrib['component'
                             ])][paramname])))
    return aligntree


# Limited use due to typically different units (deg, mm)
# in default alignment xmls than in alignment (rad, cm)


def sum_xml_alignment(alignment_xml_path1, alignment_xml_path2):
    aligntree1 = xml.parse(alignment_xml_path1)
    aligntree2 = xml.parse(alignment_xml_path2)
    for comp1 in aligntree1.iter('Align'):
        comp2 = aligntree2.find(str('Align[@component="{vxdid}"]'
                                ).format(vxdid=comp1.attrib['component']))
        if comp2 is None:
            continue
        for paramname in paramnames:
            param1 = comp1.find(paramname)
            param2 = comp2.find(paramname)
            if param1.attrib['unit'] != param2.attrib['unit']:
                raise Exception('Incompatible units in alignment parameters')

            if param2 is not None:
                param1.text = str(float(param1.text) + float(param2.text))
    return aligntree1


class SetAlignment(Module):

    """A utility module to manipulate alignment in VXD testbeam
     """

    def __init__(self, main_xml_path, alignment_xml_path):
        """Initialize the module"""

        super(SetAlignment, self).__init__()
      # # The main geometry xml
        self.main_xml_path = main_xml_path
      # # The alignment xml ... <Alignment>...</Alignment>
        self.alignment_xml_path = alignment_xml_path
      # # Original content of main xml file
        self.original_xml_data = ''

    def initialize(self):
        """ Replace the alignment in main xml """

        maintree = xml.parse(self.main_xml_path)
        aligntree = xml.parse(self.alignment_xml_path)

        mainalignment = \
            maintree.getroot().find('./DetectorComponent/Content/Alignment')
      # remove content of alignment node
        mainalignment.clear()
      # add all align components from alignment xml to main geometry xml
        mainalignment.extend(aligntree.findall('Align'))
      # store a backup of the whole main xml file
        file = open(self.main_xml_path, 'r')
        self.original_xml_data = file.read()
      # write main xml updated with alignment to file
        file = open(self.main_xml_path, 'w')
        maintree.write(file, encoding='UTF-8', xml_declaration=True)
        file.close()

    def beginRun(self):
        """ Do nothing. """

    def event(self):
        """Do nothing."""

    def terminate(self):
        """ Restore previous content of main xml """

        file = open(self.main_xml_path, 'w')
        file.write(self.original_xml_data)
        file.close()


