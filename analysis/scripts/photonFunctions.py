# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Functions that work on photon lists.

Contributor(s): Torben Ferber
                Sam Cunliffe
                Michael De Nuccio
"""

from basf2 import *
import string
import random
from analysisPath import *
from variables import variables
from modularAnalysis import *


def getRandomId(size=6, chars=string.ascii_uppercase + string.digits):
    return ''.join(random.choice(chars) for _ in range(size))


def writeClosestPhotonExtraInfo(
    photonList,
    photonSelection='True',
    roe_path=None,
    deadend_path=None,
    path=None
):
    """
    Add various variables to the first photon that are related to their angular separation and kinematics.

    @param photonList Photon list with photon candidates that will have the extra information in the end
    @param photonSelection Selection for the other photon
    @param roe_path a path for the rest of event to be executed
    @param deadend_path a path for skipping irrelevant RestOfEvent objects that may exist (if this was called twice, for instance)
    @param path modules are added to this path
    """

    if not roe_path:
        roe_path = create_path()

    if not deadend_path:
        deadend_path = create_path()

    # build rest of event
    buildRestOfEvent(photonList, path=path)

    # get random listnames (in case we run this function multiple times)
    pListPair = 'vpho:writeClosestPhotonExtraInfo' + getRandomId()
    pList0 = 'gamma:writeClosestPhotonExtraInfo' + getRandomId()
    pList1 = 'gamma:writeClosestPhotonExtraInfo' + getRandomId()

    signalSideParticleFilter(photonList, '', roe_path, deadend_path)

    fillSignalSideParticleList(pList0, '^' + photonList, path=roe_path)

    fillParticleList(pList1, 'isInRestOfEvent == 1 and ' + photonSelection, path=roe_path)

    reconstructDecay(pListPair + ' -> ' + pList0 + ' ' + pList1, '', path=roe_path)

    # only keep the one with the smallest opening angle
    rankByLowest(pListPair, 'daughterAngleInBetween(0, 1)', 1, path=roe_path)

    # add new variables to the signal side particle
    variableToSignalSideExtraInfo(pListPair, {'useLabFrame(daughterAngleInBetween(0, 1))': 'openingAngle'}, path=roe_path)
    variableToSignalSideExtraInfo(pListPair, {'useLabFrame(daughterDiffOf(0, 1, theta))': 'deltaTheta'}, path=roe_path)
    variableToSignalSideExtraInfo(pListPair, {'useLabFrame(daughterDiffOfPhi(0, 1))': 'deltaPhi'}, path=roe_path)

    path.for_each('RestOfEvent', 'RestOfEvents', roe_path)
