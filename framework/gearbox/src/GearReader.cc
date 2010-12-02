/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/GearReader.h>

#include <framework/gearbox/MaterialProperty.h>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <TGeoManager.h>

using namespace Belle2;
using namespace std;


TGeoMaterial* GearReader::readMaterial(GearDir& gearDir)
{
  TGeoMaterial* geomMat = NULL;
  double weight = 0;

  string nodeName = gearDir.getNodeName();
  gearDir.convertNodeToPath();

  //Check if the GearDir points to a <Material> or <Mixture> node
  if (nodeName == "Mixture") {
    geomMat = readMixtureSection(gearDir, weight);
  } else if (nodeName == "Material") {
    geomMat = readMaterialSection(gearDir, weight);
  } else {
    B2ERROR("The specified Material is neither a <Material> nor a <Mixture> section (" << gearDir.getDirPath() << ") !");
  }

  gearDir.convertPathToNode();
  return geomMat;
}


//=========================================================
//                  Protected methods
//=========================================================

TGeoElement* GearReader::readElementSection(GearDir& elementContent, double& weight)
{
  weight = readWeightAttribute(elementContent);
  string matName = readNameAttribute(elementContent);

  if (matName.empty()) {
    B2ERROR("The <Element> specified by '" << elementContent.getDirPath() << "' has no name !");
    return NULL;
  }

  int    atomNumber = static_cast<int>(elementContent.getParamNumValue("AtomNumber"));
  double massNumber = elementContent.getParamNumValue("MassNumber");

  TGeoElement* geoElem = new TGeoElement(matName.c_str(), matName.c_str(), atomNumber, massNumber);
  return geoElem;
}


TGeoMaterial* GearReader::readMaterialSection(GearDir& materialContent, double& weight)
{
  weight = readWeightAttribute(materialContent);
  string matName = readNameAttribute(materialContent);

  if (matName.empty()) {
    B2ERROR("The <Material> specified by '" << materialContent.getDirPath() << "' has no name !");
    return NULL;
  }

  TGeoMaterial* geoMat = NULL;

  //Check if the first parameter is available, if not try to get the material from gGeoManager
  if (materialContent.isParamAvailable("AtomNumber")) {

    double atomNumber  = materialContent.getParamNumValue("AtomNumber");
    double massNumber  = materialContent.getParamNumValue("MassNumber");
    double density     = materialContent.getParamDensity("Density");

    //Build root material
    geoMat = new TGeoMaterial(matName.c_str(), massNumber, atomNumber, density);

    //If the RadLength or InterLength is not given, they are computed by Root using the G3 formula
    if ((materialContent.isParamAvailable("RadLength")) && (materialContent.isParamAvailable("InterLength"))) {
      double radlength   = materialContent.getParamNumValue("RadLength");   //radiation length
      double interlength = materialContent.getParamNumValue("InterLength"); //interaction length
      geoMat->SetRadLen(radlength, interlength);
    }

    //Read material properties (if available)
    if (materialContent.isPathValid("Properties")) {
      GearDir propContent(materialContent, "Properties/");
      geoMat->SetCerenkovProperties(readMaterialProperties(propContent));
    }

  } else {
    geoMat = gGeoManager->GetMaterial(matName.c_str());
    if (geoMat == NULL) B2ERROR("Material " << matName << " could not be found in gGeoManager !")
    }

  return geoMat;
}


TGeoMixture* GearReader::readMixtureSection(GearDir& mixtureContent, double& weight)
{
  weight = readWeightAttribute(mixtureContent);
  string matName = readNameAttribute(mixtureContent);

  if (matName.empty()) {
    B2ERROR("The <Mixture> specified by '" << mixtureContent.getDirPath() << "' has no name !");
    return NULL;
  }

  TGeoMixture *geoMix = NULL;

  //Check if the mixture consists of elements, if not try to get the mixture from gGeoManager
  if (mixtureContent.isPathValid("Elements")) {

    //1) Get the number of Elements, Materials and Mixtures
    int nElem = 0;
    GearDir elements(mixtureContent);
    if (mixtureContent.isPathValid("Elements/Element")) {
      elements.append("Elements/Element");
      nElem = elements.getNumberNodes();
    }

    int nMat = 0;
    GearDir materials(mixtureContent);
    if (mixtureContent.isPathValid("Elements/Material")) {
      materials.append("Elements/Material");
      nMat = materials.getNumberNodes();
    }

    int nMix = 0;
    GearDir mixtures(mixtureContent);
    if (mixtureContent.isPathValid("Elements/Mixture")) {
      mixtures.append("Elements/Mixture");
      nMix = mixtures.getNumberNodes();
    }

    //2) Create Mixture
    double localWeight = -1.0;
    geoMix = new TGeoMixture(matName.c_str(), nElem + nMat + nMix);

    //3) Loop over all elements
    for (int iElem = 1; iElem <= nElem; ++iElem) {
      GearDir elemContentIdx(elements, iElem);
      TGeoElement* currElement = readElementSection(elemContentIdx, localWeight);

      if (localWeight < 0.0) {
        B2ERROR("The element " << currElement->GetName() << " has no weight defined. It was not added to the mixture " << geoMix->GetName() << " !")
        continue;
      }
      geoMix->AddElement(currElement, localWeight);
    }

    //4) Loop over all materials
    for (int iMat = 1; iMat <= nMat; ++iMat) {
      GearDir matContentIdx(materials, iMat);
      TGeoMaterial* currMaterial = readMaterialSection(matContentIdx, localWeight);

      if (localWeight < 0.0) {
        B2ERROR("The material " << currMaterial->GetName() << " has no weight defined. It was not added to the mixture " << geoMix->GetName() << " !")
        continue;
      }
      geoMix->AddElement(currMaterial, localWeight);
    }

    //5) Loop over all mixtures
    for (int iMix = 1; iMix <= nMix; ++iMix) {
      GearDir mixContentIdx(mixtures, iMix);
      TGeoMixture* currMixture = readMixtureSection(mixContentIdx, localWeight);

      if (localWeight < 0.0) {
        B2ERROR("The mixture " << currMixture->GetName() << " has no weight defined. It was not added to the mixture " << geoMix->GetName() << " !")
        continue;
      }
      geoMix->AddElement(currMixture, localWeight);
    }

    //6) Read material properties (if available)
    if (mixtureContent.isPathValid("Properties")) {
      GearDir propContent(mixtureContent, "Properties/");
      geoMix->SetCerenkovProperties(readMaterialProperties(propContent));
    }

    //7) Finalize
    double density = mixtureContent.getParamDensity("Density");
    geoMix->SetDensity(density);

  } else {
    geoMix = dynamic_cast<TGeoMixture*>(gGeoManager->GetMaterial(matName.c_str()));
    if (geoMix == NULL) B2ERROR("Mixture " << matName << " could not be found in gGeoManager !")
    }

  return geoMix;
}


MaterialPropertyList* GearReader::readMaterialProperties(GearDir& propertyContent)
{
  MaterialPropertyList* propListResult = NULL;

  //Get the number of properties
  int nProp = 0;
  GearDir properties(propertyContent);
  if (properties.isPathValid("Property")) {
    properties.append("Property");
    nProp = properties.getNumberNodes();
  }

  //If there is at least one property defined, create the property list
  if (nProp > 0) {
    propListResult = new MaterialPropertyList();

    //Loop over all properties
    for (int iProp = 1; iProp <= nProp; ++iProp) {
      GearDir propContentIdx(properties, iProp);

      //Read the property attributes
      string propName = readNameAttribute(propContentIdx);
      string unitName = readUnitAttribute(propContentIdx);

      //Check if a name was specified
      if (propName.empty()) {
        B2ERROR("The property '" << propContentIdx.getDirPath() << "' has no name. The property was skipped !")
        continue;
      }

      //Get the number of the values
      int nValue = 0;
      GearDir values(propContentIdx);
      if (values.isPathValid("value")) {
        values.append("value");
        nValue = values.getNumberNodes();
      }

      //If the property has at least one value, create it
      if (nValue > 0) {
        MaterialProperty& currProperty = propListResult->addProperty(propName);

        //Loop over all values
        for (int iValue = 1; iValue <= nValue; ++iValue) {
          GearDir valueContentIdx(values, iValue);

          //Read energy attribute, If it is not available, skip the value
          if (valueContentIdx.isParamAvailable("attribute::energy")) {
            double currEnergy = valueContentIdx.getParamNumValue("attribute::energy");
            valueContentIdx.convertPathToNode();
            double currValue = valueContentIdx.getParamNumValue();

            //Add a new value to the property
            currProperty.addValue(currEnergy, currValue);
          } else {
            B2ERROR("The value '" << valueContentIdx.getDirPath() << "' has no energy defined. The value was skipped !")
            continue;
          }
        }
      }
    }
  }
  return propListResult;
}


double GearReader::readWeightAttribute(GearDir& gearDir)
{
  //Check if a weight attribute is defined. If it is, read its value.
  if (gearDir.isParamAvailable("attribute::weight")) {
    return gearDir.getParamNumValue("attribute::weight");
  } else return -1.0;
}


string GearReader::readNameAttribute(GearDir& gearDir)
{
  //Check if a name attribute is defined. If it is, read its value.
  if (gearDir.isParamAvailable("attribute::name")) {
    return gearDir.getParamString("attribute::name");
  } else return string("");
}


string GearReader::readUnitAttribute(GearDir& gearDir)
{
  //Check if a unit attribute is defined. If it is, read its value.
  if (gearDir.isParamAvailable("attribute::unit")) {
    return gearDir.getParamString("attribute::unit");
  } else return string("");
}
