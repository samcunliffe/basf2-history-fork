#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Purpose:
#   basf module to histogram useful values in RawKLM and BKLMHit2d data-objects in an SROOT file.
#

import basf2
import bklmDB
import math
import ctypes
import ROOT
from ROOT import Belle2, TH1F, TH2F, TCanvas, THistPainter, TPad


class EventInspectorCoarseWindowStart(basf2.Module):
    """Fill BKLM histograms of values from RawKLMs and BKLMHit2ds"""

    #: COPPER base identifier for BKLM readout
    BKLM_ID = 0x07000000
    #: COPPER base identifier for EKLM readout
    EKLM_ID = 0x08000000
    #: bit position for strip-1 [0..47]
    BKLM_STRIP_BIT = 0
    #: bit position for plane-1 [0..1]; 0 is inner-plane
    BKLM_PLANE_BIT = 6
    #: bit position for layer-1 [0..14]; 0 is innermost
    BKLM_LAYER_BIT = 7
    #: bit position for sector-1 [0..7]; 0 is on the +x axis and 2 is on the +y axis
    BKLM_SECTOR_BIT = 11
    #: bit position for detector end [0..1]; forward is 0
    BKLM_END_BIT = 14
    #: bit position for maxStrip-1 [0..47]
    BKLM_MAXSTRIP_BIT = 15
    #: bit mask for strip-1 [0..47]
    BKLM_STRIP_MASK = 0x3f
    #: bit mask for plane-1 [0..1]; 0 is inner-plane
    BKLM_PLANE_MASK = (1 << BKLM_PLANE_BIT)
    #: bit mask for layer-1 [0..15]; 0 is innermost and 14 is outermost
    BKLM_LAYER_MASK = (15 << BKLM_LAYER_BIT)
    #: bit mask for sector-1 [0..7]; 0 is on the +x axis and 2 is on the +y axis
    BKLM_SECTOR_MASK = (7 << BKLM_SECTOR_BIT)
    #: bit mask for detector end [0..1]; forward is 0
    BKLM_END_MASK = (1 << BKLM_END_BIT)
    #: bit mask for maxStrip-1 [0..47]
    BKLM_MAXSTRIP_MASK = (63 << BKLM_MAXSTRIP_BIT)
    #: bit mask for unique module identifier (end, sector, layer)
    BKLM_MODULEID_MASK = (BKLM_END_MASK | BKLM_SECTOR_MASK | BKLM_LAYER_MASK)

    def __init__(self, exp, run, histName, pdfName, verbosity):
        """Constructor

        Arguments:
            exp (str): formatted experiment number
            run (str): formatter run number
            histName (str): path name of the output histogram ROOT file
            pdfName (str): path name of the output histogram PDF file
            verbosity (int): histogram-plot verbosity (0=minimal, 1=all)
        """
        super().__init__()
        #: internal copy of experiment number
        self.exp = exp
        #: internal copy of run number
        self.run = run
        #: internal copy of the pathname of the output histogram ROOT file
        self.histName = histName
        #: internal copy of the pathname of the output histogram PDF file
        self.pdfName = pdfName
        #: internal copy of the histogram-plot verbosity
        self.verbosity = verbosity

    def initialize(self):
        """Handle job initialization: fill the mapping database, create histograms"""

        expRun = 'e{0:02d}r{1}: '.format(int(self.exp), int(self.run))

        #: readout <-> detector map (from the information retrieved from the conditions database)
        self.electIdToModuleId = bklmDB.fillDB()
        #: map for sectorFB -> data concentrator
        self.sectorFBToDC = [11, 15, 2, 6, 10, 14, 3, 7, 9, 13, 0, 4, 8, 12, 1, 5]
        #: map for data concentrator -> sectorFB
        self.dcToSectorFB = [10, 14, 2, 6, 11, 15, 3, 7, 12, 8, 4, 0, 13, 9, 5, 1]
        #: Time-calibration constants obtained from experiment 7 run 1505
        #: RPC-time calibration adjustment (ns) for rawKLMs
        self.t0Cal = 312
        #: RPC-time calibration adjustment (ns) for BKLMHit1ds
        self.t0Cal1d = 325
        #: RPC-time calibration adjustment (ns) for BKLMHit2ds
        self.t0Cal2d = 308
        #: scint-ctime calibration adjustment (ns) for rawKLMs
        self.ct0Cal = 455
        #: scint-ctime calibration adjustment (ns) for BKLMHit1ds
        self.ct0Cal1d = 533
        #: scint-ctime calibration adjustment (ns) for BKLMHit2ds
        self.ct0Cal2d = 520
        #: per-layer variations in RPC z- and phi-time calibration adjustment (ns) for rawKLMs
        self.t0RPC = [[[0, 0, 17, 16, 15, 13, 14, 14, 16, 17, 18, 16, 16, 18, 16],
                       [0, 0, 0, -2, -1, -5, -6, -5, -5, -3, -4, -6, -8, -6, -8],
                       [0, 0, 6, 3, 5, 0, -1, -1, -2, 0, 0, -2, -3, -2, -4],
                       [0, 0, -4, -3, -6, -8, -8, -9, -9, -7, -7, -10, -10, -10, -12],
                       [0, 0, 6, 8, 4, 4, 3, 8, 4, 7, 7, 3, 5, 5, 4],
                       [0, 0, 18, 20, 18, 18, 16, 21, 17, 20, 20, 19, 21, 19, 20],
                       [0, 0, 19, 19, 19, 18, 17, 18, 22, 24, 25, 22, 22, 24, 22],
                       [0, 0, 19, 19, 18, 17, 16, 18, 18, 20, 21, 19, 20, 20, 20],
                       [0, 0, 6, 7, 9, 5, 4, 6, 6, 9, 9, 6, 7, 8, 7],
                       [0, 0, 3, 2, 2, -4, -1, -2, -2, 1, 0, -4, 246, -3, -4],
                       [0, 0, -1, -1, -1, -5, -6, -6, -8, -5, -4, -7, -7, -7, -8],
                       [0, 0, -5, -5, -5, -12, -9, -10, -8, -6, -10, -8, -8, -11, -12],
                       [0, 0, 12, 12, 13, 8, 6, 6, 7, 9, 10, 7, 6, 8, 5],
                       [0, 0, 14, 15, 43, 12, 10, 12, 11, 15, 16, 28, 14, 15, 14],
                       [0, 0, 22, 22, 21, 19, 19, 19, 21, 23, 24, 21, 22, 22, 22],
                       [0, 0, 18, 18, 17, 16, 16, 18, 18, 20, 21, 18, 18, 20, 19]],
                      [[0, 0, 6, 5, 4, 1, 1, 2, 3, 2, 2, -1, 0, 1, -1],
                       [0, 0, -11, -12, -11, -15, -18, -18, -18, -18, -19, -22, -23, -22, -24],
                       [0, 0, -4, -7, -6, -11, -12, -12, -14, -15, -15, -18, -19, -18, -20],
                       [0, 0, -15, -15, -16, -19, -22, -21, -22, -22, -22, -25, -26, -26, -27],
                       [0, 0, -5, -3, -6, -7, -9, -9, -9, -8, -8, -13, -12, -10, -13],
                       [0, 0, 6, 7, 5, 5, 3, 9, 4, 5, 6, 3, 5, 3, 4],
                       [0, 0, 9, 10, 10, 7, 7, 7, 9, 9, 9, 6, 6, 8, 8],
                       [0, 0, 7, 8, 7, 6, 4, 5, 4, 5, 5, 4, 3, 4, 3],
                       [0, 0, -5, -3, -1, -4, -8, -7, -7, -6, -6, -6, -9, -9, -9],
                       [0, 0, -8, -8, -11, -10, -14, -15, -16, -14, -15, -20, -20, -13, -20],
                       [0, 0, -12, -12, -14, -16, -16, -15, -21, -19, -19, -23, -23, -23, -24],
                       [0, 0, -15, -15, -15, -21, -22, -22, -22, -21, -23, -25, -25, -26, -27],
                       [0, 0, 0, 0, 2, -4, -5, -5, -4, -2, -1, -5, -5, -3, -7],
                       [0, 0, 3, 3, 32, 1, 0, -1, -3, 2, 1, 13, -1, 0, -2],
                       [0, 0, 11, 11, 10, 9, 6, 7, 6, 8, 8, 5, 6, 7, 6],
                       [0, 0, 7, 8, 7, 5, 3, 5, 7, 5, 5, 2, 7, 4, 3]]]

        #: per-layer variations in scint-ctime calibration adjustment (ns) for rawKLMs
        self.ct0Scint = [[[5, 7], [-27, -24], [-29, -45], [-27, -32], [3, 6], [34, 35], [48, 44], [33, 38],
                          [4, 7], [-28, -27], [-39, -34], [-36, -33], [2, 5], [25, 30], [46, 49], [41, 31]],
                         [[0, 0], [-32, -32], [-29, -54], [-31, -40], [-1, -1], [29, 27], [41, 41], [28, 28],
                          [-2, -1], [-32, -34], [-38, -45], [-40, -41], [-3, -3], [21, 20], [41, 42], [41, 19]]]

        #: per-layer variations in EKLM-ctime calibration adjustment (ns) for rawKLMs
        self.ct0EKLM = [[[0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0],
                         [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0],
                         [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0],
                         [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0]],
                        [[0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0],
                         [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0],
                         [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0],
                         [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0]]]
        #: Output ROOT TFile that will contain the histograms/scatterplots
        self.histogramFile = ROOT.TFile.Open(self.histName, "RECREATE")
        # All histograms/scatterplots in the output file will show '# of events' only
        ROOT.gStyle.SetOptStat(10)

        # create the rawKLM histograms

        #: histogram of RPC mapped-channel TDC - trigger value, corrected for inter-layer variation
        self.hist_mappedRPCTimeCal = ROOT.TH1F(
            'mappedRPCTimeCal', expRun + 'RPC mapped-strip time distribution;t - t(trigger) - dt(layer) (ns)', 256, -0.5, 1023.5)
        #: dictionary of histograms of RPC layer-corrected TDC - trigger value, keyed by coarse window-start value
        self.dict_mappedRPCTimeCalByWindow = {}
        #: cached window-start value (initialized to known 'CAFE')
        self.windowStartValue = 0xcafe
        print("WindowStartValue = {0:04x} n = 0".format(self.windowStartValue))
        label = "mappedRPCTimeCalByWindow{0:04x}".format(self.windowStartValue)
        title = "{0}RPC mapped-strip time distribution for coarse lookback-window start {1:04x}".format(
            expRun, self.windowStartValue) + ";t - t(trigger) - dt(layer) (ns)"
        self.dict_mappedRPCTimeCalByWindow[self.windowStartValue] = ROOT.TH1F(label, title, 256, -0.5, 1023.5)
        self.hist_mappedRPCTimeCalByWindow = self.dict_mappedRPCTimeCalByWindow[self.windowStartValue]
        #: dictionary of the number of RawKLM hits for each window-start value
        self.dict_nRawKLMs = {}
        self.dict_nRawKLMs[self.windowStartValue] = 0

        # Create the BKLMHit2d-related histograms

        #: scatterplot of end view of forward BKLM for all BKLMHit2ds
        self.hist_occupancyForwardXY = ROOT.TH2F('occupancyForwardXY',
                                                 expRun + 'Forward xy occupancy;x(cm);y(cm)',
                                                 230, -345.0, 345.0, 230, -345.0, 345.0)
        #: scatterplot of end view of backward BKLM for all BKLMHit2ds
        self.hist_occupancyBackwardXY = ROOT.TH2F('occupancyBackwardXYPrompt',
                                                  expRun + 'Backward xy occupancy;x(cm);y(cm)',
                                                  230, -345.0, 345.0, 230, -345.0, 345.0)

        #: dictionary of scatterplots of end view of forward BKLM, keyed by coarse window-start value
        self.dict_occupancyXYByWindow = {}
        label = "occupancyXYByWindow{0:04x}".format(self.windowStartValue)
        title = "{0}Forward xy occupancy for coarse lookback-window start {1:04x};x(cm);y(cm)".format(expRun, self.windowStartValue)
        self.dict_occupancyXYByWindow[self.windowStartValue] = ROOT.TH2F(label, title, 230, -345.0, 345.0, 230, -345.0, 345.0)
        self.hist_occupancyXYByWindow = self.dict_occupancyXYByWindow[self.windowStartValue]
        #: dictionary of the number of BKLMHit2ds for each window-start value
        self.dict_nHit2ds = {}
        self.dict_nHit2ds[self.windowStartValue] = 0
        #: highest observed window-start value
        self.maxWindowStartValue = 0

    def terminate(self):
        """Handle job termination: draw histograms, close output files"""

        canvas = ROOT.TCanvas("canvas", self.pdfName, 1600, 1600)
        title = '{0}['.format(self.pdfName)
        canvas.SaveAs(title)
        canvas.Clear()
        canvas.Divide(1, 1)
        canvas.GetPad(0).SetGrid(1, 1)
        canvas.GetPad(0).Update()
        nWindows = int(self.maxWindowStartValue / 64)
        hist_nRawKLMs = ROOT.TH1F('nRawKLMs', 'Total number of RawKLM hits;Window-start value',
                                  nWindows, 0, self.maxWindowStartValue)
        hist_nRawKLMs.SetStats(False)
        for key in self.dict_nRawKLMs:
            hist_nRawKLMs.Fill(key, self.dict_nRawKLMs[key])
        hist_nRawKLMs.Draw("HIST")
        canvas.Print(self.pdfName, "Title:{0}".format(hist_nRawKLMs.GetName()))
        hist_nHit2ds = ROOT.TH1F('nBKLMHit2ds', 'Total number of BKLMHit2ds;Window-start value',
                                 nWindows, 0, self.maxWindowStartValue)
        hist_nHit2ds.SetStats(False)
        for key in self.dict_nHit2ds:
            hist_nHit2ds.Fill(key, self.dict_nHit2ds[key])
        hist_nHit2ds.Draw("HIST")
        lastTitle = "Title:{0}".format(hist_nHit2ds.GetName())
        canvas.Print(self.pdfName, "Title:{0}".format(hist_nHit2ds.GetName()))

        if self.verbosity > 0:
            self.hist_mappedRPCTimeCal.Draw()
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_mappedRPCTimeCal.GetName()))
            for key in self.dict_mappedRPCTimeCalByWindow:
                theHist = self.dict_mappedRPCTimeCalByWindow[key]
                theHist.Draw()
                canvas.Print(self.pdfName, "Title:{0}".format(theHist.GetName()))

            self.hist_occupancyBackwardXY.Draw("colz")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_occupancyBackwardXY.GetName()))
            self.hist_occupancyForwardXY.Draw("colz")
            canvas.Print(self.pdfName, "Title:{0}".format(self.hist_occupancyForwardXY.GetName()))
            for key in self.dict_occupancyXYByWindow:
                theHist = self.dict_occupancyXYByWindow[key]
                theHist.Draw("colz")
                lastTitle = "Title:{0}".format(theHist.GetName())
                canvas.Print(self.pdfName, lastTitle)
        pdfNameLast = '{0}]'.format(self.pdfName)
        canvas.Print(pdfNameLast, lastTitle)
        self.histogramFile.Write()
        self.histogramFile.Close()
        print('Goodbye')

    def beginRun(self):
        """Handle begin of run: print diagnostic message"""
        EventMetaData = Belle2.PyStoreObj('EventMetaData')
        print('beginRun', EventMetaData.getRun())

    def endRun(self):
        """Handle end of run: print diagnostic message"""
        EventMetaData = Belle2.PyStoreObj('EventMetaData')
        print('endRun', EventMetaData.getRun())

    def event(self):
        """Process one event: fill histograms"""

        EventMetaData = Belle2.PyStoreObj('EventMetaData')
        event = EventMetaData.getEvent()
        rawklms = Belle2.PyStoreArray('RawKLMs')
        hit2ds = Belle2.PyStoreArray('BKLMHit2ds')

        # Process the RawKLMs

        for copper in range(0, len(rawklms)):
            rawklm = rawklms[copper]
            if rawklm.GetNumEntries() != 1:
                print('##0 Event', event, 'copper', copper, ' getNumEntries=', rawklm.GetNumEntries())
                continue
            nodeID = rawklm.GetNodeID(0) - self.BKLM_ID
            if nodeID >= self.EKLM_ID - self.BKLM_ID:
                nodeID = nodeID - (self.EKLM_ID - self.BKLM_ID) + 4
            if (nodeID < 0) or (nodeID > 4):  # examine BKLM nodes only
                continue
            trigCtime = (rawklm.GetTTCtime(0) & 0x7ffffff) << 3  # (ns)
            for finesse in range(0, 4):
                dc = (finesse << 2) + (copper & 0x3)
                sectorFB = self.dcToSectorFB[dc]
                nWords = rawklm.GetDetectorNwords(0, finesse)
                if nWords <= 0:
                    continue
                bufSlot = rawklm.GetDetectorBuffer(0, finesse)
                lastWord = bufSlot[nWords - 1]
                windowStartValue = (lastWord >> 16) & 0xffff
                if windowStartValue != self.windowStartValue:
                    if windowStartValue in self.dict_mappedRPCTimeCalByWindow:
                        self.windowStartValue = windowStartValue
                        self.hist_mappedRPCTimeCalByWindow = self.dict_mappedRPCTimeCalByWindow[windowStartValue]
                        self.hist_occupancyXYByWindow = self.dict_occupancyXYByWindow[windowStartValue]
                    else:
                        if (copper == 0) and (finesse == 0):
                            print("Event {2}: new WindowStartValue = {0:04x} n = {1}".format(windowStartValue, nWords, event))
                            if windowStartValue > self.maxWindowStartValue:
                                self.maxWindowStartValue = windowStartValue
                            self.windowStartValue = windowStartValue
                            expRun = 'e{0:02d}r{1}: '.format(int(self.exp), int(self.run))
                            label = "mappedRPCTimeCalByWindow{0:04x}".format(windowStartValue)
                            title = "{0}RPC mapped-strip time distribution for coarse lookback-window start {1:04x}".format(
                                expRun, windowStartValue) + ";t - t(trigger) - dt(layer) (ns)"
                            self.dict_mappedRPCTimeCalByWindow[windowStartValue] = ROOT.TH1F(label, title, 256, -0.5, 1023.5)
                            self.hist_mappedRPCTimeCalByWindow = self.dict_mappedRPCTimeCalByWindow[windowStartValue]
                            self.dict_nRawKLMs[windowStartValue] = 0
                            label = "occupancyXYByWindow{0:04x}".format(windowStartValue)
                            title = "{0}Forward xy occupancy for coarse lookback-window start {1:04x};x(cm);y(cm)".format(
                                expRun, windowStartValue)
                            self.dict_occupancyXYByWindow[windowStartValue] = ROOT.TH2F(label, title,
                                                                                        230, -345.0, 345.0, 230, -345.0, 345.0)
                            self.hist_occupancyXYByWindow = self.dict_occupancyXYByWindow[windowStartValue]
                            self.dict_nHit2ds[windowStartValue] = 0
                        else:
                            print("Event {2}: lastWord = {0:04x} n = {1} for copper {3} finesse {4}".format(
                                windowStartValue, nWords, event, copper, finesse))
                if lastWord & 0xffff != 0:
                    print("##1 Event", event, 'copper', copper, 'finesse', finesse, 'n=', nWords, 'lastWord=', hex(lastWord))
                if (nWords % 2) == 0:
                    print("##2 Event", event, 'copper', copper, 'finesse', finesse, 'n=', nWords, 'should be odd -- skipping')
                    continue
                n = nWords >> 1  # number of Data-Concentrator data packets
                self.dict_nRawKLMs[windowStartValue] += n
                # first (only) pass over this DC's hits: histogram everything
                for j in range(0, n):
                    word0 = bufSlot[j * 2]
                    word1 = bufSlot[j * 2 + 1]
                    ctime = word0 & 0xffff
                    channel = (word0 >> 16) & 0x7f
                    axis = (word0 >> 23) & 0x01
                    lane = (word0 >> 24) & 0x1f  # 1..2 for scints, 8..20 for RPCs (=readout-board slot - 7)
                    flag = (word0 >> 30) & 0x03  # 1 for RPCs, 2 for scints
                    tdc = (word1 >> 16) & 0x07ff
                    isRPC = (flag == 1)
                    electId = (channel << 12) | (axis << 11) | (lane << 6) | (finesse << 4) | nodeID
                    if electId in self.electIdToModuleId:  # BKLM mapped-channel histograms
                        if isRPC:
                            t = (tdc - trigCtime) & 0x03ff  # in ns, range is 0..1023
                            tCal = t - self.t0RPC[axis][sectorFB][lane - 6]
                            self.hist_mappedRPCTimeCal.Fill(tCal)
                            self.hist_mappedRPCTimeCalByWindow.Fill(tCal)

        # Process the BKLMHit2ds

        self.dict_nHit2ds[windowStartValue] += len(hit2ds)
        for hit2d in hit2ds:
            key = hit2d.getModuleID()
            fb = (key & self.BKLM_END_MASK) >> self.BKLM_END_BIT
            x = hit2d.getGlobalPositionX()
            y = hit2d.getGlobalPositionY()
            if fb == 0:  # backward
                self.hist_occupancyBackwardXY.Fill(x, y)
            else:  # forward
                self.hist_occupancyForwardXY.Fill(x, y)
                self.hist_occupancyXYByWindow.Fill(x, y)
