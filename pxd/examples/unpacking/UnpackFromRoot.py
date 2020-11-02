#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import shutil
import basf2 as b2

from basf2 import Module

input = b2.register_module('RootInput')

unpacker = b2.register_module('PXDUnpacker')
# unpacker.param('DoNotStore',True);
# unpacker.param('HeaderEndianSwap', False)

histoman = b2.register_module('HistoManager')
histoman.param('histoFileName', 'your_histo_file.root')

simpleoutput = b2.register_module('RootOutput')
simpleoutput.param('outputFileName', 'PXDRawHit.root')
simpleoutput.param('compressionLevel', 0)

main = b2.create_path()

main.add_module(input)
main.add_module(histoman)
main.add_module(unpacker)
main.add_module('PXDRawDQM')
main.add_module('PXDROIDQM')
main.add_module('Progress')
main.add_module(simpleoutput)

b2.process(main)
