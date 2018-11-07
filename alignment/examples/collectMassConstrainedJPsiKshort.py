#!/usr/bin/env python
# -*- coding: utf-8 -*-

import basf2
from ROOT import Belle2

import simulation as sim
import reconstruction as reco
import modularAnalysis as ana
import beamparameters as beam
import generators as gen

import ROOT

main = basf2.create_path()

basf2.reset_database()
basf2.use_local_database('centraldb/dbcache.txt')
# basf2.use_local_database('localdb/database.txt')

# main.add_module("RootInput")
main.add_module("EventInfoSetter")
main.add_module("Gearbox")
main.add_module("Geometry")
main.add_module("HistoManager", histoFileName="CollectorOutput.root")
beam.add_beamparameters(main, 'Y4S', None, vertex=[0., 0., 0.])

gen.add_evtgen_generator(main, finalstate='signal', signaldecfile='/home/tadeas/belle2/clean_master/decfiles/dec/1111440100.dec')

sim.add_simulation(main)
reco.add_reconstruction(main, pruneTracks=False, add_muid_hits=True)
main.add_module('DAFRecoFitter')
# Pre-fit with beam+vertex constraint decays for alignment
ana.fillParticleList('mu+:Jpsi', 'muonID > 0.1', writeOut=True, path=main)
ana.fillParticleList('pi+:K_S0', 'pionID > 0.1', writeOut=True, path=main)
ana.reconstructDecay('J/psi:mumu -> mu-:Jpsi mu+:Jpsi', 'InvM > 2.9969 and InvM <  3.1969', writeOut=True, path=main)
ana.reconstructDecay('K_S0:pipi -> pi-:K_S0 pi+:K_S0', 'InvM > 0.44761 and InvM <  0.54761', writeOut=True, path=main)

ana.vertexRaveDaughtersUpdate('J/psi:mumu', 0.0, path=main)
ana.vertexRaveDaughtersUpdate('K_S0:pipi', 0.0, path=main)

# ana.fitVertex(list_name='Z0:mumu', conf_level=0.0, fitter='kfitter', fit_type='fourC', daughtersUpdate=True, path=main)
# ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.0, path=main)#, constraint='ipprofile')
# ana.vertexRaveDaughtersUpdate('K_S0:pipi', 0.0, path=main)

ana.printVariableValues('J/psi:mumu', ['InvM'], path=main)
ana.printVariableValues('K_S0:pipi', ['InvM'], path=main)

m1 = main.add_module('VariablesToEventBasedTree', fileName='Jpsi.root',
                     particleList='J/psi:mumu',
                     variables=['InvM', 'Ecms',
                                'daughter(0, p)', 'daughter(1, p)'],
                     event_variables=['nTracks', 'expNum', 'runNum', 'evtNum'])
main.add_module('VariablesToEventBasedTree', fileName='K_S0.root',
                particleList='K_S0:pipi',
                variables=['InvM', 'Ecms',
                           'daughter(0, p)', 'daughter(1, p)'],
                event_variables=['nTracks', 'expNum', 'runNum', 'evtNum'])
main.add_module(
    'MillepedeCollector',
    components=[],
    twoBodyDecays=['K_S0:pipi', 'J/psi:mumu'], tracks=[], primaryVertices=[],
    calibrateVertex=True,
    useGblTree=False)

main.add_module('RootOutput')
main.add_module("Progress")

basf2.print_path(main)
basf2.process(main)
print(basf2.statistics)
