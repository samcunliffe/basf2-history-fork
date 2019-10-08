#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------------------------
# Make a flat ntuple to study single photon time resolution with di-muon data
# by comparing photon times w.r.t leading PDF peak position.
# The following draw command must show a peak that represents an overall time resolution:
#    tree->Draw("time-t0>>h(100,-1,2)", "t0<10")
#
# Usage: basf2 cdst_timeResoNtuple.py -i <cdst_file-dimuon_skim.root> [mc]
# ---------------------------------------------------------------------------------------

from basf2 import *
from ROOT import Belle2
from ROOT import TH1F, TH2F, TFile
from ROOT import gROOT, AddressOf, gRandom
import math
import ROOT
import glob
import sys

MC = False
if len(sys.argv) > 1:
    MC = True


gROOT.ProcessLine('struct TreeStruct {\
   int run;       /* run number */ \
   float offset;  /* current bunch offset */ \
   int usedTrk;   /* number of tracks used for bunch reconstruction */ \
   int slot;      /* slot ID */ \
   float p;       /* extHit momentum */ \
   float cth;     /* extHit local cos(theta) (global must be pretty much the same) */ \
   float phi;     /* extHit local phi */ \
   float z;       /* extHit local z */ \
   float x;       /* extHit local x */ \
   int charge;    /* particle charge */ \
   int channel ;  /* channel */ \
   int pixel ;    /* pixel ID */ \
   float time;    /* photon time */ \
   float timeErr; /* photon time uncertainty */ \
   int pulseHeight; /* photon pulse height */ \
   float pulseWidth; /* photon pulse width */ \
   int sample;    /* sample number modulo 256 */ \
   int status;    /* calibration status bits */ \
   float t0;      /* leading PDF peak: position */ \
   float wid0;    /* leading PDF peak: width w/o TTS */ \
   float nph0;    /* leading PDF peak: number of photons */ \
   float t1;      /* next to leading PDF peak: position */ \
   float wid1;    /* next to leading PDF peak: width w/o TTS */ \
   float nph1;    /* next to leading PDF peak: number of photons */ \
};')

from ROOT import TreeStruct


class Ntuple(Module):
    ''' Makes a flat ntuple '''

    #: histogram counter
    nhisto = 0

    def initialize(self):
        ''' initialize: open root file, construct ntuple '''

        if MC:
            outName = 'timeResoNtuple-MC.root'
        else:
            evtMetaData = Belle2.PyStoreObj('EventMetaData')
            run = '{:0=5d}'.format(evtMetaData.getRun())
            outName = 'timeResoNtuple-r' + run + '.root'

        #: file object
        self.file = ROOT.TFile(outName, 'recreate')
        #: histogram of bunch offset
        self.bunchOffset = TH1F("bunchOffset", "bunch offset", 100, -1.0, 1.0)
        self.bunchOffset.SetXTitle("bunch offset [ns]")
        #: histogram of momentum
        self.h_momentum = TH1F("momentum", "momentum", 100, 0.0, 10.0)
        self.h_momentum.SetXTitle("p [GeV/c]")
        #: histogram of cos(theta)
        self.h_cth = TH1F("cos_theta", "local cos(theta)", 100, -1.0, 1.0)
        self.h_cth.SetXTitle("cos #theta")
        #: histogram of local phi
        self.h_phi = TH1F("local_phi", "local phi", 100, -math.pi, math.pi)
        self.h_phi.SetXTitle("local #phi")
        #: histogram of local z
        self.h_z = TH1F("local_z", "local z", 100, -140.0, 140.0)
        self.h_z.SetXTitle("local z [cm]")
        #: histogram of local x
        self.h_x = TH1F("local_x", "local x", 100, -23.0, 23.0)
        self.h_x.SetXTitle("local x [cm]")
        #: histogram of charge
        self.h_charge = TH1F("charge", "charge", 3, -1.5, 1.5)
        self.h_charge.SetXTitle("charge")
        #: tree object
        self.tree = ROOT.TTree('tree', '')
        #: data structure
        self.data = TreeStruct()

        for key in TreeStruct.__dict__.keys():
            if '__' not in key:
                formstring = '/F'
                if isinstance(self.data.__getattribute__(key), int):
                    formstring = '/I'
                self.tree.Branch(key, AddressOf(self.data, key), key + formstring)

    def sortPeaks(self, unsortedPeaks):
        ''' sort PDF peaks according to their positions '''

        py_list = [x for x in unsortedPeaks]
        return sorted(py_list, key=lambda x: (x.mean))

    def pdfHistogram(self, pdf):
        ''' make histogram of PDF peak positions for the first 20 tracks '''

        self.nhisto += 1
        if self.nhisto > 20:
            return
        h = TH2F('pdf' + str(self.nhisto), 'muon PDF',
                 512, 0.0, 512.0, 1000, 0.0, 75.0)
        h.SetXTitle('pixelID - 1')
        h.SetYTitle('peak positions [ns]')
        for x in range(512):
            peaks = pdf[x]
            for peak in peaks:
                h.Fill(x, peak.mean)
        h.Write()

    def event(self):
        ''' event processing '''

        recBunch = Belle2.PyStoreObj('TOPRecBunch')
        if not recBunch:
            B2ERROR('no TOPRecBunch')
            return
        if not recBunch.isReconstructed():
            return
        self.bunchOffset.Fill(recBunch.getCurrentOffset())

        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        self.data.run = evtMetaData.getRun()
        self.data.offset = recBunch.getCurrentOffset()
        self.data.usedTrk = recBunch.getUsedTracks()

        for track in Belle2.PyStoreArray('Tracks'):
            pdfs = track.getRelated('TOPPDFCollections')
            if not pdfs:
                continue
            self.data.slot = pdfs.getModuleID()
            momentum = pdfs.getAssociatedLocalMomentum()
            position = pdfs.getAssociatedLocalHit()
            self.data.p = momentum.Mag()
            self.data.cth = momentum.CosTheta()
            self.data.phi = momentum.Phi()
            self.data.z = position.Z()
            self.data.x = position.X()
            try:
                tfit = track.getTrackFitResultWithClosestMass(Belle2.Const.muon)
                self.data.charge = tfit.getChargeSign()
            except:
                B2ERROR("No trackFitResult available")
                continue
            self.h_momentum.Fill(self.data.p)
            self.h_cth.Fill(self.data.cth)
            self.h_phi.Fill(self.data.phi)
            self.h_z.Fill(self.data.z)
            self.h_x.Fill(self.data.x)
            self.h_charge.Fill(self.data.charge)
            try:
                pdf = pdfs.getHypothesisPDF(13)
            except:
                B2ERROR("No PDF available for PDG = 13")
                continue
            self.pdfHistogram(pdf)
            for digit in Belle2.PyStoreArray('TOPDigits'):
                if digit.getModuleID() == self.data.slot and digit.getHitQuality() == 1:
                    peaks = pdf[digit.getPixelID() - 1]
                    if peaks.empty():
                        continue
                    self.data.channel = digit.getChannel()
                    self.data.pixel = digit.getPixelID()
                    self.data.time = digit.getTime()
                    self.data.timeErr = digit.getTimeError()
                    self.data.pulseHeight = digit.getPulseHeight()
                    self.data.pulseWidth = digit.getPulseWidth()
                    self.data.sample = digit.getModulo256Sample()
                    self.data.status = digit.getStatus()
                    sorted_peaks = self.sortPeaks(peaks)
                    self.data.t0 = sorted_peaks[0].mean
                    self.data.wid0 = sorted_peaks[0].width
                    self.data.nph0 = sorted_peaks[0].area
                    self.data.t1 = self.data.t0 + 100
                    self.data.wid1 = 0
                    self.data.nph1 = 0
                    if peaks.size() > 1:
                        self.data.t1 = sorted_peaks[1].mean
                        self.data.wid1 = sorted_peaks[1].width
                        self.data.nph1 = sorted_peaks[1].area
                    self.tree.Fill()

    def terminate(self):
        ''' terminate: close root file '''

        self.file.cd()
        self.file.Write()
        self.file.Close()


# If real data, define a global tag (note: the one given bellow will become out-dated!)
if not MC:
    use_central_database('data_reprocessing_proc8')

# Create path
main = create_path()

# Input: cdst file(s), use -i option
main.add_module('RootInput')

# Initialize TOP geometry parameters (creation of Geant geometry is not needed)
main.add_module('TOPGeometryParInitializer')

# Channel masking
main.add_module('TOPChannelMasker')

# Make a muon PDF available at datastore
main.add_module('TOPPDFDebugger', pdgCodes=[13])

# Write ntuple
main.add_module(Ntuple())

# Print progress
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
