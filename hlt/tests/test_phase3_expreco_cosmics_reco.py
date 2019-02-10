#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from softwaretrigger.path_functions import add_expressreco_processing
from softwaretrigger.test_support import create_test_path, finalize_test_path

path, tempfolder = create_test_path(runtype="cosmics", expNum=1003, location="expressreco")

add_expressreco_processing(path, run_type="cosmics", do_reconstruction=True,
                           data_taking_period="phase3")

finalize_test_path(path, tempfolder, has_softwaretriggerresult=False)
