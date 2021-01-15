#!/usr/bin/env python3

# Thomas Keck 2016

# Create a simple data sample with some variables

import basf2 as b2
import modularAnalysis as ma
import vertex as vx
import b2biiConversion

variables = ['p', 'pt', 'pz', 'phi',
             'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)', 'daughter(0, phi)',
             'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)', 'daughter(1, phi)',
             'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)', 'daughter(2, phi)',
             'chiProb', 'dr', 'dz', 'dphi',
             'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, dz)', 'daughter(1, dz)',
             'daughter(0, dphi)', 'daughter(1, dphi)',
             'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)', 'daughter(2, M)',
             'daughter(0, atcPIDBelle(3,2))', 'daughter(1, atcPIDBelle(3,2))',
             'daughterAngle(0, 1)', 'daughterAngle(0, 2)', 'daughterAngle(1, 2)',
             'daughter(2, daughter(0, E))', 'daughter(2, daughter(1, E))',
             'daughter(2, daughter(0, clusterLAT))', 'daughter(2, daughter(1, clusterLAT))',
             'daughter(2, daughter(0, clusterHighestE))', 'daughter(2, daughter(1, clusterHighestE))',
             'daughter(2, daughter(0, clusterNHits))', 'daughter(2, daughter(1, clusterNHits))',
             'daughter(2, daughter(0, clusterE9E25))', 'daughter(2, daughter(1, clusterE9E25))',
             'daughter(2, daughter(0, minC2TDist))', 'daughter(2, daughter(1, minC2TDist))',
             'daughterInvariantMass(0, 1)', 'daughterInvariantMass(0, 2)', 'daughterInvariantMass(1, 2)']
spectators = ['isSignal', 'M', 'mcErrors']


def reconstruction_path(inputfiles):
    path = b2.create_path()
    b2biiConversion.convertBelleMdstToBelleIIMdst(None, applySkim=True, path=path)
    ma.setAnalysisConfigParams({'mcMatchingVersion': 'Belle'}, path)
    ma.fillParticleLists([('K-', 'atcPIDBelle(3,2) > 0.2 and dr < 2 and abs(dz) < 4'),
                          ('pi+', 'atcPIDBelle(3,2) < 0.2 and dr < 2 and abs(dz) < 4')], path=path)
    vx.kFit('pi0:mdst', 0.1, path=path, fit_type='massvertex')
    ma.reconstructDecay('D0 -> K- pi+ pi0:mdst', '1.7 < M < 2.0', path=path)
    vx.kFit('D0', 0.1, path=path)
    ma.applyCuts('D0', '1.7 < M < 2.0', path=path)
    ma.matchMCTruth('D0', path=path)
    return path


if __name__ == "__main__":
    from basf2 import conditions
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    path = reconstruction_path([])
    ma.variablesToNtuple('D0', variables + spectators, filename='validation.root', treename='tree', path=path)
    b2.process(path)
