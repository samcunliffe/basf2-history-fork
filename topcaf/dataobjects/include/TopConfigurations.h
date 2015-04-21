#ifndef TopConfigurations_H
#define TopConfigurations_H

#include <TList.h>
#include <TObject.h>
#include <string>
#include <utility>
#include <vector>

#include "topcaf/utils/iTopTypedef.h" // the itop typedef declarations.

namespace Belle2 {

  /** The top configurations object will read and allow access to itop configurations payloads.
   *
   *
   */
  class TopConfigurations : public TObject {

  public:

    /** Get a pointer to the TopConfigurations instance */
    static TopConfigurations* GetInstance();


    /** Read back a payload associated with a particular global tag and run for a particular algorithm.
     *  The TFile that contains the configurations information must be associated with the private variable m_fileName.
     *
     */
    void ReadTopConfigurations();

    /** Channel Numbering, pixel number to pixel row */
    unsigned int pixel_to_pixelRow(TopPixel pixel) {return m_PixeltoRow[pixel];}
    /** Channel Numbering, pixel number to pixel column */
    unsigned int pixel_to_pixelColumn(TopPixel pixel) {return m_PixeltoColumn[pixel];}

    /** Channel Numbering, pixel number to PMT number */
    unsigned int pixel_to_PMTNumber(TopPixel pixel) {return m_PixeltoPMT[pixel];}
    /** Channel Numbering, pixel number to PMT channel */
    unsigned int pixel_to_channelNumber(TopPixel pixel) {return m_PixeltoPMTChannel[pixel];}

    /** Channel Numbering, pixel number to electronics module */
    TopElectronicModule pixel_to_electronicsModule(TopPixel pixel) {return m_PixeltoElectronicModule[pixel];}
    /** Channel Numbering, pixel number to ASIC row */
    unsigned int pixel_to_asicRow(TopPixel pixel) {return m_PixeltoAsicRow[pixel];}
    /** Channel Numbering, pixel number to ASIC column */
    unsigned int pixel_to_asicColumn(TopPixel pixel) {return m_PixeltoAsicColumn[pixel];}
    /** Channel Numbering, pixel number to ASIC channel */
    unsigned int pixel_to_asicChannel(TopPixel pixel) {return m_PixeltoAsicChannel[pixel];}
    /** Channel Numbering, SCROD number to electronics module */
    TopElectronicModule scrod_to_electronicsModule(unsigned int scrod) {return m_ScrodtoElectronicModule[scrod];}
    /** Channel Numbering, SCROD number to electronics module number */
    unsigned int scrod_to_electronicsModuleNumber(unsigned int scrod) {return m_ScrodtoElectronicModule[scrod].second;}

    /** Channel Numbering, channel number to pixel number */
    unsigned int channelNumber_to_pixelNumber(unsigned int PMT, unsigned int channel)
    {
      return (channel - 1) % 4 + (PMT - 1) * 4 + ((channel - 1) / 4) * 64 + ((PMT - 1) / 16) * 192 + 1;
    }

    /** Channel Numbering, hardware ID to pixel */
    TopPixel hardwareID_to_pixel(unsigned int hardwareID) { return m_HardwareIDtoPixel[hardwareID]; }
    /** Channel Numbering, hardware ID to pixel number */
    unsigned int hardwareID_to_pixelNumber(unsigned int hardwareID) { return m_HardwareIDtoPixel[hardwareID].second; }

  private:
    /** Singleton, so control use of constructors */
    TopConfigurations();

    /** Singleton, so control use of constructors */
    TopConfigurations(TopConfigurations const&);

    /** Singleton, so control use of copy operator */
    void operator=(TopConfigurations const&);

    /** Singleton, so control use of destructor */
    ~TopConfigurations();


    /** The maps for the TOP Numbering Scheme v1.0  https://belle2.cc.kek.jp/~twiki/bin/view/Archive/Belle2note0026 **/
    TopPixelRefMap m_PixeltoRow;
    TopPixelRefMap m_PixeltoColumn;
    TopPixelRefMap m_PixeltoPMT;
    TopPixelRefMap m_PixeltoPMTChannel;
    TopPixelRefMap m_PixeltoScrod;
    TopPixelRefMap m_PixeltoAsicRow;
    TopPixelRefMap m_PixeltoAsicColumn;
    TopPixelRefMap m_PixeltoAsicChannel;
    TopPixelRetMap m_HardwareIDtoPixel;
    TopElectronicRetMap  m_ScrodtoElectronicModule;
    TopPixelRefElectronicRetMap m_PixeltoElectronicModule;
    TopElectronicRefMap  m_ElectronicModuletoScrod;
    std::vector<TopElectronicConstructionName> m_TopModuleElectronicConstructions;

    /** The single instance of this class */
    static TopConfigurations* m_Instance;

    /** The path and filename of the TFile containing the configurations information */
    std::string m_fileName;

    /** A buffer to temporarily get rid of compiler warnings */
    std::string m_buffer;

    ClassDef(TopConfigurations, 1);


  };
}
#endif
