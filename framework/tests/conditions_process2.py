#!/usr/bin/env python3

"""
Check that running process twice actually reinitializes the database correctly
"""

import basf2
import b2test_utils

basf2.set_random_seed("something important")
b2test_utils.configure_logging_for_tests()

# run once and print the beam parameters after we modified the database
# settings to add an additional globaltag just to check if the settings are
# kept
basf2.conditions.globaltags += ["Legacy_MagneticField_MC8_MC9_MC10"]
p = basf2.Path()
p.add_module("EventInfoSetter")
p.add_module("PrintBeamParameters")
basf2.process(p)
# run again to make sure we get the same globaltags
basf2.process(p)
# run again but with empty globaltag. This should fail horribly
basf2.conditions.reset()
basf2.conditions.override_globaltags([])
b2test_utils.safe_process(p)
