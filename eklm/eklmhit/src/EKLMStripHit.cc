/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/eklmhit/EKLMStripHit.h>
#include <framework/logging/Logger.h>


using namespace Belle2;
using namespace std;

ClassImp(EKLMStripHit);

EKLMStripHit::EKLMStripHit(const char * name)
{
  m_Name = name;
}

EKLMStripHit::EKLMStripHit(std::string & name)
{
  m_Name = name;
}

void EKLMStripHit::Print()
{
  std::cout << "Strip Hit: " << m_Name << std::endl;
}



