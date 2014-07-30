#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Thomas Keck 2014

from FullEventInterpretation import *
from basf2 import *

pid_variables = ['eid', 'eid_dEdx', 'eid_TOP', 'eid_ARICH', 'eid_ECL',
                 'Kid', 'Kid_dEdx', 'Kid_TOP', 'Kid_ARICH',
                 'prid', 'prid_dEdx', 'prid_TOP', 'prid_ARICH',
                 'muid', 'muid_dEdx', 'muid_TOP', 'muid_ARICH']
kinematic_variables = ['p', 'pt', 'p_CMS', 'pt_CMS', 'M', 'E', 'px', 'py', 'pz', 'px_CMS', 'py_CMS', 'pz_CMS']
vertex_variables = ['dr', 'dz', 'dx', 'dy', 'chiProb', 'cosAngleBetweenMomentumAndVertexVector', 'significanceOfDistance', 'distance']

particles = []

################### Charged FSP ###############################
mva_chargedFSP = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=pid_variables + kinematic_variables + ['chiProb'],
    target='isSignal', targetCluster=1
)

particles.append(Particle('pi+', mva_chargedFSP))
particles.append(Particle('e+', mva_chargedFSP))
particles.append(Particle('mu+', mva_chargedFSP))
particles.append(Particle('K+', mva_chargedFSP))

################## GAMMA ############################

mva_gamma = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=kinematic_variables + ['clusterReg', 'goodGamma', 'goodGammaUnCal', 'clusterNHits', 'clusterTrackMatch', 'clusterE9E25'],
    target='isSignal', targetCluster=1
)

particles.append(Particle('gamma', mva_gamma))


################## PI0 ###############################
mva_pi0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=kinematic_variables + ['daughterAngle'],
    target='isSignal', targetCluster=1
)

pre_pi0 = Particle.PreCutConfiguration(
    variable='M',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

pre_pi0_incomplete = Particle.PreCutConfiguration(
    variable='daughterProductOf(getExtraInfo(SignalProbability))',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

particles.append(Particle('pi0', mva_pi0, pre_pi0).addChannel(['gamma', 'gamma']))
particles.append(Particle('pi0:incomplete', mva_pi0, pre_pi0_incomplete).addChannel(['gamma']))

################### KS0 ###############################
mva_KS0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=kinematic_variables + vertex_variables + ['daughterAngle'],
    target='isSignal', targetCluster=1
)

mva_KS0_pi0pi0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['p', 'pt', 'M', 'E', 'daughterAngle'],
    target='isSignal', targetCluster=1
)

pre_KS0 = Particle.PreCutConfiguration(
    variable='M',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('K_S0', mva_KS0, pre_KS0)
p.addChannel(['pi+', 'pi-'])
p.addChannel(['pi0', 'pi0'], mva_KS0_pi0pi0)
particles.append(p)

######################## D0 #########################

mva_D0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_D0 = Particle.PreCutConfiguration(
    variable='M',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

pre_D0_semileptonic = Particle.PreCutConfiguration(
    variable='daughterProductOf(getExtraInfo(SignalProbability))',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('D0', mva_D0, pre_D0)
p.addChannel(['K-', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi0'])
p.addChannel(['K-', 'pi+', 'pi0', 'pi0'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi-', 'pi0'])
p.addChannel(['pi-', 'pi+'])
p.addChannel(['pi-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['pi-', 'pi+', 'pi0'])
p.addChannel(['pi-', 'pi+', 'pi0', 'pi0'])
p.addChannel(['K_S0', 'pi0'])
p.addChannel(['K_S0', 'pi+', 'pi-'])
p.addChannel(['K_S0', 'pi+', 'pi-', 'pi0'])
p.addChannel(['K-', 'K+'])
p.addChannel(['K-', 'K+', 'pi0'])
p.addChannel(['K-', 'K+', 'K_S0'])
particles.append(p)

p = Particle('D0:semileptonic', mva_D0, pre_D0_semileptonic)
p.addChannel(['K-', 'e+'])
p.addChannel(['K-', 'pi0', 'e+'])
p.addChannel(['K-', 'mu+'])
p.addChannel(['K-', 'pi0', 'mu+'])
p.addChannel(['K_S0', 'pi-', 'e+'])
p.addChannel(['K_S0', 'pi-', 'mu+'])
particles.append(p)

######################## D+ #########################

mva_DPlus = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_DPlus = Particle.PreCutConfiguration(
    variable='M',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

pre_DPlus_semileptonic = Particle.PreCutConfiguration(
    variable='daughterProductOf(getExtraInfo(SignalProbability))',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('D+', mva_DPlus, pre_DPlus)
p.addChannel(['K-', 'pi+', 'pi+'])
p.addChannel(['K-', 'pi+', 'pi+', 'pi0'])
p.addChannel(['K-', 'K+', 'pi+'])
p.addChannel(['K-', 'K+', 'pi+', 'pi0'])
p.addChannel(['pi+', 'pi0'])
p.addChannel(['pi+', 'pi+', 'pi-'])
p.addChannel(['pi+', 'pi+', 'pi-', 'pi0'])
p.addChannel(['K_S0', 'pi+'])
p.addChannel(['K_S0', 'pi+', 'pi0'])
p.addChannel(['K_S0', 'pi+', 'pi+', 'pi-'])
p.addChannel(['K+', 'K_S0', 'K_S0'])

particles.append(p)

p = Particle('D+:semileptonic', mva_DPlus, pre_DPlus_semileptonic)
p.addChannel(['K-', 'pi+', 'e+'])
p.addChannel(['K-', 'pi+', 'mu+'])
particles.append(p)

######################## D*+ #########################

mva_DStarPlus = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_DStarPlus = Particle.PreCutConfiguration(
    variable='Q',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

pre_DStarPlus_semileptonic = Particle.PreCutConfiguration(
    variable='daughterProductOf(getExtraInfo(SignalProbability))',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('D*+', mva_DStarPlus, pre_DStarPlus)
p.addChannel(['D0', 'pi+'])
p.addChannel(['D+', 'pi0'])
particles.append(p)

p = Particle('D*+:semileptonic', mva_DStarPlus, pre_DStarPlus_semileptonic)
p.addChannel(['D0:semileptonic', 'pi+'])
p.addChannel(['D+:semileptonic', 'pi0'])
particles.append(p)

######################## D*0 #########################

mva_DStar0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_DStar0 = Particle.PreCutConfiguration(
    variable='Q',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

pre_DStar0_semileptonic = Particle.PreCutConfiguration(
    variable='daughterProductOf(getExtraInfo(SignalProbability))',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('D*0', mva_D0, pre_DStar0)
p.addChannel(['D0', 'pi0'])
p.addChannel(['D0', 'gamma'])
particles.append(p)

p = Particle('D*0:semileptonic', mva_DStar0, pre_DStar0_semileptonic)
p.addChannel(['D0:semileptonic', 'pi0'])
p.addChannel(['D0:semileptonic', 'gamma'])
particles.append(p)

######################## D_s+ #########################

mva_DS = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_DS = Particle.PreCutConfiguration(
    variable='M',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)


p = Particle('D_s+', mva_DS, pre_DS)
p.addChannel(['K+', 'K_S0'])
p.addChannel(['K+', 'pi+', 'pi-'])
p.addChannel(['K+', 'K-', 'pi+'])
p.addChannel(['K+', 'K-', 'pi+', 'pi0'])
p.addChannel(['K+', 'K_S0', 'pi+', 'pi-'])
p.addChannel(['K-', 'K_S0', 'pi+', 'pi+'])
p.addChannel(['K+', 'K-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['pi+', 'pi+', 'pi-'])
p.addChannel(['K_S0', 'pi+'])
p.addChannel(['K_S0', 'pi+', 'pi0'])
particles.append(p)


######################## DStar_s+ #########################

mva_DStarS = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_DStarS = Particle.PreCutConfiguration(
    variable='Q',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('D_s*+', mva_DStarS, pre_DStarS)
p.addChannel(['D_s+', 'gamma'])
p.addChannel(['D_s+', 'pi0'])
particles.append(p)

######################## J/psi #########################

mva_J = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))'] + kinematic_variables + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_J = Particle.PreCutConfiguration(
    variable='M',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('J/psi', mva_J, pre_J)
p.addChannel(['e+', 'e-'])
p.addChannel(['mu+', 'mu-'])
particles.append(p)


################### B+ ##############################

mva_BPlus = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))', 'deltaE'] + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_BPlus = Particle.PreCutConfiguration(
    variable='Mbc',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

pre_BPlus_semileptonic = Particle.PreCutConfiguration(
    variable='daughterProductOf(getExtraInfo(SignalProbability))',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('B+', mva_BPlus, pre_BPlus)
p.addChannel(['anti-D0', 'pi+'])
p.addChannel(['anti-D0', 'pi+', 'pi0'])
p.addChannel(['anti-D0', 'pi+', 'pi+', 'pi-'])
p.addChannel(['D_s+', 'anti-D0'])
p.addChannel(['anti-D*0', 'pi+'])
p.addChannel(['anti-D*0', 'pi+', 'pi0'])
p.addChannel(['anti-D*0', 'pi+', 'pi+', 'pi-'])
p.addChannel(['anti-D*0', 'pi+', 'pi+', 'pi-', 'pi0'])
p.addChannel(['D_s*+', 'anti-D0'])
p.addChannel(['D_s+', 'anti-D*0'])
p.addChannel(['anti-D0', 'K+'])
p.addChannel(['D-', 'pi+', 'pi+'])
p.addChannel(['J/psi', 'K+'])
p.addChannel(['J/psi', 'K+', 'pi+', 'pi-'])
p.addChannel(['J/psi', 'K+', 'pi0'])
p.addChannel(['J/psi', 'K_S0', 'pi+'])
particles.append(p)

p = Particle('B+:semileptonic', mva_BPlus, pre_BPlus_semileptonic)
p.addChannel(['anti-D0', 'e+'])
p.addChannel(['anti-D0', 'mu+'])
p.addChannel(['anti-D0:semileptonic', 'pi+'])
p.addChannel(['anti-D0:semileptonic', 'pi+', 'pi0'])
p.addChannel(['anti-D0:semileptonic', 'K+'])
p.addChannel(['anti-D*0:semileptonic', 'pi+'])
p.addChannel(['anti-D*0:semileptonic', 'pi+', 'pi0'])
p.addChannel(['anti-D*0:semileptonic', 'pi+', 'pi+', 'pi-'])
p.addChannel(['anti-D*0:semileptonic', 'pi+', 'pi+', 'pi-', 'pi0'])
p.addChannel(['D-:semileptonic', 'pi+', 'pi+'])
particles.append(p)

################### B0 ##############################

mva_B0 = Particle.MVAConfiguration(
    name='FastBDT', type='Plugin', config='!H:CreateMVAPdfs:!V:NTrees=100:Shrinkage=0.10:RandRatio=0.5:NCutLevel=8:NTreeLayers=3',
    variables=['daughterProductOf(getExtraInfo(SignalProbability))', 'deltaE'] + vertex_variables,
    target='isSignal', targetCluster=1
)

pre_B0 = Particle.PreCutConfiguration(
    variable='Mbc',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

pre_B0_semileptonic = Particle.PreCutConfiguration(
    variable='daughterProductOf(getExtraInfo(SignalProbability))',
    method='Same',
    efficiency=0.9,
    purity=0.0001,
)

p = Particle('B0', mva_B0, pre_B0)
p.addChannel(['D-', 'pi+'])
p.addChannel(['D-', 'pi0', 'pi+'])
p.addChannel(['D-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['anti-D0', 'pi0'])
p.addChannel(['D_s+', 'D-'])
p.addChannel(['D*-', 'pi+'])
p.addChannel(['D*-', 'pi0', 'pi+'])
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-'])
p.addChannel(['D*-', 'pi+', 'pi+', 'pi-', 'pi0'])
p.addChannel(['D_s*+', 'D-'])
p.addChannel(['D_s+', 'D*-'])
p.addChannel(['D_s*+', 'D*-'])
p.addChannel(['J/psi', 'K_S0'])
p.addChannel(['J/psi', 'K+', 'pi-'])
p.addChannel(['J/psi', 'K_S0', 'pi+', 'pi-'])
particles.append(p)

p = Particle('B0:semileptonic', mva_B0, pre_B0)
p.addChannel(['D-', 'e+'])
p.addChannel(['D-', 'mu+'])
p.addChannel(['anti-D0:semileptonic', 'pi0'])
p.addChannel(['D-:semileptonic', 'pi+'])
p.addChannel(['D-:semileptonic', 'pi0', 'pi+'])
p.addChannel(['D-:semileptonic', 'pi+', 'pi+', 'pi-'])
p.addChannel(['D*-:semileptonic', 'pi+'])
p.addChannel(['D*-:semileptonic', 'pi0', 'pi+'])
p.addChannel(['D*-:semileptonic', 'pi+', 'pi+', 'pi-'])
p.addChannel(['D*-:semileptonic', 'pi+', 'pi+', 'pi-', 'pi0'])
particles.append(p)


main = create_path()
main.add_module(register_module('RootInput'))
main.add_module(register_module('ParticleLoader'))

FullEventInterpretation(main, particles)

main.add_module(register_module('ProgressBar'))

# show constructed path
print main

process(main)
B2WARNING('event() statistics:')
print statistics
B2WARNING('endRun() statistics:')
print statistics(statistics.END_RUN)
