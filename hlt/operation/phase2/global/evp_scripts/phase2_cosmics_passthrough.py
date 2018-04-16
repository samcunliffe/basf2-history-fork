#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, create_hlt_path, add_hlt_dqm, finalize_hlt_path


args = setup_basf2_and_db()
path = create_hlt_path(args)

# no reconstruction or software trigger added at all

finalize_hlt_path(path, args)
basf2.print_path(path)
basf2.process(path)
