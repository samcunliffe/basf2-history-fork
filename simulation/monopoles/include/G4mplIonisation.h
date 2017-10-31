//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// modified from exoticphysics/monopole/*

/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef G4MPLIONISATION_H
#define G4MPLIONISATION_H

#include <G4VEnergyLossProcess.hh>
#include <globals.hh>
#include <G4VEmModel.hh>

class G4Material;
class G4VEmFluctuationModel;

namespace Belle2 {

  namespace Monopoles {

    class G4mplIonisation : public G4VEnergyLossProcess {

    public:

      explicit G4mplIonisation(G4double mCharge = 0.0,
                               const G4String& name = "mplIoni");

      virtual ~G4mplIonisation();

      virtual G4bool IsApplicable(const G4ParticleDefinition& p) override;

      // Print out of the class parameters
      virtual void PrintInfo() override;

    protected:

      virtual void InitialiseEnergyLossProcess(const G4ParticleDefinition*,
                                               const G4ParticleDefinition*) override;

    private:

      // hide assignment operator
      G4mplIonisation& operator=(const G4mplIonisation& right) = delete;
      G4mplIonisation(const G4mplIonisation&) = delete;

      G4double    magneticCharge;
      G4bool      isInitialised;

    };

  } //end Monopoles namespace

} //end Belle2 namespace
#endif
