import basf2
from basf2 import *
from beamparameters import add_beamparameters
import os
import sys
import math
import string
import datetime
from background import add_output


argvs = sys.argv
argc = len(argvs)

bgType = argvs[1]     # Coulomb, Touschek, Brems
accRing = argvs[2]    # LER or HER
num = int(argvs[3])   # output file number
folder = argvs[4]

d = datetime.datetime.today()
print(d.strftime('job start: %Y-%m-%d %H:%M:%S\n'))

main = create_path()

fnames = []
patha = "/group/belle2/BGcampaigns/g4sim/" + folder

pathb = patha + '/output/'
pathc = patha + '/beast_hist/'
# takes 10 input files
for i in range(num * 10, (num + 1) * 10):
    filenn = pathb + bgType + '_' + accRing + '_study-phase2-' + str(i) + '.root'
    print(filenn)
    fnames.append(filenn)

histfile = pathc + bgType + '_' + accRing + '_' + str(num) + '.root'

input_module = register_module('RootInput')
input_module.param('inputFileNames', fnames)
main.add_module(input_module)

# Histogram manager module
histo = register_module('HistoManager')
histo.param('histoFileName', histfile)  # File to save histograms
main.add_module(histo)

gearbox = register_module('Gearbox')
gearbox.param('fileName', '/geometry/Beast2_phase2.xml')
main.add_module(gearbox)

main.add_module("Progress")

# set digitizers and study modules parameters according to my best knowledge
# should be examined by beast experts

MIP_to_PE = [27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27]
he3digi = register_module('He3Digitizer')
he3digi.param('conversionFactor', 0.303132019)
he3digi.param('useMCParticles', False)
main.add_module(he3digi)
diadigi = register_module('BeamDigitizer')
diadigi.param('WorkFunction', 13.25)
diadigi.param('FanoFactor', 0.382)
main.add_module(diadigi)
pindigi = register_module('PinDigitizer')
pindigi.param('WorkFunction', 3.64)
pindigi.param('FanoFactor', 0.13)
main.add_module(pindigi)
clawsdigi = register_module('ClawsDigitizer')
clawsdigi.param('C_MIP_to_PE', MIP_to_PE)
main.add_module(clawsdigi)
qcssdigi = register_module('QcsmonitorDigitizer')
qcssdigi.param('ScintCell', 40)
qcssdigi.param('C_keV_to_MIP', 1629.827)
qcssdigi.param('C_MIP_to_PE', 15.0)
qcssdigi.param('MIPthres', 0.5)
main.add_module(qcssdigi)
fangsdigi = register_module('FANGSDigitizer')
main.add_module(fangsdigi)
tpcdigi = register_module('TpcDigitizer')
main.add_module(tpcdigi)

beamab_study = register_module('BeamabortStudy')
main.add_module(beamab_study)

claws_study = register_module('ClawsStudy')
main.add_module(claws_study)

fangs_study = register_module('FANGSStudy')
main.add_module(fangs_study)

tpc_study = register_module('MicrotpcStudy')
main.add_module(tpc_study)

he3_study = register_module('He3tubeStudy')
main.add_module(he3_study)

pin_study = register_module('PindiodeStudy')
# main.add_module(pin_study)

qcs_study = register_module('QcsmonitorStudy')
# main.add_module(qcs_study)

basf2.process(main)

print('Event Statistics:')
print(basf2.statistics)

d = datetime.datetime.today()
print(d.strftime('job finish: %Y-%m-%d %H:%M:%S\n'))
