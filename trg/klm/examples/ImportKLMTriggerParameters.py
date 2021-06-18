#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Import KLMTriggerParameters payloads

from ROOT.Belle2 import KLMTriggerParameters, Database, IntervalOfValidity

parameters = KLMTriggerParameters()
parameters.setNLayers(1)
parameters.setWhichLayers("2:3")

database = Database.Instance()

iov = IntervalOfValidity(0, 0, 0, -1)
database.storeData('EventsOfDoomParameters', parameters, iov)
iov = IntervalOfValidity(1002, 0, 1002, -1)
database.storeData('EventsOfDoomParameters', parameters, iov)
iov = IntervalOfValidity(1003, 0, 1003, -1)
database.storeData('EventsOfDoomParameters', parameters, iov)
