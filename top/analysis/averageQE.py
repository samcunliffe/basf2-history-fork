#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
import sys
from ROOT import Belle2
from ROOT import TH1F, TH2F, TFile, TProfile

# --------------------------------------------------------------------
# a tool to calculate average quantum and collection efficiency from
# Nagoya QA data stored in central database, payload TOPPmtQEs
#
# usage: basf2 averageQE.py [globalTag]
# --------------------------------------------------------------------


class AverageQE(Module):
    ''' determination of average quantum and collection efficiency from TOPPmtQEs '''

    def initialize(self):
        ''' run the procedure, store the results in a root file and print in xml format '''

        dbarray = Belle2.PyDBArray('TOPPmtQEs')
        if dbarray.getEntries() == 0:
            B2ERROR("No entries in TOPPmtQEs")
            return

        geo = Belle2.PyDBObj('TOPGeometry')
        if not geo:
            B2ERROR("No TOPGeometry")
            return

        lambdaStep = dbarray[0].getLambdaStep()
        lambdaFirst = dbarray[0].getLambdaFirst()
        lambdaLast = dbarray[0].getLambdaLast()
        n = int((lambdaLast - lambdaFirst) / lambdaStep) + 1
        print(lambdaFirst, lambdaLast, lambdaStep, n)
        qe_2d = TH2F("qe_2d", "quantum times collection efficiencies",
                     n, lambdaFirst - lambdaStep / 2, lambdaLast + lambdaStep / 2,
                     100, 0.0, 0.3)
        qe_prof = TProfile("qe_prof", "quantum times collection efficiencies",
                           n, lambdaFirst - lambdaStep / 2, lambdaLast + lambdaStep / 2,
                           0.0, 1.0)
        qe_nom = TH1F("qe_nom", "nominal quantum times collection efficiency",
                      n, lambdaFirst - lambdaStep / 2, lambdaLast + lambdaStep / 2)
        ce_1d = TH1F("ce_1d", "collection efficiencies", 100, 0.0, 1.0)

        ce = 0
        for pmtQE in dbarray:
            ce_1d.Fill(pmtQE.getCE(True))
            ce += pmtQE.getCE(True)
            for pmtPixel in range(1, 17):
                for i in range(n):
                    lam = lambdaFirst + lambdaStep * i
                    effi = pmtQE.getEfficiency(pmtPixel, lam, True)
                    qe_2d.Fill(lam, effi)
                    qe_prof.Fill(lam, effi)

        nominalQE = geo.getNominalQE()
        for i in range(n):
            lam = lambdaFirst + lambdaStep * i
            effi = nominalQE.getEfficiency(lam)
            qe_nom.SetBinContent(i+1, effi)

        ce /= dbarray.getEntries()
        print('average CE =', ce)
        file = TFile('averageQE.root', 'recreate')
        qe_2d.Write()
        qe_prof.Write()
        qe_nom.Write()
        ce_1d.Write()
        file.Close()

        print()
        print('<ColEffi descr="average over all PMTs">' + str(round(ce, 4)) + '</ColEffi>')
        print('<LambdaFirst unit="nm">' + str(lambdaFirst) + '</LambdaFirst>')
        print('<LambdaStep unit="nm">' + str(lambdaStep) + '</LambdaStep>')
        for i in range(n):
            qe = qe_prof.GetBinContent(i+1) / ce
            print('<Qeffi>' + str(round(qe, 3)) + '</Qeffi>')
        print()


# Central database
argvs = sys.argv
if len(argvs) == 2:
    use_central_database(argvs[1])

# Create path
main = create_path()

# Set number of events to generate
eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [1]})
main.add_module(eventinfosetter)

# Run the procedure
main.add_module(AverageQE())

# Process events
process(main)
