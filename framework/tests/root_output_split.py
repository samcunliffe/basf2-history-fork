#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from ROOT import Belle2
from b2test_utils import clean_working_directory, safe_process
import subprocess
import json
import os

# @cond internal_test


class CreateDummyData(basf2.Module):
    """Create some random data to have event size not be too small"""
    def __init__(self, size):
        super().__init__()
        self.size = size // 8
        self.chunk_data = Belle2.PyStoreObj(Belle2.TestChunkData.Class())

    def initialize(self):
        self.chunk_data.registerInDataStore()

    def event(self):
        self.chunk_data.assign(Belle2.TestChunkData(self.size))


def check_file(filename):
    subprocess.check_call(["b2file-check", filename])


def get_metadata(filename):
    meta = subprocess.check_output(["b2file-metadata-show", "--json", filename])
    return json.loads(meta)


if __name__ == "__main__":
    basf2.logging.log_level = basf2.LogLevel.WARNING
    basf2.set_random_seed("something important")
    with clean_working_directory():
        # create 2 files around 3 MB
        path = basf2.Path()
        path.add_module("EventInfoSetter", evtNumList=550)
        path.add_module(CreateDummyData(1024 * 10))  # 10 kb dummy data
        path.add_module("RootOutput", outputFileName="test_split.root", buildIndex=False, updateFileCatalog=False,
                        compressionAlgorithm=0, compressionLevel=0, outputSplitSize=3)
        assert safe_process(path) == 0, "RootOutput failed"

        # check files and set a well known lfn
        for i in range(2):
            check_file(f"test_split.f0000{i}.root")
            subprocess.check_call(["b2file-metadata-add", "-l", f"parent{i}", f"test_split.f0000{i}.root"])
        assert os.path.exists("test_split.f00002.root") is False, "There should not be a third file"

        # create 3 files around 2 MB from the previous files
        path = basf2.Path()
        path.add_module("RootInput", inputFileNames=["test_split.f00000.root", "test_split.f00001.root"])
        path.add_module("RootOutput", outputFileName="file://test_parents.root", buildIndex=False, updateFileCatalog=False,
                        compressionAlgorithm=0, compressionLevel=0, outputSplitSize=2)
        assert safe_process(path) == 0, "RootInput/Output failed"

        # check the files
        for i in range(3):
            check_file(f"test_parents.f0000{i}.root")
        assert os.path.exists("test_split.f00003.root") is False, "There should not be a fourth file"

        # check metadata
        meta = [get_metadata(e) for e in ["test_parents.f00000.root", "test_parents.f00001.root", "test_parents.f00002.root"]]
        assert meta[0]["parents"] == ["parent0"], "parents wrong"
        assert meta[1]["parents"] == ["parent0", "parent1"], "parents wrong"
        assert meta[2]["parents"] == ["parent1"], "parents wrong"

        last = 0
        for m in meta:
            assert m["eventLow"] == last + 1, "eventLow is not correct"
            last = m["eventHigh"]
            assert m["eventLow"] + m["nEvents"] - 1 == m["eventHigh"], "event high is inconsistent"

        assert sum(e["nEvents"] for e in meta) == 550, "number of events must be correct"

    # check what happens with other extensions
    check_filenames = {
        # no extension: just add one
        "test_noext": "test_noext.f00000.root",
        # any other extension: replace
        "test_otherext.foo": "test_otherext.f00000.root",
        # but keep paremeters or anchors in urls untouched. TFile::Open ignores
        # them for file:// urls but they are printed on the command line
        "file://test_param?foo=bar": "test_param.f00000.root",
        "file://test_anchor#foo": "test_anchor.f00000.root",
    }
    basf2.logging.log_level = basf2.LogLevel.INFO
    for name, result in check_filenames.items():
        with clean_working_directory():
            path = basf2.Path()
            path.add_module("EventInfoSetter")
            path.add_module("RootOutput", outputFileName=name, outputSplitSize=1, updateFileCatalog=False)
            safe_process(path)
            assert os.listdir() == [result], "wrong output file name"

# @endcond
