#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, add_hlt_dqm, create_hlt_path, finalize_hlt_path

setup_basf2_and_db()
path = create_hlt_path()

# no reconstruction or software trigger added at all
# currently, not all DQM modules can be executed without the reconstruction module in the path
# add_hlt_dqm(path, run_type="collisions")

finalize_hlt_path(path)
basf2.print_path(path)
basf2.process(path)
