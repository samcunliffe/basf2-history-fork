/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: bjoern.spruck@belle2.org                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/PXDClusterPropFilterModule.h>
#include <map>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDClusterPropFilter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDClusterPropFilterModule::PXDClusterPropFilterModule() : Module()
{
  // Set module properties
  setDescription("The module produce a StoreArray of PXDClusters with specific cuts on properties.");

  // Parameter definitions
  addParam("PXDClustersName", m_PXDClustersName, "The name of the StoreArray of PXDClusters to be filtered", std::string(""));
  addParam("PXDClustersName", m_PXDClustersInsideCutsName, "The name of the StoreArray of Filtered PXDClusters inside cuts",
           std::string("PXDClustersIN"));
  addParam("PXDClustersOutsideCutsName", m_PXDClustersOutsideCutsName,
           "The name of the StoreArray of Filtered PXDClusters outside cuts",
           std::string("PXDClustersOUT"));
  addParam("CreateInside", m_CreateInside, "Create the StoreArray of PXD clusters inside of cuts", true);
  addParam("CreateOutside", m_CreateOutside, "Create the StoreArray of PXD clusters outside of cuts", false);
}

void PXDClusterPropFilterModule::initialize()
{

  // We have to change it once the hardware type clusters are well defined
  StoreArray<PXDCluster> PXDClusters(m_PXDClustersName);   /**< The PXDClusters to be filtered */
  PXDClusters.isRequired();

  if (m_CreateInside) {
    m_selectorIN.registerSubset(PXDClusters, m_PXDClustersInsideCutsName);
    m_selectorIN.inheritAllRelations();
  }
  if (m_CreateOutside) {
    m_selectorOUT.registerSubset(PXDClusters, m_PXDClustersOutsideCutsName);
    m_selectorOUT.inheritAllRelations();
  }
}

bool PXDClusterPropFilterModule::CheckCuts(const PXDCluster& thePXDCluster)
{
  return true;
  return false;
}

void PXDClusterPropFilterModule::event()
{
  // Perform filtering
  StoreArray<PXDCluster> PXDClusters(m_PXDClustersName);   /**< The PXDClusters to be filtered */

  if (m_CreateInside) {
    m_selectorIN.select([this](const PXDCluster * thePxdCluster) {
      return CheckCuts(*thePxdCluster); // the cluster has interesting Properties.
    });
  }

  if (m_CreateOutside) {
    m_selectorOUT.select([this](const PXDCluster * thePxdCluster) {
      return ! CheckCuts(*thePxdCluster); // the cluster has NO interesting Properties.
    });
  }
}
