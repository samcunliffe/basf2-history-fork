#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *

analysis_main = create_path()


def inputMdst(filename, path=analysis_main):
    """
    Loads the specified ROOT (DST/mDST/muDST) file with the RootInput module.

    @param filename the name of the file to be loaded
    @param modules are added to this path
    """

    roinput = register_module('RootInput')
    roinput.param('inputFileName', filename)
    path.add_module(roinput)
    gearbox = register_module('Gearbox')
    path.add_module(gearbox)
    progress = register_module('Progress')
    path.add_module(progress)


def inputMdstList(filelist, path=analysis_main):
    """
    Loads the specified ROOT (DST/mDST/muDST) files with the RootInput module.

    @param filelist the filename list of files to be loaded
    @param modules are added to this path
    """

    roinput = register_module('RootInput')
    roinput.param('inputFileNames', filelist)
    path.add_module(roinput)
    gearbox = register_module('Gearbox')
    path.add_module(gearbox)
    progress = register_module('Progress')
    path.add_module(progress)


def outputMdst(filename, path=analysis_main):
    import reconstruction
    reconstruction.add_mdst_output(path, mc=True, filename=filename)


def outputUdst(filename, path=analysis_main):
    """
    Save uDST (micro-Data Summary Tables) = MDST + Particles
    """

    import reconstruction
    partBranches = ['Particles', 'ParticlesToMCParticles',
                    'ParticlesToPIDLikelihoods']
    reconstruction.add_mdst_output(path, mc=True, filename=filename,
                                   additionalBranches=partBranches)


def generateY4S(noEvents, decayTable, path=analysis_main):
    """
    Generated e+e- -> Y(4S) events with EvtGen event generator.
    The Y(4S) decays according to the user specifed decay table.

    The experiment and run numbers are set to 1.

    If the simulation and reconstruction is not performed in the sam job,
    then the Gearbox needs to be loaded. Use loadGearbox(path) function
    for this purpose.

    @param noEvents   number of events to be generated
    @param decayTable file name of the decay table to be used
    @param path       modules are added to this path
    """

    evtnumbers = register_module('EventInfoSetter')
    evtnumbers.param('evtNumList', [noEvents])
    evtnumbers.param('runList', [1])
    evtnumbers.param('expList', [1])
    evtgeninput = register_module('EvtGenInput')
    evtgeninput.param('userDECFile', decayTable)
    evtgeninput.param('boost2LAB', True)
    path.add_module(evtnumbers)
    path.add_module(evtgeninput)


def generateContinuum(
    noEvents,
    inclusiveP,
    decayTable,
    inclusiveT=2,
    path=analysis_main,
    ):
    """
    Generated e+e- -> gamma* -> qq-bar where light quarks hadronize
    and decay in user specified way (via specified decay table).

    The experiment and run numbers are set to 1.

    If the simulation and reconstruction is not performed in the sam job,
    then the Gearbox needs to be loaded. Use loadGearbox(path) function
    for this purpose.

    @param noEvents   number of events to be generated
    @param inclusiveP each event will contain this particle
    @param decayTable file name of the decay table to be used
    @param inclusiveT whether (2) or not (1) charge conjugated inclusive Particles should be included
    @param path       modules are added to this path
    """

    evtnumbers = register_module('EventInfoSetter')
    evtnumbers.param('evtNumList', [noEvents])
    evtnumbers.param('runList', [1])
    evtnumbers.param('expList', [1])
    evtgeninput = register_module('EvtGenInput')
    evtgeninput.param('userDECFile', decayTable)
    evtgeninput.param('ParentParticle', 'vpho')
    evtgeninput.param('InclusiveParticle', inclusiveP)
    evtgeninput.param('InclusiveType', inclusiveT)
    evtgeninput.param('boost2LAB', True)
    path.add_module(evtnumbers)
    path.add_module(evtgeninput)


def loadGearbox(path=analysis_main):
    """
    Loads Gearbox module to the path.

    This is neccessary in a job with event generation only
    (without reconstruction and reconstruction).

    @param path modules are added to this path
    """

    paramloader = register_module('Gearbox')
    path.add_module(paramloader)


def printPrimaryMCParticles(path=analysis_main):
    """
    Prints all primary MCParticles.
    """

    mcparticleprinter = register_module('PrintMCParticles')
    path.add_module(mcparticleprinter)


def loadMCParticles(path=analysis_main):
    ploader = register_module('ParticleLoader')
    ploader.param('useMCParticles', True)
    path.add_module(ploader)


def loadReconstructedParticles(path=analysis_main):
    ploader = register_module('ParticleLoader')
    ploader.param('useMCParticles', False)
    path.add_module(ploader)


def copyList(
    outputListName,
    inputListName,
    persistent=False,
    path=analysis_main,
    ):

    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', [inputListName])
    pmanipulate.param('persistent', persistent)
    path.add_module(pmanipulate)


def copyLists(
    outputListName,
    inputListNames,
    persistent=False,
    path=analysis_main,
    ):

    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', inputListNames)
    pmanipulate.param('persistent', persistent)
    path.add_module(pmanipulate)


def cutAndCopyLists(
    outputListName,
    inputListNames,
    criteria,
    persistent=False,
    path=analysis_main,
    ):

    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCutAndCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', inputListNames)
    pmanipulate.param('cuts', criteria)
    pmanipulate.param('persistent', persistent)
    path.add_module(pmanipulate)


def cutAndCopyList(
    outputListName,
    inputListName,
    criteria,
    persistent=False,
    path=analysis_main,
    ):

    pmanipulate = register_module('ParticleListManipulator')
    pmanipulate.set_name('PListCutAndCopy_' + outputListName)
    pmanipulate.param('outputListName', outputListName)
    pmanipulate.param('inputListNames', [inputListName])
    pmanipulate.param('cuts', criteria)
    pmanipulate.param('persistent', persistent)
    path.add_module(pmanipulate)


def fillParticleList(
    decayString,
    criteria,
    persistent=False,
    path=analysis_main,
    ):

    pselect = register_module('ParticleSelector')
    pselect.set_name('ParticleSelector_' + decayString)
    pselect.param('decayString', decayString)
    pselect.param('select', criteria)
    pselect.param('persistent', persistent)
    path.add_module(pselect)


def selectParticle(
    decayString,
    criteria=[],
    persistent=False,
    path=analysis_main,
    ):

    pselect = register_module('ParticleSelector')
    pselect.set_name('ParticleSelector_' + decayString)
    pselect.param('decayString', decayString)
    pselect.param('select', criteria)
    pselect.param('persistent', persistent)
    path.add_module(pselect)


def applyCuts(list_name, criteria, path=analysis_main):
    pselect = register_module('ParticleSelector')
    pselect.set_name('ParticleSelector_applyCuts_' + list_name)
    pselect.param('decayString', list_name)
    pselect.param('select', criteria)
    path.add_module(pselect)


def reconDecay(
    decayString,
    cuts,
    persistent=False,
    path=analysis_main,
    ):

    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + decayString)
    pmake.param('decayString', decayString)
    pmake.param('cuts', cuts)
    pmake.param('persistent', persistent)
    path.add_module(pmake)


def makeParticle(
    decayString,
    cuts,
    persistent=False,
    path=analysis_main,
    ):

    pmake = register_module('ParticleCombiner')
    pmake.set_name('ParticleCombiner_' + decayString)
    pmake.param('decayString', decayString)
    pmake.param('cuts', cuts)
    pmake.param('persistent', persistent)
    path.add_module(pmake)


def fitVertex(
    list_name,
    conf_level,
    decay_string='',
    fitter='rave',
    fit_type='vertex',
    constraint='',
    path=analysis_main,
    ):
    """
    Perform the specified kinematic fit for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param decay_string select particles used for the vertex fit
    @param fitter       rave or kfitter
    @param fit_type     type of the kinematic fit (valid options are vertex/massvertex/mass)
    @param constraint   type of additional constraints (valid options are empty string/ipprofile/iptube)
    @param path         modules are added to this path
    """

    if 'Geometry' in [m.name() for m in path.modules()]:
        print '[INFO] fitVertex: Geometry already in path'
    else:
        geometry = register_module('Geometry')
        geometry.param('components', ['MagneticField'])
        path.add_module(geometry)

    pvfit = register_module('ParticleVertexFitter')
    pvfit.set_name('ParticleVertexFitter_' + list_name)
    pvfit.param('listName', list_name)
    pvfit.param('confidenceLevel', conf_level)
    pvfit.param('vertexFitter', fitter)
    pvfit.param('fitType', fit_type)
    pvfit.param('withConstraint', constraint)
    pvfit.param('decayString', decay_string)
    path.add_module(pvfit)


def vertexKFit(
    list_name,
    conf_level,
    decay_string='',
    constraint='',
    path=analysis_main,
    ):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param constraint   add aditional constraint to the fit (valid options are ipprofile or iptube)
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'kfitter',
        'vertex',
        constraint,
        path,
        )


def massVertexKFit(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
    ):
    """
    Perform mass-constrained vertex fit using the kfitter for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'kfitter',
        'massvertex',
        '',
        path,
        )


def massKFit(
    list_name,
    conf_level,
    decay_string='',
    path=analysis_main,
    ):
    """
    Perform vertex fit using the kfitter for each Particle in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param conf_level   minimum value of the confidence level to accept the fit
    @param path         modules are added to this path
    @param decay_string select particles used for the vertex fit
    """

    fitVertex(
        list_name,
        conf_level,
        decay_string,
        'kfitter',
        'mass',
        '',
        path,
        )


def printDataStore(path=analysis_main):
    """
    Prints the contents of DataStore in each event,
    listing all objects and arrays (including size).

    @param path   modules are added to this path
    """

    printDS = register_module('PrintCollections')
    path.add_module(printDS)


def printVariableValues(list_name, var_names, path=analysis_main):
    prlist = register_module('ParticlePrinter')
    prlist.set_name('ParticlePrinter_' + list_name)
    prlist.param('listName', list_name)
    prlist.param('fullPrint', False)
    prlist.param('variables', var_names)
    path.add_module(prlist)


def printList(list_name, full, path=analysis_main):
    prlist = register_module('ParticlePrinter')
    prlist.set_name('ParticlePrinter_' + list_name)
    prlist.param('listName', list_name)
    prlist.param('fullPrint', full)
    path.add_module(prlist)


def ntupleFile(file_name, path=analysis_main):
    ntmaker = register_module('NtupleMaker')
    ntmaker.set_name('NtupleMaker_ntupleFile_' + file_name)
    ntmaker.param('fileName', file_name)
    path.add_module(ntmaker)


def ntupleTree(
    tree_name,
    list_name,
    tools,
    path=analysis_main,
    ):

    ntmaker = register_module('NtupleMaker')
    ntmaker.set_name('NtupleMaker_ntupleTree_' + list_name)
    ntmaker.param('treeName', tree_name)
    ntmaker.param('listName', list_name)
    ntmaker.param('tools', tools)
    path.add_module(ntmaker)


def findMCDecay(
    list_name,
    decay,
    persistent=False,
    path=analysis_main,
    ):

    decayfinder = register_module('MCDecayFinder')
    decayfinder.set_name('MCDecayFinder_' + list_name)
    decayfinder.param('listName', list_name)
    decayfinder.param('decayString', decay)
    decayfinder.param('persistent', persistent)
    path.add_module(decayfinder)


def summaryOfLists(particleLists, path=analysis_main):
    particleStats = register_module('ParticleStats')
    particleStats.param('particleLists', particleLists)
    path.add_module(particleStats)


def matchMCTruth(list_name, path=analysis_main):
    """
    Performs MC matching (sets relation Particle<->MCParticle) for
    all particles (and its (grand)^N-daughter particles) in the specified
    ParticleList.

    @param list_name name of the input ParticleList
    @param path      modules are added to this path
    """

    mcMatch = register_module('MCMatching')
    mcMatch.set_name('MCMatching_' + list_name)
    mcMatch.param('listName', list_name)
    path.add_module(mcMatch)


def buildRestOfEvent(list_name, only_good_ecl=True, path=analysis_main):
    """
    Creates for each Particle in the given ParticleList a RestOfEvent
    dataobject and makes BASF2 relation between them.

    @param list_name name of the input ParticleList
    @param path      modules are added to this path
    """

    roeBuilder = register_module('RestOfEventBuilder')
    roeBuilder.set_name('ROEBuilder_' + list_name)
    roeBuilder.param('particleList', list_name)
    roeBuilder.param('onlyGoodECLClusters', only_good_ecl)
    path.add_module(roeBuilder)


def TagV(
    list_name,
    confidenceLevel,
    MCassociation=True,
    useConstraint='boostcut',
    path=analysis_main,
    ):
    """
    For each Particle in the given Breco ParticleList:
    perform the fit of tag side using the track list from the RestOfEvent dataobject
    save the MC Btag in case of signal MC

    @param list_name name of the input Breco ParticleList
    @param confidenceLevel minimum value of the ConfidenceLevel to accept the fit
    @param MCassociation: use standard MC association or the internal one
    @param useConstraint: choose constraint for the tag vertes fit
    @param path      modules are added to this path
    """

    if 'Geometry' in [m.name() for m in path.modules()]:
        print '[INFO] TAgV: Geometry already in path'
    else:
        geometry = register_module('Geometry')
        geometry.param('components', ['MagneticField'])
        path.add_module(geometry)

    tvfit = register_module('TagVertex')
    tvfit.set_name('TagVertex_' + list_name)
    tvfit.param('listName', list_name)
    tvfit.param('confidenceLevel', confidenceLevel)
    tvfit.param('MCAssociation', MCassociation)
    tvfit.param('useConstraint', useConstraint)
    path.add_module(tvfit)


def buildContinuumSuppression(list_name, path=analysis_main):
    """
    Creates for each Particle in the given ParticleList a ContinuumSuppression
    dataobject and makes BASF2 relation between them.

    @param list_name name of the input ParticleList
    @param path      modules are added to this path
    """

    qqBuilder = register_module('ContinuumSuppressionBuilder')
    qqBuilder.set_name('QQBuilder_' + list_name)
    qqBuilder.param('particleList', list_name)
    path.add_module(qqBuilder)


def FlavTag(list_name, path=analysis_main):
    """
    For each Particle in the given Breco ParticleList:
    Tag the flavour of the tag side using the Track, the ECLCluster and the KLMCluster list from the RestOfEvent dataobject 
    The flavour is predicted by trained Neural Networks
    Module under development (not ready for users)
    
    @param list_name name of the input Breco ParticleList
    @param path      modules are added to this path
    """

    flavtag = register_module('FlavorTagging')
    flavtag.set_name('FlavorTagging_' + list_name)
    flavtag.param('usingMode', mode)
    flavtag.param('listName', list_name)
    path.add_module(flavtag)


def calibratePhotonEnergy(list_name, path=analysis_main):
    """
    Performs energy calibration for photons given in the input ParticleList.
    @param list_name name of the input photon ParticleList
    @param path      modules are added to this path
    """

    gammacal = register_module('GammaEnergyCalibration')
    gammacal.param('gammaListName', list_name)
    path.add_module(gammacal)


