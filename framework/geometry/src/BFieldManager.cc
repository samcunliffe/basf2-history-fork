/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/geometry/BFieldManager.h>

using namespace Belle2;

BFieldManager& BFieldManager::getInstance()
{
  static BFieldManager instance;
  return instance;
}
