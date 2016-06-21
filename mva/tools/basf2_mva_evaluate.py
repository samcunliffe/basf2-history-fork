#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2_mva
from basf2_mva_evaluation import plots
import argparse
import tempfile

import numpy as np
from B2Tools import b2latex
from ROOT import Belle2

import os
import shutil
import sys


def getCommandLineOptions():
    """ Parses the command line options of the fei and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--identifier', dest='identifier', type=str, required=True, action='append', nargs='+',
                        help='DB Identifier or weightfile')
    parser.add_argument('-d', '--datafile', dest='datafile', type=str, required=True, help='Data file containing ROOT TTree')
    parser.add_argument('-t', '--treename', dest='treename', type=str, default='tree', help='Treename in data file')
    parser.add_argument('-o', '--outputfile', dest='outputfile', type=str, default='output.pdf',
                        help='Name of the outputted pdf file')
    args = parser.parse_args()
    return args


class ExpertInformation(object):
    """
    Contains information of a export extracted from the GeneralOptions
    for a convinient access
    """

    def __init__(self, identifier):
        """
        Extract information about an expert from the database using the given identifier
        """
        general_options = basf2_mva.GeneralOptions()
        weightfile = basf2_mva.Weightfile.load(identifier)
        weightfile.getOptions(general_options)
        #: Branchname used by basf2_mva_expert for the signal probability
        self.branchname = Belle2.makeROOTCompatible(identifier)
        #: Branchname used by basf2_mva_expert for the target
        self.target = Belle2.makeROOTCompatible(general_options.m_target_variable)
        #: Variables used by the expert
        self.variables = [Belle2.makeROOTCompatible(v) for v in general_options.m_variables]
        #: Signal class in case of classification
        self.signal_class = general_options.m_signal_class


if __name__ == '__main__':
    args = getCommandLineOptions()

    tempdir = tempfile.mkdtemp()
    identifiers = sum(args.identifier, [])
    rootfilename = tempdir + '/expert.root'
    basf2_mva.expert(basf2_mva.vector(*identifiers), args.datafile, args.treename, rootfilename)
    expert_informations = [ExpertInformation(identifier) for identifier in identifiers]

    probabilities = [expert_information.branchname for expert_information in expert_informations]
    truths = [expert_information.target for expert_information in expert_informations]

    # Change working directory after experts run, because they might want to access
    # a locadb in the current working directory
    old_cwd = os.getcwd()
    os.chdir(tempdir)

    o = b2latex.LatexFile()
    o += b2latex.TitlePage(title='Automatic MVA Evaluation',
                           authors=['Thomas Keck'],
                           abstract='Evaluation plots',
                           add_table_of_contents=True).finish()

    o += b2latex.Section("Plots")

    graphics = b2latex.Graphics()
    plots.Distribution(rootfilename, 'verbose_distribution_plot.png', probabilities, truths)
    graphics.add('verbose_distribution_plot.png', width=1.0)
    o += graphics.finish()

    graphics = b2latex.Graphics()
    plots.ROC(rootfilename, 'roc_plot.png', probabilities, truths)
    graphics.add('roc_plot.png', width=1.0)
    o += graphics.finish()

    graphics = b2latex.Graphics()
    plots.Diagonal(rootfilename, 'diagonal_plot.png', probabilities, truths)
    graphics.add('diagonal_plot.png', width=1.0)
    o += graphics.finish()

    o.finish()
    o.save('latex.tex', compile=True)
    os.chdir(old_cwd)
    shutil.copy(tempdir + '/latex.pdf', args.outputfile)
