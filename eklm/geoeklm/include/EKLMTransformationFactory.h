/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Timofey Uglov                                           *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/

#ifndef EKLMTTRANSFORMATIONFACTORY_H_
#define EKLMTTRANSFORMATIONFACTORY_H_

#include <G4Transform3D.hh>
#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>


namespace Belle2 {

  class EKLMTransformationFactory {

  public:
    static EKLMTransformationFactory* getInstance();

    //! adds strip transformation matrix entry
    void addMatrixEntry(int , int, int, int , int, G4Transform3D);
    //! adds strip length entry
    void addLengthEntry(int , double);

    //! returns transformation matrix for the strip
    //! [#endcap][#layer][#sector][#plane][#strip]
    G4Transform3D getTransformation(int , int , int , int , int);

    //! returns length of the  the strip
    double getStripLength(int);

    //! reads information from file to memory
    void readFromXMLFile(const GearDir&);

    //! reads information from file to memory
    void readFromFile(const char*);




    //! write collected information  to XML file
    void writeToFile(const char* filename) const;

    //! write collected information  to XML file
    void writeToXMLFile(std::string filename);
    //! write collected information  to XML file
    void writeToXMLFile(char* filename);

    //! clear
    void clear();

  private:
    static EKLMTransformationFactory* EKLMTransformationFactory_instance;

    //constructor
    EKLMTransformationFactory();

    //! G4Transform3D  matrix for the strip
    G4Transform3D stripMatrixArray[2][14][4][2][75]; // [#endcap][#layer][#sector][#plane][#strip]

    //!  the strip length
    double stripLengthArray[75]; // [#strip]


  };



}

#endif

