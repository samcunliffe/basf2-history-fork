/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/MagneticField.h>

#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <globals.hh>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;


MagneticField::MagneticField(): G4MagneticField(), m_bField(BFieldMap::Instance())
{

}


MagneticField::~MagneticField()
{

}


void MagneticField::GetFieldValue(const G4double Point[3], G4double *Bfield) const
{
  //Get the magnetic field vector from the central magnetic field map (Geant4 uses [mm] as a length unit)
  TVector3 magField = m_bField.getBField(TVector3(Point[0] * Unit::mm, Point[1] * Unit::mm, Point[2] * Unit::mm));

  //Set the magnetic field (Use the Geant4 tesla unit here !)
  Bfield[0] = magField[0] * tesla;
  Bfield[1] = magField[1] * tesla;
  Bfield[2] = magField[2] * tesla;
}
