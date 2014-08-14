/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Katsuro Nakamura                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVD_TRANSPARENT_DIGIT_H
#define SVD_TRANSPARENT_DIGIT_H

#include <string>
#include <sstream>

#include <vxd/dataobjects/VxdID.h>
//#include <svd/online/SVDPar.h>

#include <root/TObject.h>

namespace Belle2 {

  /**
   * The SVD digit class.
   *
   * The SVDTransparentDigit is an APV25 signal sample.
   * This is a development implementation which is intentionally kept
   * somewhat bulky. I record strip coordinates that won't be kept in future.
   * Also the sensor and ncell IDs could be somewhat compressed, if desired.
   */

  class SVDTransparentDigit : public TObject {

  public:

    /** Useful Constructor.
     * @param sensorID Sensor compact ID.
     * @param isU True if v strip, false if v.
     * @param cellID Strip ID.
     * @param cellPosition Strip mid-line coordinate.
     * @param charge The charge collected on the strip.
     * @param time Index of the sample, e.g., 0 to 5.
     */
    SVDTransparentDigit(const VxdID sensorID, const bool isU,
                        const unsigned short fadcID,
                        const unsigned short apvID,
                        const unsigned short nSample = 0,
                        const short** adc  = NULL,
                        const short*  cmc1 = NULL,
                        const short*  cmc2 = NULL,
                        const short** corradc = NULL,
                        const short** fine_cmc1 = NULL,
                        const short** fine_cmc2 = NULL,
                        const short** fine_corradc = NULL)
      : m_sensorID(sensorID), m_isU(isU),
        m_fadcID(fadcID), m_apvID(apvID), m_nSample(nSample) {
      if (adc != NULL) {
        for (int channel = 0; channel < 128; channel++) {
          //for(int index=0;index<SVDPar::maxSample;index++) {
          for (int index = 0; index < 6; index++) {
            m_ADC[channel][index] = adc[channel][index];
          }
        }
      } else {
        memset(m_ADC, 0, sizeof(m_ADC));
      }

      if (cmc1 != NULL) {
        //for(int index=0;index<SVDPar::maxSample;index++) {
        for (int index = 0; index < 6; index++) {
          m_cmc1[index] = cmc1[index];
        }
      } else {
        memset(m_cmc1, 0, sizeof(m_cmc1));
      }

      if (cmc2 != NULL) {
        //for(int index=0;index<SVDPar::maxSample;index++) {
        for (int index = 0; index < 6; index++) {
          m_cmc2[index] = cmc2[index];
        }
      } else {
        memset(m_cmc2, 0, sizeof(m_cmc2));
      }

      if (corradc != NULL) {
        for (int channel = 0; channel < 128; channel++) {
          //for(int index=0;index<SVDPar::maxSample;index++) {
          for (int index = 0; index < 6; index++) {
            m_corrADC[channel][index] = corradc[channel][index];
          }
        }
      } else {
        memset(m_corrADC, 0, sizeof(m_corrADC));
      }

      if (fine_cmc1 != NULL) {
        for (int channel = 0; channel < 128; channel++) {
          //for(int index=0;index<SVDPar::maxSample;index++) {
          for (int index = 0; index < 6; index++) {
            m_fine_cmc1[channel][index] = fine_cmc1[channel][index];
          }
        }
      } else {
        memset(m_fine_cmc1, 0, sizeof(m_fine_cmc1));
      }

      if (fine_cmc2 != NULL) {
        for (int channel = 0; channel < 128; channel++) {
          //for(int index=0;index<SVDPar::maxSample;index++) {
          for (int index = 0; index < 6; index++) {
            m_fine_cmc2[channel][index] = fine_cmc2[channel][index];
          }
        }
      } else {
        memset(m_fine_cmc2, 0, sizeof(m_fine_cmc2));
      }

      if (fine_corradc != NULL) {
        for (int channel = 0; channel < 128; channel++) {
          //for(int index=0;index<SVDPar::maxSample;index++) {
          for (int index = 0; index < 6; index++) {
            m_fine_corrADC[channel][index] = fine_corradc[channel][index];
          }
        }
      } else {
        memset(m_fine_corrADC, 0, sizeof(m_fine_corrADC));
      }

    }

    /** Default constructor for the ROOT IO. */
    SVDTransparentDigit()
      : SVDTransparentDigit(0, true, 0, 0, 0, NULL, NULL, NULL, NULL)
    {}

    /** Get the sensor ID.
     * @return ID of the sensor.
     */
    VxdID getSensorID() const { return m_sensorID; }

    /** Get strip direction.
     * @return true if u, false if v.
     */
    bool isUStrip() const { return m_isU; }

    /** Get FADC ID.
     * @return FADC ID value.
     */
    unsigned short getFadcID() const { return m_fadcID; }

    /** Get APV ID.
     * @return APV ID value.
     */
    unsigned short getApvID() const { return m_apvID; }

    /** Get number of samples.
     * @return number of samples (1~6).
     */
    unsigned short getNSample() const { return m_nSample; }

    /** Get ADC value.
     * @return charge collected in the strip.
     */
    const short*  getADC(short channel) const {
      if (channel < 0 || 128 <= channel) return NULL;
      else return m_ADC[channel];
    }
    short getADC(short channel, short index) const {
      //if(index<0||SVDPar::maxSample<=index) return -1;
      if (index < 0 || 6 <= index) return -9999;
      if (channel < 0 || 128 <= channel) return -9999;
      return m_ADC[channel][index];
    }

    /** Get 1st commom-mode correction value.
     * @return 1st commom-mode correction value
     */
    const short* getCMC1() const { return m_cmc1; }
    short  getCMC1(short index) const {
      if (index < 0 || 6 <= index) return -9999;
      return m_cmc1[index];
    }

    /** Get 2nd commom-mode correction value.
     * @return 2nd commom-mode correction value
     */
    const short* getCMC2() const { return m_cmc2; }
    short  getCMC2(short index) const {
      if (index < 0 || 6 <= index) return -9999;
      return m_cmc2[index];
    }

    /** Get total commom-mode correction value.
     * @return total commom-mode correction value
     */
    const short* getCMC() const { return m_cmc; }
    short  getCMC(short index) const {
      if (index < 0 || 6 <= index) return -9999;
      return m_cmc[index] ;
    }

    /** Get 1st fine commom-mode correction value.
     * @return 1st fine commom-mode correction value
     */
    const short*  getFineCMC1(short channel) const {
      if (channel < 0 || 128 <= channel) return NULL;
      return m_fine_cmc1[channel];
    }
    short getFineCMC1(short channel, short index) const {
      //if(index<0||SVDPar::maxSample<=index) return -1;
      if (index < 0 || 6 <= index) return -9999;
      if (channel < 0 || 128 <= channel) return -9999;
      else return m_fine_cmc1[channel][index];
    }
    /** Get 2nd fine commom-mode correction value.
     * @return 2nd fine commom-mode correction value
     */
    const short*  getFineCMC2(short channel) const {
      if (channel < 0 || 128 <= channel) return NULL;
      return m_fine_cmc2[channel];
    }
    short getFineCMC2(short channel, short index) const {
      //if(index<0||SVDPar::maxSample<=index) return -1;
      if (index < 0 || 6 <= index) return -9999;
      if (channel < 0 || 128 <= channel) return -9999;
      else return m_fine_cmc2[channel][index];
    }
    /** Get total fine commom-mode correction value.
     * @return total fine commom-mode correction value
     */
    const short*  getFineCMC(short channel) const {
      if (channel < 0 || 128 <= channel) return NULL;
      return m_fine_cmc[channel];
    }
    short getFineCMC(short channel, short index) const {
      //if(index<0||SVDPar::maxSample<=index) return -1;
      if (index < 0 || 6 <= index) return -9999;
      if (channel < 0 || 128 <= channel) return -9999;
      else return m_fine_cmc[channel][index];
    }

    /** Get corrected ADC value with pedestal and common-mode.
     * @return ADC value corrected with pedestal and common-mode
     */
    const short*  getCorrADC(short channel) const {
      if (channel < 0 || 128 <= channel) return NULL;
      return m_corrADC[channel];
    }
    short getCorrADC(short channel, short index) const {
      //if(index<0||SVDPar::maxSample<=index) return -1;
      if (index < 0 || 6 <= index) return -9999;
      if (channel < 0 || 128 <= channel) return -9999;
      else return m_corrADC[channel][index];
    }

    /** Get precisely corrected ADC value with pedestal and fine common-mode.
     * @return ADC value precisely corrected with pedestal and fine common-mode
     */
    const short*  getFineCorrADC(short channel) const {
      if (channel < 0 || 128 <= channel) return NULL;
      return m_fine_corrADC[channel];
    }
    short getFineCorrADC(short channel, short index) const {
      //if(index<0||SVDPar::maxSample<=index) return -1;
      if (index < 0 || 6 <= index) return -9999;
      if (channel < 0 || 128 <= channel) return -9999;
      else return m_fine_corrADC[channel][index];
    }


    /** Set the sensor ID.
     */
    void setSensorID(VxdID vxdid) { m_sensorID = vxdid.getID(); }

    /** Set strip direction.
     */
    void setUStrip(bool isu) { m_isU = isu; }

    /** Set FADC ID.
     */
    void setFadcID(unsigned short fadcID) { m_fadcID = fadcID; }

    /** Set APV ID.
     */
    void setApvID(unsigned short apvID) { m_apvID = apvID; }

    /** Set number of samples.
     */
    void setNSample(unsigned short nSample) { m_nSample = nSample; }

    /** Set ADC values.
     */
    void setADC(short** adc) {
      if (adc != NULL) {
        for (int channel = 0; channel < 128; channel++) {
          //for(int index=0;index<SVDPar::maxSample;index++) {
          for (int index = 0; index < 6; index++) {
            m_ADC[channel][index] = adc[channel][index];
          }
        }
      }
    }
    void setADC(short* adc, short channel) {
      if (adc != NULL) {
        //for(int index=0;index<SVDPar::maxSample;index++) {
        for (int index = 0; index < 6; index++) {
          m_ADC[channel][index] = adc[index];
        }
      }
    }
    void setADC(short adc, short channel, short index) {
      if (0 <= index && index < 6 && 0 <= channel && channel < 128)
        m_ADC[channel][index] = adc;
    }

    /** Set 1st commom-mode correction value.
     */
    void setCMC1(short* cmc1) {
      for (int index = 0; index < 6; index++) {
        m_cmc1[index] = cmc1[index];
      }
    }
    void setCMC1(short cmc1, int index) {
      if (0 <= index && index < 6) m_cmc1[index] = cmc1;
    }

    /** Set 2nd commom-mode correction value.
     */
    void setCMC2(short* cmc2) {
      for (int index = 0; index < 6; index++) {
        m_cmc2[index] = cmc2[index];
      }
    }
    void setCMC2(short cmc2, int index) {
      if (0 <= index && index < 6) m_cmc2[index] = cmc2;
    }

    /** Set Total commom-mode correction value.
     */
    void setCMC(short* cmc) {
      for (int index = 0; index < 6; index++) {
        m_cmc[index] = cmc[index];
      }
    }
    void setCMC(short cmc, int index) {
      if (0 <= index && index < 6) m_cmc[index] = cmc;
    }

    /** Set channel-dependent 1st CMC values.
     */
    void setFineCMC1(short** fine_cmc1) {
      if (fine_cmc1 != NULL) {
        for (int channel = 0; channel < 128; channel++) {
          //for(int index=0;index<SVDPar::maxSample;index++) {
          for (int index = 0; index < 6; index++) {
            m_fine_cmc1[channel][index] = fine_cmc1[channel][index];
          }
        }
      }
    }
    void setFineCMC1(short* fine_cmc1, short channel) {
      if (fine_cmc1 != NULL) {
        //for(int index=0;index<SVDPar::maxSample;index++) {
        for (int index = 0; index < 6; index++) {
          m_fine_cmc1[channel][index] = fine_cmc1[index];
        }
      }
    }
    void setFineCMC1(short fine_cmc1, short channel, short index) {
      if (0 <= index && index < 6 && 0 <= channel && channel < 128)
        m_fine_cmc1[channel][index] = fine_cmc1;
    }

    void setFineCMC2(short** fine_cmc2) {
      if (fine_cmc2 != NULL) {
        for (int channel = 0; channel < 128; channel++) {
          //for(int index=0;index<SVDPar::maxSample;index++) {
          for (int index = 0; index < 6; index++) {
            m_fine_cmc2[channel][index] = fine_cmc2[channel][index];
          }
        }
      }
    }
    void setFineCMC2(short* fine_cmc2, short channel) {
      if (fine_cmc2 != NULL) {
        //for(int index=0;index<SVDPar::maxSample;index++) {
        for (int index = 0; index < 6; index++) {
          m_fine_cmc2[channel][index] = fine_cmc2[index];
        }
      }
    }
    void setFineCMC2(short fine_cmc2, short channel, short index) {
      if (0 <= index && index < 6 && 0 <= channel && channel < 128)
        m_fine_cmc2[channel][index] = fine_cmc2;
    }

    void setFineCMC(short** fine_cmc) {
      if (fine_cmc != NULL) {
        for (int channel = 0; channel < 128; channel++) {
          //for(int index=0;index<SVDPar::maxSample;index++) {
          for (int index = 0; index < 6; index++) {
            m_fine_cmc[channel][index] = fine_cmc[channel][index];
          }
        }
      }
    }
    void setFineCMC(short* fine_cmc, short channel) {
      if (fine_cmc != NULL) {
        //for(int index=0;index<SVDPar::maxSample;index++) {
        for (int index = 0; index < 6; index++) {
          m_fine_cmc[channel][index] = fine_cmc[index];
        }
      }
    }
    void setFineCMC(short fine_cmc, short channel, short index) {
      if (0 <= index && index < 6 && 0 <= channel && channel < 128)
        m_fine_cmc[channel][index] = fine_cmc;
    }

    /** Set corrected ADC values.
     */
    void setCorrADC(short** corradc) {
      if (corradc != NULL) {
        for (int channel = 0; channel < 128; channel++) {
          //for(int index=0;index<SVDPar::maxSample;index++) {
          for (int index = 0; index < 6; index++) {
            m_corrADC[channel][index] = corradc[channel][index];
          }
        }
      }
    }
    void setCorrADC(short* corradc, short channel) {
      if (corradc != NULL) {
        //for(int index=0;index<SVDPar::maxSample;index++) {
        for (int index = 0; index < 6; index++) {
          m_corrADC[channel][index] = corradc[index];
        }
      }
    }
    void setCorrADC(short corradc, short channel, short index) {
      if (0 <= index && index < 6 && 0 <= channel && channel < 128)
        m_corrADC[channel][index] = corradc;
    }

    /** Set precisely corrected ADC values.
     */
    void setFineCorrADC(short** corradc) {
      if (corradc != NULL) {
        for (int channel = 0; channel < 128; channel++) {
          //for(int index=0;index<SVDPar::maxSample;index++) {
          for (int index = 0; index < 6; index++) {
            m_fine_corrADC[channel][index] = corradc[channel][index];
          }
        }
      }
    }
    void setFineCorrADC(short* corradc, short channel) {
      if (corradc != NULL) {
        //for(int index=0;index<SVDPar::maxSample;index++) {
        for (int index = 0; index < 6; index++) {
          m_fine_corrADC[channel][index] = corradc[index];
        }
      }
    }
    void setFineCorrADC(short corradc, short channel, short index) {
      if (0 <= index && index < 6 && 0 <= channel && channel < 128)
        m_fine_corrADC[channel][index] = corradc;
    }

    /** Display main parameters in this object
     */
    std::string print() {
      VxdID thisSensorID = static_cast<VxdID>(m_sensorID);
      VxdID::baseType id      = thisSensorID.getID();
      VxdID::baseType layer   = thisSensorID.getLayerNumber();
      VxdID::baseType ladder  = thisSensorID.getLadderNumber();
      VxdID::baseType sensor  = thisSensorID.getSensorNumber();
      VxdID::baseType segment = thisSensorID.getSegmentNumber();

      std::ostringstream os;
      os << std::endl;
      os << "Sensor ID     : " << m_sensorID << ", (VXD ID: " << id << ")" << std::endl;
      os << "(layer: " << layer << ", ladder: " << ladder
         << ", sensor: " << sensor << ", segment: " << segment << ")"
         << std::endl;
      if (m_isU)
        os << "Strip side    : U" << std::endl;
      else
        os << "Strip side    : V" << std::endl;

      return os.str();
    }

  private:

    unsigned short m_sensorID; /**< Compressed sensor identifier.*/
    bool m_isU;                /**< True if U, false if V. */

    unsigned short m_fadcID;   /**< FADC ID. */
    unsigned short m_apvID;    /**< APV25 ID. */

    unsigned short m_nSample;  /**< Number of samples. */

    //int  m_ADC[SVDPar::maxSample];  /**< ADC values fo strip signal. Index represents samples.*/
    short  m_ADC[128][6];  /**< ADC values of strip signal. Index represents channel index (0~127) and sample index (0~5).*/

    short  m_cmc1[6];  /**< 1st common-mode correction values. Index represents sample index (0~5).*/
    short  m_cmc2[6];  /**< 2nd common-mode correction values. Index represents sample index (0~5).*/
    short  m_cmc[6];  /**< total common-mode correction values. Index represents sample index (0~5).*/

    short  m_fine_cmc1[128][6];  /**< Channel-dependent 1st common-mode correction values. Index represents channel index (0~127) and sample index (0~5).*/
    short  m_fine_cmc2[128][6];  /**< Channel-dependent 2nd common-mode correction values. Index represents channel index (0~127) and sample index (0~5).*/
    short  m_fine_cmc [128][6];  /**< Channel-dependent total common-mode correction values. Index represents channel index (0~127) and sample index (0~5).*/

    short  m_corrADC[128][6];  /**< ADC values of strip signal corrected by pedestal and commom-mode. Index represents channel index (0~127) and sample index (0~5).*/
    short  m_fine_corrADC[128][6];  /**< ADC values of strip signal precisely corrected by pedestal and fine commom-mode. Index represents channel index (0~127) and sample index (0~5).*/

    ClassDef(SVDTransparentDigit, 2)

  }; // class SVDTransparentDigit


} // end namespace Belle2

#endif
