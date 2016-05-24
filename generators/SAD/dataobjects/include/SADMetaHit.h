/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2016  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SADMETAHIT_H
#define SADMETAHIT_H

#include <simulation/dataobjects/SimHitBase.h>
//#include <framework/metastore/RelationsObject.h>

// ROOT
#include <TObject.h>
#include <TVector3.h>
#include <vector>

namespace Belle2 {
  /**
  * ClassSADMetaHit - digitization simulated metahit for the SAD detector.
  *
  * This class holds particle metahit meta from digitization simulation. As the simulated
  * metahit classes are used to generate detector response, they contain _local_
  * information.
  */
  class SADMetaHit : public SimHitBase {
  public:




    /** default constructor for ROOT */
    SADMetaHit(): m_inputSAD_ssraw(0), m_inputSAD_sraw(0), m_inputSAD_ss(0), m_inputSAD_s(0),
      m_inputSAD_Lss(0), m_inputSAD_nturn(0),
      m_inputSAD_x(0), m_inputSAD_y(0), m_inputSAD_px(0), m_inputSAD_py(0), m_inputSAD_xraw(0), m_inputSAD_yraw(0),
      m_inputSAD_r(0), m_inputSAD_rr(0), m_inputSAD_dp_over_p0(0), m_inputSAD_E(0), m_inputSAD_rate(0), m_inputSAD_watt(0) {}
    /** Meta hit store array */
    SADMetaHit(double inputSAD_ssraw, double inputSAD_sraw, double inputSAD_ss, double inputSAD_s,
               double inputSAD_Lss, double inputSAD_nturn,
               double inputSAD_x, double inputSAD_y, double inputSAD_px, double inputSAD_py, double inputSAD_xraw, double inputSAD_yraw,
               double inputSAD_r, double inputSAD_rr, double inputSAD_dp_over_p0, double inputSAD_E, double inputSAD_rate, double inputSAD_watt):
      m_inputSAD_ssraw(inputSAD_ssraw), m_inputSAD_sraw(inputSAD_sraw), m_inputSAD_ss(inputSAD_ss), m_inputSAD_s(inputSAD_s),
      m_inputSAD_Lss(inputSAD_Lss), m_inputSAD_nturn(inputSAD_nturn),
      m_inputSAD_x(inputSAD_x), m_inputSAD_y(inputSAD_y), m_inputSAD_px(inputSAD_px), m_inputSAD_py(inputSAD_py),
      m_inputSAD_xraw(inputSAD_xraw), m_inputSAD_yraw(inputSAD_yraw),
      m_inputSAD_r(inputSAD_r), m_inputSAD_rr(inputSAD_rr), m_inputSAD_dp_over_p0(inputSAD_dp_over_p0), m_inputSAD_E(inputSAD_E),
      m_inputSAD_rate(inputSAD_rate), m_inputSAD_watt(inputSAD_watt)
    {
    }

    /** Return location of scattering in the ring */
    int getssraw()  const { return m_inputSAD_ssraw; }
    /** Return */
    int getsraw()  const { return m_inputSAD_sraw; }
    /** Return */
    int getss()  const { return m_inputSAD_ss; }
    /** Return location of particle loss in the ring */
    int gets()  const { return m_inputSAD_s; }
    /** Return */
    int getLss()  const { return m_inputSAD_Lss; }
    /** Return number of turns until particle loss */
    int getn_turn()  const { return m_inputSAD_nturn; }
    /** Return x-coordinate of the lost particle */
    int getx()  const { return m_inputSAD_x; }
    /** Return y-coordinate of the lost particle  */
    int gety()  const { return m_inputSAD_y; }
    /** Return momentum x-coordinate of the lost particle */
    int getpx()  const { return m_inputSAD_px; }
    /** Return momentum y-coordinate of the lost particle */
    int getpy()  const { return m_inputSAD_py; }
    /** Return */
    int getxraw()  const { return m_inputSAD_xraw; }
    /** Return */
    int getyraw()  const { return m_inputSAD_yraw; }
    /** Return */
    int getr()  const { return m_inputSAD_r; }
    /** Return */
    int getrr()  const { return m_inputSAD_rr; }
    /** Return momentum deviation of the lost particle  */
    int getdp_over_p0()  const { return m_inputSAD_dp_over_p0; }
    /** Return */
    int getE()  const { return m_inputSAD_E; }
    /** Return */
    int getrate()  const { return m_inputSAD_rate; }
    /** Return */
    int getwatt()  const { return m_inputSAD_watt; }

  private:

    /** Return */
    double m_inputSAD_ssraw;
    /** Return */
    double m_inputSAD_sraw;
    /** Return */
    double m_inputSAD_ss;
    /** Return */
    double m_inputSAD_s;
    /** Return */
    double m_inputSAD_Lss;
    /** Return */
    double m_inputSAD_nturn;
    /** Return */
    double m_inputSAD_x;
    /** Return */
    double m_inputSAD_y;
    /** Return */
    double m_inputSAD_px;
    /** Return */
    double m_inputSAD_py;
    /** Return */
    double m_inputSAD_xraw;
    /** Return */
    double m_inputSAD_yraw;
    /** Return */
    double m_inputSAD_r;
    /** Return */
    double m_inputSAD_rr;
    /** Return */
    double m_inputSAD_dp_over_p0;
    /** Return */
    double m_inputSAD_E;
    /** Return */
    double m_inputSAD_rate;
    /** Return */
    double m_inputSAD_watt;

    ClassDef(SADMetaHit, 1)
  };

} // end namespace Belle2

#endif
