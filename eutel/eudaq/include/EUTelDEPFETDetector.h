// -*- mode: c++; mode: auto-fill; mode: flyspell-prog; -*-
/*
 *   This source code is part of the Eutelescope package of Marlin.
 *   You are free to use this source files for your own development as
 *   long as it stays in a public research context. You are not
 *   allowed to use it for commercial purpose. You must put this
 *   header with author names in all development based on this file.
 *
 */

#ifndef EUTELDEPFETDETECTOR_H
#define EUTELDEPFETDETECTOR_H

// personal includes ".h"
#include <eutel/eudaq/EUTELESCOPE.h>
#include <eutel/eudaq/EUTelPixelDetector.h>

// lcio includes <.h>

// system includes <>
#include <iostream>
#include <vector>
#include <string>

namespace eutelescope {


  //! This is the TLU fake detector
  /*!
   *
   *  @author Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
   *  @version $Id: EUTelDEPFETDetector.h,v 1.1 2008-11-12 12:03:16 furletova Exp $
   */

  class EUTelDEPFETDetector : public EUTelPixelDetector {

  public:
    //! Default constructor
    EUTelDEPFETDetector() ;

    //! Default destructor
    virtual ~EUTelDEPFETDetector() {;}

    //! Get the first pixel along x
    virtual unsigned short getXMin() const { return _xMin ; }

    //! Get the first pixel along y
    virtual unsigned short getYMin() const { return _yMin ; }

    //! Get the last pixel along x
    virtual unsigned short getXMax() const { return _xMax ; }

    //! Get the last pixel along y
    virtual unsigned short getYMax() const { return _yMax ; }

    //! Get the no of pixel along X
    virtual unsigned short getXNoOfPixel() const { return _xMax - _xMin + 1 ; }

    //! Get the no of pixel along Y
    virtual unsigned short getYNoOfPixel() const { return _yMax - _yMin + 1 ; }

    //! Get the pixel pitch along X
    virtual float getXPitch() const { return _xPitch ; }

    //! Get the pixel pitch along Y
    virtual float getYPitch() const  { return _yPitch ; }

    //! Get signal polarity
    virtual short getSignalPolarity() const { return _signalPolarity ; }

    //! Get detector name
    virtual std::string getDetectorName() const { return _name ; }

    //! Get RO mode
    virtual std::string getMode() const { return _mode ; }

    //! Get marker position
    virtual std::vector< size_t > getMarkerPosition() const { return _markerPos ; }

    //! Has marker?
    virtual bool hasMarker() const  { if (_markerPos.size() != 0) return true; else return false; }

    //! Set the RO modality
    void setMode(std::string mode);

    //! Has subchannel?
    virtual bool hasSubChannels() const;

    //! Get subchannels
    virtual std::vector< EUTelROI > getSubChannels(bool withMarker = false) const ;

    //! Get a subchannel boundaries
    virtual EUTelROI getSubChannelBoundary(size_t iChan, bool witMarker = false) const ;


    /*! This method is used to print out the detector
     *
     *  @param os The input output stream
     */
    virtual void print(std::ostream& os) const ;



  protected:



  };

}

#endif
