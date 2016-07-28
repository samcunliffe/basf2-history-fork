#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Stateless basf2
Import this module at the top of your steering file to get a (more) stateless version of basf2.
This is useful if you want to pickle the path using --dump-path and execute it later using --execute-path.
Usually all calls to functions in basf2 like use_central_database are forgotten if you only save the path.
With this module, these functions are executed again if you execute the pickled path using --execute-path.
"""


import sys
import pickle
import inspect
import unittest.mock as mock
import basf2
sys.modules['original_basf2'] = sys.modules['basf2']
import original_basf2


class BASF2StateRecorder(object):
    """ Drop-in replacement of the basf2 module, which keeps track of all functions calls """
    def __getattr__(self, name):
        """ Return attribute with the given name in the basf2 namespace """
        return getattr(original_basf2, name)


basf2_state_recorder = BASF2StateRecorder()


def process(path, max_event=0):
    """ Process call which pickles the recorded state in addition to the path """
    sys.modules['basf2'] = original_basf2
    original_basf2.process(path, max_event)
    state = [(name, list(map(tuple, x.mock_calls)))
             for name, x in basf2_state_recorder.__dict__.items() if isinstance(x, mock.Mock)]
    pickle_path = original_basf2.fw.get_pickle_path()
    print("Path", path)
    print("State", state)
    if pickle_path != '' and path is not None:
        serialized = original_basf2.serialize_path(path)
        serialized['state'] = state
        pickle.dump(serialized, open(pickle_path, 'bw'))
    sys.modules['basf2'] = basf2_state_recorder


for name, x in original_basf2.__dict__.items():
    # We record function and fake Boost.Python.function objects
    if inspect.isfunction(x) or isinstance(x, type(original_basf2.use_central_database)):
        setattr(basf2_state_recorder, name, mock.Mock(x, side_effect=x))
    # Other names we have to set as well, because otherwise they won't be important by from basf2 import *
    else:
        setattr(basf2_state_recorder, name, x)

basf2_state_recorder.process = process
sys.modules['basf2'] = basf2_state_recorder
