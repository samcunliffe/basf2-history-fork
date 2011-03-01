/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOREADER_H_
#define GEOREADER_H_

#include <geometry/dataobjects/MaterialPropertyList.h>

#include <TGeoMaterial.h>
#include <string>

namespace Belle2 {

  class GearDir;

  /**
   * The GeoReader class.
   *
   * Provides static methods to read special geometry objects from a GearDir.
   */
  class GeoReader {

  public:

    /**
     * Reads a material definition section and returns a pointer to the created TGeoMaterial object.
     * The material section can be either a <Material> or a <Mixture>.
     *
     * The following definitions for a <Material> are supported:
     * <Material name="">
     *   <Density unit="g/cm3"></Density>
     *   <RadLength></RadLength>
     *   <InterLength></InterLength>
     *   <AtomNumber> </AtomNumber>
     *   <MassNumber> </MassNumber>
     * </Material>
     *
     * <Material name="">
     *   <Density unit="g/cm3"></Density>
     *   <AtomNumber> </AtomNumber>
     *   <MassNumber> </MassNumber>
     * </Material>
     *
     * A Mixture consist of elements and a weight factor for each of them.
     * These elements can be an arbitrary combination of Elements, Materials
     * or other Mixtures. The basic structure is:
     * <Mixture name="">
     *   <Density unit="g/cm3"></Density>
     *   <Elements>
     *     add here the elements of the mixture.
     *   </Elements>
     * </Mixture>
     *
     * The elements of a mixture can be
     * 1) Elements
     *    <Element name="" weight="">
     *      <AtomNumber></AtomNumber>
     *      <MassNumber></MassNumber>
     *    </Element>
     *
     * 2) Materials
     *    Both material definitions given above are supported.
     *
     * 3) Mixtures
     *    As described here.
     *
     * 4) Existing Materials or Mixtures
     *    <Material name="" weight=""/>
     *    or
     *    <Mixture name="" weight=""/>
     *
     * Both, materials and mixtures, can have an optional material properties section.
     * The values are used to fill the G4MaterialPropertiesTable for the material in Geant4.
     * <Properties>
     *   <Property name="" unit="eV">
     *     <value energy=""> </value>
     *     <value energy=""> </value>
     *   </Property>
     *   <Property name="" unit="eV">
     *     <value energy=""> </value>
     *     <value energy=""> </value>
     *   </Property>
     * </Properties>
     *
     * @param gearDir Reference to a GearDir pointing to the node representing a TGeoMaterial or TGeoMixture section. Returns NULL if the TGeoMaterial could not be created.
     */
    static TGeoMaterial* readMaterial(GearDir& gearDir);


  protected:

    /**
     * Reads an element section.
     *
     * @param elementContent The GearDir pointing to the element section.
     * @param weight Returns a weight which is meant for usage inside a material mixture. If it isn't defined -1 is returned.
     * @return Pointer to the created TGeoElement. Ownership is given to the method receiving the pointer. Returns NULL if the TGeoMaterial could not be created.
     */
    static TGeoElement* readElementSection(GearDir& elementContent, double& weight);

    /**
     * Reads a material section.
     *
     * @param materialContent The GearDir pointing to the material section.
     * @param weight Returns a weight which is meant for usage inside a material mixture. If it isn't defined -1 is returned.
     * @return Pointer to the created TGeoMaterial. Ownership is given to the method receiving the pointer. Returns NULL if the TGeoMaterial could not be created.
     */
    static TGeoMaterial* readMaterialSection(GearDir& materialContent, double& weight);

    /**
     * Reads a mixture section.
     *
     * @param mixtureContent The GearDir pointing to the mixture section.
     * @param weight Returns a weight which is meant for usage inside a material mixture. If it isn't defined -1 is returned.
     * @return Pointer to the created TGeoMixture. Ownership is given to the method receiving the pointer. Returns NULL if the TGeoMaterial could not be created.
     */
    static TGeoMixture* readMixtureSection(GearDir& mixtureContent, double& weight);

    /**
     * Reads a material property section.
     *
     * @param propertyContent The GearDir pointing to the material property section.
     * @return Pointer to the created MaterialPropertyList object. Ownership is given to the method receiving the pointer. Returns NULL if the MaterialProperties could not be created.
     */
    static MaterialPropertyList* readMaterialProperties(GearDir& propertyContent);

  private:

  };

}

#endif /* GEOREADER_H_ */
