# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>SVDValidationTTreeRecoDigit.root</input>
    <output>SVDRecoDigitPerformance.root</output>
    <description>
    Validation plots related to RecoDigit performance.
    </description>
    <contact>
    SVD Software Group, svd-software@belle2.org
    </contact>
</header>
"""

from plotUtils import *

# open the files with simulated and reconstructed events data
input = R.TFile.Open("../SVDValidationTTreeRecoDigit.root")

tree = input.Get("tree")

histsRDP = R.TFile.Open("SVDRecoDigitPerformance.root", "recreate")

ploter(
    name='timeResolution_sideU',
    title='Resolution of the reconstructed time of a single strip on side U',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='RecoDigit time resolution (ns)',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='recodigit_time - truehit_time',
    cut=cut_U,
    descr='(reconstructed time of the RecoDigit) - (time of the Truehit) for all RecoDigit related to TrueHits',
    check='peak around 0',
    isShifter=True)

ploter(
    name='timeResolution_sideV',
    title='Resolution of the reconstructed time of a single strip on side V',
    nbins=200,
    xmin=-100,
    xmax=100,
    x_label='RecoDigit time resolution (ns)',
    y_label='counts',
    granules=granulesD,
    tree=tree,
    expr='recodigit_time - truehit_time',
    cut=cut_V,
    descr='(reconstructed time of the RecoDigit) - (time of the Truehit) for all RecoDigit related to TrueHits',
    check='peak around 0',
    isShifter=True)
