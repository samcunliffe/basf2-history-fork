/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/eklmhit/EKLMSimHit.h>
#include <framework/logging/Logger.h>
#include<iostream>
#include<fstream>

using namespace Belle2;

ClassImp(Belle2::EKLMSimHit)

G4Allocator<EKLMSimHit> EKLMSimHitAllocator;

void EKLMSimHit::Save(char* filename)
{
  std::ofstream save_hit(filename, std::fstream::app);
  save_hit << '\n';
  save_hit << "EKLM Hit: \n" ;
  save_hit << "Global position: " << m_global_pos << '\n'  ;
  save_hit << "Local position: " << m_local_pos << '\n'  ;
  save_hit << "Time: " << m_time << '\n' ;
  save_hit << "Energy Deposition: " <<  m_eDep << '\n' ;
  save_hit << "PDG code: " << m_PDGcode << '\n';
  save_hit.close();
}



//} //end of Belle2 namespace
