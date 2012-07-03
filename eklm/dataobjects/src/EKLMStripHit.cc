/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <G4Box.hh>

#include <eklm/dataobjects/EKLMStripHit.h>
//#include <eklm/geoeklm/G4PVPlacementGT.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/GearDir.h>

using namespace std;
using namespace Belle2;


ClassImp(Belle2::EKLMStripHit);





EKLMStripHit::EKLMStripHit(const EKLMSimHit* hit)
  : EKLMHitBase((EKLMHitBase)(*hit)),
    m_Plane(hit->getPlane()),
    m_Strip(hit->getStrip()),
    m_NumberPhotoElectrons(-1),
    m_pv(hit->getVolume()),
    m_MCtime(hit->getTime())
{}



double EKLMStripHit::getNumberPhotoElectrons() const
{
  return m_NumberPhotoElectrons;
}

void EKLMStripHit::setNumberPhotoElectrons(double npe)
{
  m_NumberPhotoElectrons = npe;
}


bool EKLMStripHit::isGood() const
{
  return m_good;
}

void EKLMStripHit::isGood(bool status)
{
  m_good = status;
}


int EKLMStripHit::getPlane() const
{
  return m_Plane;
}
void EKLMStripHit::setPlane(int plane)
{
  m_Plane = plane;
}
int EKLMStripHit::getStrip() const
{
  return m_Strip;
}
void EKLMStripHit::setStrip(int strip)
{
  m_Strip = strip;
}

const TFitResult* EKLMStripHit::getFitResults() const
{
  return &m_fitResults;
}

void EKLMStripHit::setFitResults(TFitResult& res)
{
  m_fitResults = res;
}

void EKLMStripHit::setFitResults(TFitResultPtr resPtr)
{
  m_fitResults = *resPtr;
}

void EKLMStripHit::setFitStatus(int s)
{
  m_fitStatus = s;
}

int EKLMStripHit::getFitStatus()
{
  return m_fitStatus;
}



const G4VPhysicalVolume* EKLMStripHit::getVolume() const
{
  return m_pv;
}

void EKLMStripHit::setVolume(const G4VPhysicalVolume* pv)
{
  m_pv = pv;
}



void EKLMStripHit::setMCTS(double ts)
{m_timeshift = ts;}

double EKLMStripHit::getMCTS() const
{return m_timeshift;}

void EKLMStripHit::Print() const
{
  std::cout << "Endcap: " << getEndcap()
            << " Layer: " << getLayer()
            << " Sector: " << getSector()
            << " Plane: " << getPlane()
            << " Strip: " << getStrip()
            << " # Time: " << m_Time << "\n";
}
