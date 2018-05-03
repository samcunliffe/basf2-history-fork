#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from ROOT import TVector3

import simulation
import reconstruction

set_random_seed(42)


class CheckRelationBremClusterTestModule(Module):

    # Module which checks if a generated bremsstrahlung cluster is assigned correctly
    # to the primary ECL cluster generated by an electron

    def event(self):

        # load the one track from the data store and check if the relation to the brem cluster
        # can been set correctly
        clusters = Belle2.PyStoreArray("ECLClusters")

        bremCluster = None

        for cluster in clusters:
            if cluster.isTrack() and cluster.getHypothesisId() == 5:
                # this is the primary of the electron

                # is there a relation to our secondary cluster ?
                bremCluster = cluster.getRelated("ECLClusters")

        assert(bremCluster)


class SearchForHits(Module):

    # Module used to define the position and direction of the 'virtual' bremsstrahlung photon
    # generated by the particle gun
    # Not used at the moment (only for fit location)

    def event(self):

        reco_tracks = Belle2.PyStoreArray("RecoTracks")

        for recoTrack in reco_tracks:
            # hit = recoTrack.getMeasuredStateOnPlaneFromFirstHit()
            print("!!!!!!!!!!!!!!!!!!!!!!!!!Position!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
            hit = recoTrack.getMeasuredStateOnPlaneClosestTo(TVector3(10, 5, -2))
            hit_pos = hit.getPos()
            print(hit_pos.X())
            print(hit_pos.Y())
            print(hit_pos.Z())
            print(hit.getMom().Phi())
            print(hit.getMom().Theta())

# to run the framework the used modules need to be registered


main = create_path()


# generates electron with given direction
main.add_module('ParticleGun',
                pdgCodes=[11],
                nTracks=1,
                momentumGeneration='fixed',
                momentumParams=0.5,
                thetaGeneration='fixed',
                thetaParams=95,
                phiGeneration='fixed',
                phiParams=30)


# generates a photon which characteristics are chosen that it would be a bremsstrahlung photon radiated by the electron
main.add_module('ParticleGun',
                pdgCodes=[22],
                nTracks=1,
                momentumGeneration='fixed',
                momentumParams=0.1,
                thetaGeneration='fixed',
                thetaParams=1.6614126908216453 * 180 / 3.1415,
                phiGeneration='fixed',
                phiParams=0.6210485691762964 * 180 / 3.1415,
                xVertexParams=[9.27695426703659],
                yVertexParams=[5.949838410158973],
                zVertexParams=[-0.9875516764256207],
                )


# Create Event information
main.add_module('EventInfoSetter')  # ,evtNumList=[1], 'runList': [1]})
simulation.add_simulation(main)

reconstruction.add_reconstruction(main, pruneTracks=False)

main.add_module('ECLTrackBremFinder')

main.add_module(CheckRelationBremClusterTestModule())

# Process events
process(main)
