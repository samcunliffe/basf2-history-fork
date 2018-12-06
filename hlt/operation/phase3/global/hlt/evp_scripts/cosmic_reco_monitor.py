#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import basf2
from softwaretrigger.path_functions import setup_basf2_and_db, create_hlt_path, add_hlt_processing, finalize_hlt_path


args = setup_basf2_and_db(dbfile="/dev/shm/LocalDB.rel0101/database.txt")
path = create_hlt_path(args)

# no reconstruction or software trigger added at all
add_hlt_processing(path, run_type="cosmics", softwaretrigger_mode="monitoring", data_taking_period="phase3")

finalize_hlt_path(path, args)
basf2.print_path(path)
basf2.process(path)
