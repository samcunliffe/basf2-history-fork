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

#ifndef G4MONOPOLEPHYSICS_H
#define G4MONOPOLEPHYSICS_H

#include <G4VPhysicsConstructor.hh>
#include <globals.hh>

namespace Belle2 {

  namespace Monopoles {

    class G4MonopolePhysicsMessenger;
    class G4Monopole;

    class G4MonopolePhysics : public G4VPhysicsConstructor {
    public:

      G4MonopolePhysics(const G4String& nam = "Monopole Physics");

      ~G4MonopolePhysics();

      // This method is dummy for physics
      virtual void ConstructParticle();

      // This method will be invoked in the Construct() method.
      // each physics process will be instantiated and
      // registered to the process manager of each particle type
      virtual void ConstructProcess();

      void SetMagneticCharge(G4double);
      void SetElectricCharge(G4double);
      void SetMonopoleMass(G4double);

    private:

      // hide assignment operator
      G4MonopolePhysics& operator=(const G4MonopolePhysics& right);
      G4MonopolePhysics(const G4MonopolePhysics&);

      G4double    fMagCharge;
      G4double    fElCharge;
      G4double    fMonopoleMass;

      G4MonopolePhysicsMessenger*  fMessenger;
      G4Monopole* fMpl; //Monopole
      G4Monopole* fApl; //Anti_Monopole

    };

  } //end Monopoles namespace

} //end Belle2 namespace
#endif
