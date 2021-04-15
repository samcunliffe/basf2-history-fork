#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import argparse
import basf2 as b2

parser = argparse.ArgumentParser()
# TODO capire come conviene usare questi argomenti per chiamare più volte event()
parser.add_argument("files", nargs="+", help="Input root files with histograms")
parser.add_argument("-e", "--exp", type=int, default=1, help="Experiment number")
parser.add_argument("-r", "--run", type=int, default=1, help="Run number")
parser.add_argument("-n", "--nevt", type=int, default=1, help="Events per file")
args = parser.parse_args()

main = b2.create_path()
# Input histograms from file(s)
main.add_module(
  'DQMHistAnalysisInputRootFile', SelectHistograms=['SVDDose*'],
  FileList=args.files, RunList=[args.run] * len(args.files),
  EventList=[args.nevt] * len(args.files), Experiment=args.exp,
  EventInterval=1, NullHistogramMode=False, AutoCanvas=False)
# Analysis module
main.add_module('DQMHistAnalysisSVDDose')
# Output canvases to root file
main.add_module(
  'DQMHistAnalysisOutputFile', SaveHistos=False, SaveCanvases=True,
  HistoFile=f"analysis_E{args.exp:04d}R{args.run:05d}.root")
# Output monitoring object to root file
main.add_module(
  'DQMHistAnalysisOutputMonObj',
  Filename=f"mirabelle_E{args.exp:04d}R{args.run:05d}.root")

# Process all events
b2.process(main)
