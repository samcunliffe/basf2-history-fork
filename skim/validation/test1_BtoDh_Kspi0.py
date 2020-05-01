#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <input>../BtoDh_Kspi0.dst.root</input>
    <output>14120300.udst.root</output>
    <contact>niharikarout@physics.iitm.ac.in</contact>
    <interval>nightly</interval>
</header>
"""
__author__ = [
    "N Dash",
    "N. Rout"
]

import basf2 as b2
import modularAnalysis as ma
from skim.btocharm import BtoD0h_Kspi0

path = b2.Path()

fileList = ['../BtoD0h_Kspi0.dst.root']
ma.inputMdstList('default', fileList, path=path)

skim = BtoD0h_Kspi0()
skim(path)
b2.process(path)
