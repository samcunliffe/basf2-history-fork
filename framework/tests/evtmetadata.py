#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


class NoopModule(Module):
    """Doesn't do anything."""

    def __init__(self):
        """constructor."""
        super(NoopModule, self).__init__()
        self.setName('NoopModule')


class EvtMetaDataTest(Module):

    """Prints EventMetaData objects and stops event processing in event 3."""

    def __init__(self):
        """constructor."""

        super(EvtMetaDataTest, self).__init__()
        self.setName('EvtMetaDataTest')
        B2INFO('event\trun\texp')

    def initialize(self):
        """reimplementation of Module::initialize()."""

        B2INFO('EvtMetaDataTest::initialize()')

    def beginRun(self):
        """reimplementation of Module::beginRun()."""

        B2INFO('EvtMetaDataTest::beginRun()')

    def event(self):
        """reimplementation of Module::event()."""

        evtmetadata = Belle2.PyStoreObj('EventMetaData')
        if not evtmetadata:
            B2ERROR('No EventMetaData found')
        else:
            event = evtmetadata.obj().getEvent()
            run = evtmetadata.obj().getRun()
            exp = evtmetadata.obj().getExperiment()
            B2INFO(str(event) + '\t' + str(run) + '\t' + str(exp))
            if event == 3:
                # stop event processing.
                evtmetadata.obj().setEndOfData()

    def endRun(self):
        """reimplementation of Module::endRun()."""

        B2INFO('EvtMetaDataTest::endRun()')

    def terminate(self):
        """reimplementation of Module::terminate()."""

        B2INFO('EvtMetaDataTest::terminate()')


# Normal steering file part begins here

# Create main path
main = create_path()

# exp 0 has only 2 events, so cannot trigger the test module,
# exp 1 has no events and will be skipped
# exp 2 will be stopped in event 3 by EvtMetaDataTest
evtmetagen = register_module('EvtMetaGen')
evtmetagen.param('ExpList', [0, 1, 2])
evtmetagen.param('RunList', [1, 2, 3])
evtmetagen.param('EvtNumList', [2, 0, 5])

evtmetadatatest = EvtMetaDataTest()

# not used for anything, just checking wether the master module
# can be found if it's not the first module in the path.
main.add_module(NoopModule())

main.add_module(evtmetagen)
main.add_module(evtmetadatatest)

process(main)
