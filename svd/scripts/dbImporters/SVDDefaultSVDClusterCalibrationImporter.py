#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
SVD Default Cluster Calibration importer.
t_min = -20,
"""
import basf2
from basf2 import *
from svd import *
import ROOT
from ROOT import Belle2
from ROOT.Belle2 import SVDClusterCuts
from ROOT.Belle2 import SVDHitTimeSelectionFunction

import os

# default values
clsMinTime = -20
clsSeedSNR = 5
clsAdjSNR = 3
clsMinSNR = 0
clsScaleErrSize1 = 1
clsScaleErrSize2 = 1
clsScaleErrSize3 = 1


class defaultSVDClusterCalibrationImporter(basf2.Module):

    def beginRun(self):

        iov = Belle2.IntervalOfValidity.always()

        cls_payload = Belle2.SVDClusterCalibrations.t_payload()
        time_payload = Belle2.SVDClusterCalibrations.t_time_payload()

        # cluster time
        hitTimeSelection = SVDHitTimeSelectionFunction()
        hitTimeSelection.setMinTime(clsMinTime)

        # cluster reconstruction & position error
        clsParam = SVDClusterCuts()
        clsParam.minSeedSNR = clsSeedSNR
        clsParam.minAdjSNR = clsAdjSNR
        clsParam.minClusterSNR = clsMinSNR
        clsParam.scaleError_clSize1 = clsScaleErrSize1
        clsParam.scaleError_clSize2 = clsScaleErrSize2
        clsParam.scaleError_clSize3 = clsScaleErrSize3

        geoCache = Belle2.VXD.GeoCache.getInstance()

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            for ladder in geoCache.getLadders(layer):
                ladderNumber = ladder.getLadderNumber()
                for sensor in geoCache.getSensors(ladder):
                    sensorNumber = sensor.getSensorNumber()
                    for side in (0, 1):
                        print("setting SVDCluster calibrations for " +
                              str(layerNumber) + "." + str(ladderNumber) + "." + str(sensorNumber))
                        cls_payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, clsParam)
                        time_payload.set(layerNumber, ladderNumber, sensorNumber, bool(side), 1, hitTimeSelection)

        Belle2.Database.Instance().storeData(Belle2.SVDClusterCalibrations.name, cls_payload, iov)
        Belle2.Database.Instance().storeData(Belle2.SVDClusterCalibrations.time_name, time_payload, iov)


use_database_chain()
use_local_database("localDB/database.txt", "localDB", invertLogging=True)

main = create_path()

# Event info setter - execute single event
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1], 'expList': 0, 'runList': 0})
main.add_module(eventinfosetter)

main.add_module("Gearbox")  # fileName="/geometry/Beast2_phase2.xml")
main.add_module("Geometry", components=['SVD'])  # , useDB = False)

main.add_module(defaultSVDClusterCalibrationImporter())

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)
