#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import setAnalysisConfigParams
import os
import re
import requests
import http


def setupBelleDatabaseServer():
    """
    Sets the Belle DB server to the one recommended in /sw/belle/local/var/belle_postgres_server.

    If the user does not have the access rights to the above file, the can01 server is set by default.
    """
    belleDBServerFile = '/sw/belle/local/var/belle_postgres_server'
    belleDBServer = 'can01'

    try:
        with open(belleDBServerFile) as f:
            belleDBServer = (f.read()).strip()
    except IOError:
        pass

    os.environ['BELLE_POSTGRES_SERVER'] = belleDBServer


def setupBelleMagneticField(path):
    """
    This function set the Belle Magnetic field (constant).
    """
    B2WARNING('setupBelleMagneticField function is obsolete. Please remove it from your scripts. '
              'The Belle magnetic field is now being set via the settings in inputMdst(List) fucntion.')


def setupB2BIIDatabase(isMC=False):
    """
    Setup the database for B2BII.

    This automatically chooses the correct global tag and sets up a database suitable for B2BII conversion.

    Warning:
        This function is not up to date and should not be called

    Args:
        isMC (bool): should be True for MC data and False for real data
    """
    # we only want the central database with the B2BII content
    tagname = "B2BII%s" % ("_MC" if isMC else "")
    # and we want to cache them in a meaningful but separate directory
    payloaddir = tagname + "_database"
    reset_database()
    use_database_chain()
    # fallback to previously downloaded payloads if offline
    if not isMC:
        use_local_database("%s/dbcache.txt" % payloaddir, payloaddir, True, LogLevel.ERROR)
        # get payloads from central database
        use_central_database(tagname, LogLevel.WARNING, payloaddir)
    # unless they are already found locally
    if isMC:
        use_local_database("%s/dbcache.txt" % payloaddir, payloaddir, False, LogLevel.WARNING)


def convertBelleMdstToBelleIIMdst(inputBelleMDSTFile, applySkim=True,
                                  useBelleDBServer=None,
                                  generatorLevelReconstruction=False,
                                  generatorLevelMCMatching=False,
                                  path=None, entrySequences=None,
                                  matchType2E9oE25Threshold=-1.1,
                                  HadronA=True, HadronB=True):
    """
    Loads Belle MDST file and converts in each event the Belle MDST dataobjects to Belle II MDST
    data objects and loads them to the StoreArray.

    Args:
        inputBelleMDSTFile (str): Name of the file(s) to be loaded.
        applySkim (bool): Apply skim conditions in B2BIIFixMdst.
        useBelleDBServer (str): None to use the recommended BelleDB server.
        generatorLevelReconstruction (bool): Enables to bypass corrections applied in B2BIIFixMdst.
        generatorLevelMCMatching (bool): Enables to switch MCTruth matching to generator-level particles.
        path (basf2.Path): Path to add modules in.
        entrySequences (list(str)): The number sequences (e.g. 23:42,101) defining
            the entries which are processed for each inputFileName.
        convertECLCrystalEnergies (bool): Enables to convert Datecl_mc_ehits into ECLHits
        convertExtHits (bool): Enables to convert Mdst_ecl_trk into ExtHits.
        matchType2E9oE25Threshold (float): Clusters with a E9/E25 value above this threshold are classified as neutral
            even if tracks are matched to their connected region (matchType == 2 in basf).
        HadronA (bool): Enables to switch on HadronA skim in B2BIIFixMdst module.
        HadronB (bool): Enables to switch on HadronB skim in B2BIIFixMdst module.
    """

    if useBelleDBServer is None:
        setupBelleDatabaseServer()
    else:
        os.environ['BELLE_POSTGRES_SERVER'] = useBelleDBServer

    B2INFO('Belle DB server is set to: ' + os.environ['BELLE_POSTGRES_SERVER'])

    setAnalysisConfigParams({'mcMatchingVersion': 'Belle'}, path)

    input = register_module('B2BIIMdstInput')
    if inputBelleMDSTFile is not None:
        input.param('inputFileNames', parse_process_url(inputBelleMDSTFile))
    if entrySequences is not None:
        input.param('entrySequences', entrySequences)
    # input.logging.set_log_level(LogLevel.DEBUG)
    # input.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
    path.add_module(input)

    # we need magnetic field which is different than default.
    # shamelessly copied from analysis/scripts/modularAnalysis.py:inputMdst
    from ROOT import Belle2  # reduced scope of potentially-misbehaving import
    field = Belle2.MagneticField()
    field.addComponent(Belle2.MagneticFieldComponentConstant(Belle2.B2Vector3D(0, 0, 1.5 * Belle2.Unit.T)))
    Belle2.DBStore.Instance().addConstantOverride("MagneticField", field, False)

    if (not generatorLevelReconstruction):
        # Fix MSDT Module
        fix = register_module('B2BIIFixMdst')
        # fix.logging.set_log_level(LogLevel.DEBUG)
        # fix.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
        # Hadron skim settings
        fix.param('HadronA', HadronA)
        fix.param('HadronB', HadronB)
        if (HadronA is not True and HadronB is True):
            B2WARNING('HadronB(J) skim includes HadronA requirements. '
                      'Are you sure you want to turn off HadronA when HadronB(J) is still applied?')
        path.add_module(fix)

        if(applySkim):
            emptypath = create_path()
            # discard 'bad events' marked by fixmdst
            fix.if_value('<=0', emptypath)
        else:
            B2INFO('applySkim is set to be False. '
                   'No bad events marked by fixmdst will be discard.')
    # Convert MDST Module
    convert = register_module('B2BIIConvertMdst')
    if (generatorLevelMCMatching):
        convert.param('mcMatchingMode', 'GeneratorLevel')
    convert.param("matchType2E9oE25Threshold", matchType2E9oE25Threshold)
    # convert.logging.set_log_level(LogLevel.DEBUG)
    # convert.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
    path.add_module(convert)


def parse_process_url(url):
    """
    Handle process_url style requests.
    If url is a list just return it without looking at it. If the url starts
    with http(s):// return the list of files provided by that url, otherwise
    check if a file with the name given as url exists and return this as a list
    with one item. If not such file exists return an empty list

    >>> parse_process_url(["foo", "bar"])
    ['foo', 'bar']
    >>> parse_process_url("/dev/null")
    ['/dev/null']
    >>> parse_process_url("missing_file.mdst")
    []
    >>> parse_process_url("http://bweb3.cc.kek.jp/mdst.php?ex=55&rs=1&re=5&skm=HadronBorJ&dt=on_resonance&bl=caseB") \
        # doctest: +NORMALIZE_WHITESPACE
    ['/group/belle/bdata_b/dstprod/dat/e000055/HadronBJ/0127/on_resonance/00/HadronBJ-e000055r000003-b20090127_0910.mdst',
     '/group/belle/bdata_b/dstprod/dat/e000055/HadronBJ/0127/on_resonance/00/HadronBJ-e000055r000004-b20090127_0910.mdst',
     '/group/belle/bdata_b/dstprod/dat/e000055/HadronBJ/0127/on_resonance/00/HadronBJ-e000055r000005-b20090127_0910.mdst']

    Some files are missing and thus marked with "### LOST ###", make sure we don't pick them up

    >>> parse_process_url("http://bweb3.cc.kek.jp/mdst.php?ex=65&rs=1&re=30&skm=ypipi")
    []

    If the URL cannot be found an error is raised and an empty list is returned.
    Also for other http errors or non existing urls

    >>> parse_process_url("http://httpbin.org/status/404")
    []
    >>> parse_process_url("http://httpbin.org/status/500")
    []
    >>> parse_process_url("http://nosuchurl")
    []
    """

    if isinstance(url, list) or isinstance(url, tuple):
        return url

    if not url.startswith("http://") and not url.startswith("https://"):
        if os.path.exists(url):
            return [url]
        else:
            B2ERROR("Could not parse url '{0}': no such file or directory".format(url))
            return []

    # regular expression to find process_event lines in html response
    process_event = re.compile(br"^\s*process_event\s+(.*)\s+0$", re.MULTILINE)
    # get the url
    try:
        request = requests.get(url)
        request.raise_for_status()
        return [e.decode("ASCII") for e in process_event.findall(request.content)]
    except (requests.ConnectionError, requests.HTTPError) as e:
        B2ERROR("Failed to connect to '{url}': {message}".format(url=url, message=str(e)))

    return []
