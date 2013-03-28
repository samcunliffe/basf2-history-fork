#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *


def add_reconstruction(path, components=None):
    """
    This function adds the standard reconstruction modules to a path.
    """

    # tracking
    if components == None or 'PXD' in components or 'SVD' in components \
        or 'CDC' in components:

        # find MCTracks in CDC, SVD, and PXD
        mc_trackfinder = register_module('MCTrackFinder')
        if components == None or 'PXD' in components:
            mc_trackfinder.param('UsePXDHits', 1)
        if components == None or 'SVD' in components:
            mc_trackfinder.param('UseSVDHits', 1)
        if components == None or 'CDC' in components:
            mc_trackfinder.param('UseCDCHits', 1)
        path.add_module(mc_trackfinder)

        # track fitting
        trackfitter = register_module('GenFitter')
        path.add_module(trackfitter)

        # dE/dx PID
        dEdxPID = register_module('DedxPID')
        path.add_module(dEdxPID)

        # track extrapolation
        ext = register_module('Ext')
        path.add_module(ext)

    # TOP reconstruction
    if components == None or 'TOP' in components:
        top_rec = register_module('TOPReconstructor')
        path.add_module(top_rec)

    # ARICH reconstruction
    if components == None or 'ARICH' in components:
        arich_rec = register_module('ARICHReconstructor')
        path.add_module(arich_rec)

    # ECL reconstruction
    if components == None or 'ECL' in components:

        # shower reconstruction
        ecl_shower_rec = register_module('ECLReconstructor')
        path.add_module(ecl_shower_rec)

        # gamma reconstruction
        gamma_rec = register_module('ECLGammaReconstructor')
        path.add_module(gamma_rec)

        # pi0 reconstruction
        pi0_rec = register_module('ECLPi0Reconstructor')
        path.add_module(pi0_rec)

    # EKLM reconstruction
    if components == None or 'EKLM' in components:
        eklm_rec = register_module('EKLMReconstructor')
        path.add_module(eklm_rec)

        # Muon reconstruction
        eklm_muon_rec = register_module('EKLMMuonReconstructor')
        path.add_module(eklm_muon_rec)

        # K0L reconstruction
        eklm_k0l_rec = register_module('EKLMK0LReconstructor')
        path.add_module(eklm_k0l_rec)

    # charged particle PID
    if components == None or 'PXD' in components or 'SVD' in components \
        or 'CDC' in components:
        mdstPID = register_module('MdstPIDtmp')
        path.add_module(mdstPID)


def add_mdst_output(path, mc=True):
    """
    This function adds the mdst output modules to a path.
    """

    output = register_module('RootOutput')
    output.param('outputFileName', 'mdst.root')
    branches = [
        'Tracks',
        'TrackFitResults',
        'PIDLikelihoods',
        'TracksToPIDLikelihoods',
        'ECLShowers',
        'ECLGammas',
        'ECLGammasToECLShowers',
        'ECLPi0s',
        'ECLPi0sToECLGammas',
        'EKLMK0Ls',
        ]
    if mc:
        branches += ['MCParticles', 'MCParticlesToTracks']
    output.param('branchNames', branches)
    path.add_module(output)


