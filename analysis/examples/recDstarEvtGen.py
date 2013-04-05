#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

# ----------------------------------------------------------------------------------
# Example of reconstruction D*+ -> D0 pi+, D0 -> K- pi+ using EvtGen output file
#
# To run this script you need first to prepare a root file EvtGenOutput.root
# using for example analysis/examples/EvtgenOut.py
#
# Charge conjugate combinations are always done.
#
# Selection criteria in selectParticle and applyCuts are specified using ranges:
#
# 'M 1.8:1.9'       means 1.8 <= M <= 1.9
# 'M 1.8:'          means M >= 1.8
# 'M :1.9'          means M <= 1.9
# 'dM +-0.1'        means abs(dM) <= 0.1
# 'dM +-(0.1:0.2)'  means 0.1 <= abs(dM) <= 0.2 (e.g. sidebands in M)
#
# More than one range can be specified for a single variable:
# 'dM +-0.1 +-(0.15:0.25)' to select events in signal and sideband region
#
# Cut on variable of daughter particle:
# M.1.2 means 2nd daugher of 1st daugher (e.g. K+ from D0 -> K- K+ in D*+ decay chain)
# Note: daugher numbering starts from 1
#
# selectParticle and applyCuts can take a list of cuts:
# applyCuts('D*+',['dM.1 +-0.1 +-(0.15:0.25)','dQ +-0.001'])
# meaning: (abs(dM.1) <= 0.1 or 0.15 <= abs(dM.1) <= 0.25) and abs(dQ) <= 0.001
#
# Available selection variables are printed at the beginning of execution
#
# Selection criteria converted to the usual expressions can be printed out by
# processing one event at logLevel INFO: basf2 recDstarEvtGen.py -n 1 -l INFO
#
# ----------------------------------------------------------------------------------

# definitions below will be once in the future put into a separate file


def Input(filename):
    roinput = register_module('RootInput')
    roinput.param('inputFileName', filename)
    main.add_module(roinput)
    gearbox = register_module('Gearbox')
    main.add_module(gearbox)
    progress = register_module('Progress')
    main.add_module(progress)


def Output(filename):
    rooutput = register_module('RootOutput')
    rooutput.param('outputFileName', filename)
    main.add_module(rooutput)


def loadMCParticles():
    ploader = register_module('ParticleLoader')
    ploader.param('UseMCParticles', True)
    main.add_module(ploader)


def loadReconstructedParticles():
    ploader = register_module('ParticleLoader')
    ploader.param('UseMCParticles', False)
    main.add_module(ploader)


def selectParticle(list_name, PDGcode, criteria):
    pselect = register_module('ParticleSelector')
    pselect.param('PDG', PDGcode)
    pselect.param('ListName', list_name)
    pselect.param('Select', criteria)
    main.add_module(pselect)


def applyCuts(list_name, criteria):
    pselect = register_module('ParticleSelector')
    pselect.param('ListName', list_name)
    pselect.param('Select', criteria)
    main.add_module(pselect)


def makeParticle(
    list_name,
    PDGcode,
    list_of_lists,
    mL,
    mH,
    ):

    pmake = register_module('ParticleCombiner')
    pmake.param('PDG', PDGcode)
    pmake.param('ListName', list_name)
    pmake.param('InputListNames', list_of_lists)
    pmake.param('MassCutLow', mL)
    pmake.param('MassCutHigh', mH)
    main.add_module(pmake)


def fitVertex(list_name, confidenceLevel):
    pvfit = register_module('ParticleVertexFitter')
    pvfit.param('ListName', list_name)
    pvfit.param('ConfidenceLevel', confidenceLevel)
    main.add_module(pvfit)


def printList(list_name, full):
    prlist = register_module('ParticlePrinter')
    prlist.param('ListName', list_name)
    prlist.param('FullPrint', full)
    main.add_module(prlist)


# analysis code follows

main = create_path()

Input('EvtGenOutput.root')
loadMCParticles()

selectParticle('K-', -321, [])
selectParticle('pi+', 211, [])
makeParticle('D0', 421, ['K-', 'pi+'], 1.7, 2.0)
makeParticle('D*+', 413, ['D0', 'pi+'], 1.9, 2.1)
applyCuts('D*+', ['M.1 1.81:1.91', 'Q :0.02'])

# printList('D*+',True) # uncomment to investigate the content of the list

Output('recDstarMC.root')

process(main)
print statistics
