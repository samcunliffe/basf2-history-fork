#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
import ROOT
from ROOT import Belle2


class registerEventTrackingInfo(basf2.Module):
    """Small module that registers the EventTrackingInfo that is used to set general tracking-related flags"""

    def __init__(self):
        super(registerEventTrackingInfo, self).__init__()

    def initialize(self):
        """ Register the StoreArray on the DataStore"""
        self.m_eventLevelTrackingInfo = Belle2.PyStoreObj(Belle2.EventLevelTrackingInfo.Class())
        self.m_eventLevelTrackingInfo.registerInDataStore()

    def event(self):
        """ ... and create the StoreArray """
        self.m_eventLevelTrackingInfo.create()
