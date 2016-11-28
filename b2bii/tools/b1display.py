#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2
from b2biiConversion import convertBelleMdstToBelleIIMdst, setupB2BIIDatabase, parse_process_url

import sys
try:
    exp = int(sys.argv[1])
    run = int(sys.argv[2])
    typ = str(sys.argv[3])
except:
    raise RuntimeError("Usage: b1display exp run typ got " + str(sys.argv))

on_kek_cc = parse_process_url(
    'http://bweb3.cc.kek.jp/montecarlo.php?'
    'ex={exp}&rs={run}&re={run}&ty={typ}&dt=on_resonance&bl=caseB&st=1'.format(exp=exp, run=run, typ=typ))
if len(on_kek_cc) != 1:
    raise RuntimeError("Webinterface returned not exactly one file, I can't handle this")

import subprocess
subprocess.call(['scp', 'cw01.cc.kek.jp:' + on_kek_cc[0], 'b1display_temp.mdst'])

path = create_path()
setupB2BIIDatabase(True)
convertBelleMdstToBelleIIMdst('b1display_temp.mdst', applyHadronBJSkim=True, path=path)
path.add_module('RootOutput', outputFileName='b1display_temp.root')
process(path)

subprocess.call(['b2display', 'b1display_temp.root'])
