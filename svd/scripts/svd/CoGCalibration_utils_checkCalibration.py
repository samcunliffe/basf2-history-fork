# !/usr/bin/env python
# -*- coding: utf-8 -*-

#################################################################################
#
# util module to check SVD CoG Calibration stored in a localDB
#
# called by SVDCoGTimeCalibrationCheck script in
# svd/scripts/caf/SVDCoGTimeCalibrationCheck.py
#
#################################################################################


from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2, TFile, TTree, TH1F, TH2F, TH2D, TGraph, TFitResultPtr
from ROOT import TROOT, gROOT, TF1, TMath, gStyle, gDirectory
import os
# import numpy
import math
import random
from array import array
import basf2
import sys

import matplotlib.pyplot as plt
import numpy as np
from numpy.linalg import inv, pinv
from numpy.linalg import det, norm, cond

svd_recoDigits = "SVDRecoDigitsFromTracks"
cdc_Time0 = "EventT0"
svd_Clusters = "SVDClustersFromTracks"

gROOT.SetBatch(True)

# mode = True


class SVDCoGTimeCalibrationCheckModule(basf2.Module):

    def fillLists(self, svdRecoDigitsFromTracks, svdClustersFromTracks):

        timeCluster = svdClustersFromTracks.getClsTime()
        snrCluster = svdClustersFromTracks.getSNR()
        layerCluster = svdClustersFromTracks.getSensorID().getLayerNumber()
        layerIndex = layerCluster - 3
        sensorCluster = svdClustersFromTracks.getSensorID().getSensorNumber()
        sensorIndex = sensorCluster - 1
        ladderCluster = svdClustersFromTracks.getSensorID().getLadderNumber()
        ladderIndex = ladderCluster - 1
        sideCluster = svdClustersFromTracks.isUCluster()
        if sideCluster:
            sideIndex = 1
        else:
            sideIndex = 0

        hasTimezero = self.cdcEventT0.hasEventT0()
        if hasTimezero:
            TBClusters = svdRecoDigitsFromTracks.getModeByte().getTriggerBin()
            TBIndex = ord(TBClusters)
            tZero = self.cdcEventT0.getEventT0()
            # tZero_err = self.cdcEventT0.getEventT0Uncertainty()
            # tZero_err = 5.1
            tZeroSync = tZero - 7.8625 * (3 - TBIndex)

            # filling histograms
            resHist = self.resList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            resHist.Fill(timeCluster - tZeroSync)
            spHist = self.spList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            spHist.Fill(timeCluster, tZeroSync)
            cogHist = self.cogList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            cogHist.Fill(timeCluster)
            cdcHist = self.cdcList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            cdcHist.Fill(tZeroSync)
            snrHist = self.snrList[layerIndex][ladderIndex][sensorIndex][sideIndex]
            snrHist.Fill(snrCluster)

            self.NTOT = self.NTOT + 1

    def set_localdb(self, localDB):
        self.localdb = localDB

    def initialize(self):
        self.outputFileName = "SVDCoGCalibrationCheck_" + self.localdb + ".root"

        self.resList = []
        self.spList = []
        self.cogList = []
        self.cdcList = []
        self.snrList = []

        geoCache = Belle2.VXD.GeoCache.getInstance()

        self.Evt = 0
        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerList0 = []
            layerList1 = []
            layerList2 = []
            layerList3 = []
            layerList4 = []

            self.resList.append(layerList0)
            self.spList.append(layerList1)
            self.cogList.append(layerList2)
            self.cdcList.append(layerList3)
            self.snrList.append(layerList4)

            for ladder in geoCache.getLadders(layer):
                ladderList0 = []
                ladderList1 = []
                ladderList2 = []
                ladderList3 = []
                ladderList4 = []

                layerList0.append(ladderList0)
                layerList1.append(ladderList1)
                layerList2.append(ladderList2)
                layerList3.append(ladderList3)
                layerList4.append(ladderList4)

                for sensor in geoCache.getSensors(ladder):
                    sensorList0 = []
                    sensorList1 = []
                    sensorList2 = []
                    sensorList3 = []
                    sensorList4 = []

                    ladderList0.append(sensorList0)
                    ladderList1.append(sensorList1)
                    ladderList2.append(sensorList2)
                    ladderList3.append(sensorList3)
                    ladderList4.append(sensorList4)

        for i in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerN = i.getLayerNumber()
            li = layerN - 3
            for j in geoCache.getLadders(i):
                ladderN = j.getLadderNumber()
                ldi = ladderN - 1
                for k in geoCache.getSensors(j):
                    sensorN = k.getSensorNumber()
                    si = sensorN - 1
                    for s in range(2):
                        self.resList[li][ldi][si].append(
                            TH1F("res" + "_" + str(k) + "." + str(s), " ", 200, -100, 100))
                        self.spList[li][ldi][si].append(
                            TH2D("sp" + "_" + str(k) + "." + str(s), " ", 300, -150, 150, 300, -150, 150))
                        self.cogList[li][ldi][si].append(
                            TH1F("cog" + "_" + str(k) + "." + str(s), " ", 200, -100, 100))
                        self.cdcList[li][ldi][si].append(
                            TH1F("cdc" + "_" + str(k) + "." + str(s), " ", 200, -100, 100))
                        self.snrList[li][ldi][si].append(
                            TH1F("snr" + "_" + str(k) + "." + str(s), " ", 100, 0, 100))

        self.EventT0Hist = TH1F("EventT0", " ", 160, -40, 40)
        self.alphaU = TH1F("alphaU", "first order coefficient ~ U side", 100, 0, 2)
        self.alphaV = TH1F("alphaV", "first order coefficient ~ V side", 100, 0, 2)
        self.betaU = TH1F("betaU", "beta - EventT0Sync average ~ U side", 100, -5, 5)
        self.betaV = TH1F("betaV", "beta - EventT0Sync average ~ V side", 100, -5, 5)

        self.gaus = TF1("gaus", 'gaus(0)', -150, 100)
        self.pol1 = TF1("pol1", "[0] + [1]*x", -150, 150)

        self.NTOT = 0

    def event(self):

        timeClusterU = 0
        timeClusterV = 0
        sideIndex = 0
        TBIndexU = 0
        TBIndexV = 0
        self.Evt = self.Evt + 1

        # fill EventT0 histogram
        self.cdcEventT0 = Belle2.PyStoreObj(cdc_Time0)
        if self.cdcEventT0.hasEventT0():
            et0 = self.EventT0Hist
            et0.Fill(self.cdcEventT0.getEventT0())

        # fill plots
        svdCluster_list = Belle2.PyStoreArray(svd_Clusters)
        svdRecoDigit_list = Belle2.PyStoreArray(svd_recoDigits)

        for svdCluster in svdCluster_list:
            svdRecoDigit = svdCluster.getRelatedTo(svd_recoDigits)
            self.fillLists(svdRecoDigit, svdCluster)

    def terminate(self):

        tfile = TFile(self.outputFileName, 'recreate')
        par = [0, 1]

        geoCache = Belle2.VXD.GeoCache.getInstance()
        gDirectory.mkdir("plots")
        gDirectory.cd("plots")
        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            li = layerNumber - 3
            gDirectory.mkdir("layer" + str(layer))
            gDirectory.cd("layer" + str(layer))
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                ldi = ladderNumber - 1
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    si = sensorNumber - 1
                    for side in range(2):

                        # Resolution distribution Histograms with Gaussian Fit
                        res = self.resList[li][ldi][si][side]
                        res.GetXaxis().SetTitle("cluster time - Synchronized EventT0 (ns)")
                        # fitResult = int(TFitResultPtr(res.Fit(self.gaus, "R")))
                        res.Write()
                        # COG Distribution Histograms
                        cog = self.cogList[li][ldi][si][side]
                        cog.GetXaxis().SetTitle("cluster time (ns)")
                        cog.Write()
                        # CDC EventT0 Distribution Histograms
                        cdc = self.cdcList[li][ldi][si][side]
                        cdc.GetXaxis().SetTitle("Synchronized EventT0 (ns)")
                        cdc.Write()
                        # SNR Distribution Histograms
                        snr = self.snrList[li][ldi][si][side]
                        snrMean = snr.GetMean()
                        snr.GetXaxis().SetTitle("cluster SNR")
                        snr.Write()
                        # ScatterPlot Histograms with Linear Fit
                        sp = self.spList[li][ldi][si][side]
                        covscalebias = sp.GetCovariance()
                        pfxsp = sp.ProfileX()
                        self.pol1.SetParameters(0, 1)
                        pfxsp.Fit(self.pol1, "R")
                        sp.GetXaxis().SetTitle("cluster time (ns)")
                        sp.GetYaxis().SetTitle("synchronized EventT0 (ns)")
                        sp.Write()
                        pfxsp.Write()

                        if side is 1:
                            self.alphaU.Fill(self.pol1.GetParameter(1))
                            self.betaU.Fill(self.pol1.GetParameter(0) - cdc.GetMean())
                        else:
                            self.alphaV.Fill(self.pol1.GetParameter(1))
                            self.betaV.Fill(self.pol1.GetParameter(0) - cdc.GetMean())

            gDirectory.cd("../")

        gDirectory.cd("../")
        self.EventT0Hist.Write()
        self.alphaU.Write()
        self.alphaV.Write()
        self.betaU.Write()
        self.betaV.Write()
        tfile.Close()
