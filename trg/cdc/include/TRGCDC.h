//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGCDC.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDC_FLAG_
#define TRGCDC_FLAG_

#define TRGCDC_UNDEFINED 999999

#include <vector>
#include "trg/trg/Clock.h"

class TFile;
class TTree;
class TClonesArray;

namespace HepGeom {
    template <class T> class Point3D;
}

namespace Belle2 {

namespace CDC {
    class CDCGeometryPar; 
}

class TRGTime;
class TRGClock;
class TRGLink;
class TRGCDC;
class TRGCDCWire;
class TRGCDCLayer;
class TRGCDCWireHit;
class TRGCDCWireHitMC;
class TRGCDCSegment;
class TRGCDCSegmentHit;
class TRGCDCFrontEnd;
class TRGCDCMerger;
class TRGCDCTSFBoard;
class TRGCDCTracker2D;
class TRGCDCTrackSegmentFinder;
class TRGCDCPerfectFinder;
class TRGCDCHoughFinder;
class TRGCDCHough3DFinder;
class TRGCDCFitter3D;
class TRGCDCLUT;
class TRGCDCTrack;
class TRGCDCEventTime;

///  The instance of TRGCDC is a singleton. 'TRGCDC::getTRGCDC()'
///  gives you a pointer to access the TRGCDC. Geometrical information
///  is initialized automatically. Before accessing hit information,
///  user has to call 'update()' to initialize hit information event
///  by event.

class TRGCDC {

  public:

    /// returns TRGCDC object with specific configuration.
    static TRGCDC * getTRGCDC(const std::string & configFile,
			      unsigned simulationMode = 0,
			      unsigned fastSimulationMode = 0,
			      unsigned firmwareSimulationMode = 0,
                              bool _makeRootFile = 0,
                              bool perfect2DFinder = false,
                              bool perfect3DFinder = false,
			      const std::string & innerTSLUTFile = "?",
			      const std::string & outerTSLUTFile = "?",
			      const std::string & rootTRGCDCFile = "?",
			      const std::string & rootFitter3DFile = "?",
                              unsigned houghFinderMeshX = 96,
                              unsigned houghFinderMeshY = 96,
                              unsigned houghFinderPeakMin = 5,
                              const std::string & houghMappingFilePlus = "?",
                              const std::string & houghMappingFileMinus = "?",
                              unsigned houghDoit = 2,
                              bool fLogicLUTTSF = 0,
                              bool fLRLUT = 1,
			      bool fevtTime = 1,
			      bool fzierror=1,
			      bool fmclr=0,
			      double inefficiency=0,
                              bool fileTSF=0,
                              bool fileHough3D=0,
                              int finder3DMode=0,
                              bool fileFitter3D=0,
                              int trgCDCDataInputMode=0);
    
    /// returns TRGCDC object. TRGCDC should be created with specific
    /// configuration before calling this function.
    static TRGCDC * getTRGCDC(void);

  private:

    /// Constructor
    TRGCDC(const std::string & configFile,
	   unsigned simulationMode,
	   unsigned fastSimulationMode,
	   unsigned firmwareSimulationMode,
           bool makeRootFile,
	   bool perfect2DFinder,
	   bool perfect3DFinder,
	   const std::string & innerTSLUTFile,
	   const std::string & outerTSLUTFile,
	   const std::string & rootTRGCDCFile,
	   const std::string & rootFitter3DFile,
           unsigned houghFinderMeshX,
           unsigned houghFinderMeshY,
           unsigned houghFinderPeakMin,
           const std::string & houghMappingFilePlus,
           const std::string & houghMappingFileMinus,
           unsigned houghDoit,
           bool fLogicLUTTSF,
           bool fLRLUT,
	   bool fevtTime,
	   bool fzierror,
	   bool fmclr,
	   double inefficiecny,
           bool fileTSF,
           bool fileHough3D,
           int finder3DMode,
           bool fileFitter3D,
           int trgCDCDataInputMode);

    /// Destructor
    virtual ~TRGCDC();

    /// initializes CDC geometry.
    void initialize(unsigned houghFinderMeshX,
		    unsigned houghFinderMeshY,
		    unsigned houghFinderPeakMin,
                    const std::string & houghMappingFilePlus,
                    const std::string & houghMappingFileMinus,
                    unsigned houghDoit);

    /// configures trigger modules for firmware simulation.
    void configure(void);

    /// fast trigger simulation.
    void simulate(void);

    /// terminates when run is finished
    void terminate(void);

    /// simulates track segment decisions.

  public:// Selectors

    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// returns CDC version.
    std::string versionCDC(void) const;

    /// sets simulation mode.
    unsigned mode(void) const;

    /// sets simulation mode.
    unsigned mode(unsigned);

    /// returns firmware simulation mode.
    unsigned firmwareSimulationMode(void) const;

    /// dumps debug information.
    void dump(const std::string & message) const;

    /// returns debug level.
    int debugLevel(void) const;

    /// sets and returns debug level.
    int debugLevel(int) const;

    /// returns fudge factor for drift time error.
    float fudgeFactor(void) const;

    /// sets and returns fudge factor for drift time error.
    float fudgeFactor(float);

    /// returns 2D track list (no fit).
    std::vector<TRGCDCTrack *> getTrackList2D(void);

    /// returns 2D fitted track list.
    std::vector<TRGCDCTrack *> getTrackList2DFitted(void);

    /// returns 3D track list (fitted).
    std::vector<TRGCDCTrack *> getTrackList3D(void);

  public:// Geometry

    /// returns a pointer to CDCGeometryPar
    CDC::CDCGeometryPar* m_cdcp;

    /// returns a pointer to a wire. 0 will be returned if 'wireId' is
    /// invalid.
    const TRGCDCWire * wire(unsigned wireId) const;

    /// returns a pointer to a wire. 'localId' can be negative. 0 will
    /// be returned if 'layerId' is invalid.
    const TRGCDCWire * wire(unsigned layerId, int localId) const;

    /// returns a pointer to a wire.
//    const TRGCDCWire * wire(const HepGeom::Point3D<double> & point) const;

    /// returns a pointer to a wire.
    const TRGCDCWire * wire(float r, float phi) const;

    /// returns a pointer to a layer. 0 will be returned if 'id' is invalid.
    const TRGCDCLayer * layer(unsigned id) const;

    /// returns a pointer to a super-layer. 0 will be returned if 'id'
    /// is invalid.
    const std::vector<TRGCDCLayer *> * superLayer(unsigned id) const;

    /// returns \# of wire layers in a super layer. 0 will be returned
    /// if 'superLayerId' is invalid.
    unsigned nLocalLayers(unsigned superLayerId) const;

    /// return \# of wires.
    unsigned nWires(void) const;

    /// returns \# of super layers.
    unsigned nSuperLayers(void) const;

    /// return \# of layers.
    unsigned nLayers(void) const;

    /// return \# of axial layers.
    unsigned nAxialLayers(void) const;

    /// returns \# of stereo layers.
    unsigned nStereoLayers(void) const;

    /// return \# of axial super layers.
    unsigned nAxialSuperLayers(void) const;

    /// returns \# of stereo super layers.
    unsigned nStereoSuperLayers(void) const;

    /// returns cell width in unit of radian.
    float cellWidth(unsigned superLayerId) const;

    /// returns inner radius of super layer.
    float superLayerR(unsigned superLayerId) const;

    /// returns (inner radius)^2 of super layer.
    float superLayerR2(unsigned superLayerId) const;

    /// returns \# of track segments.
    unsigned nSegments(void) const;

    /// returns a track segment.
    const TRGCDCSegment & segment(unsigned id) const;

    /// returns a track segment.
    const TRGCDCSegment & segment(unsigned lyrId, unsigned id) const;

    /// returns a track segment in axial layers. (lyrId is axial #)
    const TRGCDCSegment & axialSegment(unsigned lyrId, unsigned id) const;

    /// returns a track segment in stereo layers. (lyrId is stereo #)
    const TRGCDCSegment & stereoSegment(unsigned lyrId, unsigned id) const;

    /// returns \# of track segment layers.
    unsigned nSegmentLayers(void) const;

    /// returns a pointer to a track segment layer. 0 will be returned
    /// if 'id' is invalid.
    const TRGCDCLayer * segmentLayer(unsigned id) const;

    /// returns \# of track segments in a layer.
    unsigned nSegments(unsigned id) const;

  public:// Event by event hit information.

    /// clears all TRGCDC hit information.
    void clear(void);

    /// clears TRGCDC information.
    void fastClear(void);

    /// updates TRGCDC wire information. clear() is called in this function.
    void update(bool mcAnalysis = true);

    /// updates TRGCDC wire information by Hardware data
    /// 0: From CDC FE ASCII file (Implementing)
    /// 1: From CDC FE-DAQ root file (Not implemented)
    /// 2: From TSIM root file (Not implemented)
    void updateByData(int inputMode);

    /// returns a list of TRGCDCWireHit. 'update()' must be called
    /// before calling this function.
    std::vector<const TRGCDCWireHit *> hits(void) const;

    /// returns a list of axial hits. 'update()' must be called before
    /// calling this function.
    std::vector<const TRGCDCWireHit *> axialHits(void) const;

    /// returns a list of stereo hits. 'update()' must be called
    /// before calling this function.
    std::vector<const TRGCDCWireHit *> stereoHits(void) const;

    /// returns a list of TRGCDCSegmentHit. 'simulate()' must be
    /// called before calling this function
    std::vector<const TRGCDCSegmentHit *> segmentHits(void) const;

    /// returns a list of TRGCDCSegmentHit in a super layer
    /// N. 'simulate()' must be called before calling this function
    std::vector<const TRGCDCSegmentHit *> segmentHits(unsigned) const;

    /// returns a list of TRGCDCSegmentHit in a axial super layer
    /// N. 'simulate()' must be called before calling this function
    std::vector<const TRGCDCSegmentHit *> axialSegmentHits(unsigned) const;

    /// returns a list of TRGCDCSegmentHit in a stereo super layer
    /// N. 'simulate()' must be called before calling this function
    std::vector<const TRGCDCSegmentHit *> stereoSegmentHits(unsigned) const;

    /// returns a list of TRGCDCWireHitMC. 'updateMC()' must be called
    /// before calling this function.
    std::vector<const TRGCDCWireHitMC *> hitsMC(void) const;

    /// returns bad hits(finding invalid hits).
//  std::vector<const TRGCDCWireHit *> badHits(void) const;

    /// returns event time 
    double getEventTime(void) const;

    /// returns event number
    unsigned getEventNumber(void) const;

  public:// Utility functions

    /// returns wire name.
    std::string wireName(unsigned wireId) const;

    /// returns local ID in a layer. This function is expensive.
    unsigned localId(unsigned wireId) const;

    /// returns layer ID. This function is expensive.
    unsigned layerId(unsigned wireId) const;

    /// returns layer ID from axialStereoLayerId. This function is expensive.
    unsigned layerId(unsigned axialStereoSwitch,
                     unsigned axialStereoLayerId) const;

    /// returns super layer ID. This function is expensive.
    unsigned superLayerId(unsigned wireId) const;

    /// returns local layer ID in a super layer. This function is expensive.
    unsigned localLayerId(unsigned wireId) const;

    /// returns axialStereo super layer ID. This function is expensive.
    unsigned axialStereoSuperLayerId(unsigned axialStereo,
                                     unsigned axialStereoLayerId) const;

    /// returns true if w0 and w1 are neighbor.
    bool neighbor(const TRGCDCWire & w0, const TRGCDCWire & w1) const;

    /// fills stereo TSs to tracks using MC info.
    void perfect3DFinder(std::vector<TRGCDCTrack *> trackList) const;

    /// calculates corrected drift time. correctionFlag(bit 0:tof,
    /// 1:T0 offset, 2:propagation delay, 3:tan(lambda) correction)
//     static void driftDistance(TLink & link,
//                               const TTrack & track,
//                               unsigned correctionFlag = 0,
//                               float T0Offset = 0.);

  public:// TRG information

    /// returns the system clock.
    const TRGClock & systemClock(void) const;

    /// returns the system clock of the front-end
    const TRGClock & systemClockFE(void) const;

    /// returns the data clock.
    const TRGClock & dataClock(void) const;

    /// returns the user clock for Aurora 3.125 Gbps.
    const TRGClock & userClock3125(void) const;

    /// returns the system offset in MC.
    double systemOffsetMC(void) const;

    /// returns a front-end board.
    const TRGCDCFrontEnd * frontEnd(unsigned id) const;

    /// returns a merger board.
    const TRGCDCMerger * merger(unsigned id) const;

    /// returns a TSF board.
//  const TRGCDCTrackSegmentFinder * tsfboard(unsigned id) const;
    TRGCDCTrackSegmentFinder * tsfboard(unsigned id) const;

    /// returns a 2D tracker board.
    const TRGCDCTracker2D * tracker2D(unsigned id) const;

  private:

    /// classify hits.
    void classification(void);

    /// updates TRGCDC information for MC.
    void updateMC(void);

    /// Fast simulation
    void fastSimulation(void);

    /// Firmware simulation
    void firmwareSimulation(void);

  private:

    /// CDC trigger singleton.
    static TRGCDC * _cdc;

    /// Debug level.
    mutable int _debugLevel;

    /// CDC version.
    const std::string _cdcVersion;

    /// CDC trigger configuration filename.
    std::string _configFilename;

    /// Simulation mode.
    unsigned _simulationMode;

    /// Fast simulation mode.
    unsigned _fastSimulationMode;

    /// Firmware simulation mode.
    unsigned _firmwareSimulationMode;

    /// Track list by 2D finding.
    std::vector<TRGCDCTrack *> _trackList2D;

    /// Track list for 2D fitted tracks.
    std::vector<TRGCDCTrack *> _trackList2DFitted;

    /// Track list for 3D fitted tracks.
    std::vector<TRGCDCTrack *> _trackList3D;

    /// Switch for TRGCDC.root file.
    bool _makeRootFile;

    /// Switch to activate perfect 2D finder.
    const bool _perfect2DFinder;

    /// Switch to activate perfect 3D finder.
    const bool _perfect3DFinder;

    /// The filename of LUT for the inner-most track segments.
    std::string _innerTSLUTFilename;

    /// The filename of LUT for outer track segments.
    std::string _outerTSLUTFilename;

    /// The filename of root file for TRGCDC.
    std::string _rootTRGCDCFilename;

    /// The filename of root file for Fitter3D.
    std::string _rootFitter3DFilename;

    /// Switch for logic or LUT TSF
    bool _fLogicLUTTSF;

    /// Switch for the LR LUT in Fitter3D.
    bool _fLRLUT;

    /// Switch for the event tiem in Segment.
    bool _fevtTime;

    /// Switch for the zi error.
    bool _fzierror;

    /// Switch for MC L/R information.
    bool _fmclr;

    /// Hit inefficiency parameter.
    double _inefficiency;

    /// Switch for TSF.root file.
    bool _fileTSF;

    /// Switch for Hough3D.root file.
    bool _fileHough3D;

    /// Sets mode of 3DFinder.
    int _finder3DMode;

    /// Switch for Fitter3D.root file.
    bool _fileFitter3D;

    /// Super layers.
    std::vector<std::vector<TRGCDCLayer *> *> _superLayers;

    /// Axial super layers.
    std::vector<std::vector<TRGCDCLayer *> *> _axialSuperLayers;

    /// Stereo super layers.
    std::vector<std::vector<TRGCDCLayer *> *> _stereoSuperLayers;

    /// All layers.
    std::vector<TRGCDCLayer *> _layers;

    /// Axial layers.
    std::vector<TRGCDCLayer *> _axialLayers;

    /// Stereo layers.
    std::vector<TRGCDCLayer *> _stereoLayers;

    /// All wires.
    std::vector<TRGCDCWire *> _wires;

    /// Wires with a hit.
    std::vector<TRGCDCWire *> _hitWires;

    /// CDC hits.
    std::vector<TRGCDCWireHit *> _hits;

    /// CDC hits on axial wires.
    std::vector<TRGCDCWireHit *> _axialHits;

    /// CDC hits on stereo wires.
    std::vector<TRGCDCWireHit *> _stereoHits;

    /// Bad CDC hits.(not used now)
    std::vector<TRGCDCWireHit *> _badHits;

    /// MC info. of CDC hits.
    std::vector<TRGCDCWireHitMC *> _hitsMC;

    /// Track Segments.
    std::vector<TRGCDCSegment *> _tss;

    /// Track Segments.
    std::vector<TRGCDCSegment *> _tsSL[9];

    /// Track Segment layers.
    std::vector<TRGCDCLayer *> _tsLayers;

    /// Track Segments with hits.
    std::vector<TRGCDCSegmentHit *> _segmentHits;

    /// Track Segments with hits in each super layer.
    std::vector<TRGCDCSegmentHit *> _segmentHitsSL[9];

    /// Fudge factor for position error.
    float _fudgeFactor;

    /// Cell width in radian.
    float * _width;

    /// R of cell.
    float * _r;

    /// R^2 of cell.
    float * _r2;

    /// CDC trigger system clock.
    const TRGClock _clock;

    /// CDC FE trigger system clock.
    const TRGClock _clockFE;

    /// CDC trigger data clock.
    const TRGClock _clockD;

    /// CDC trigger user clock for Aurora 3.125 Gbps.
    const TRGClock _clockUser3125;

    /// CDC trigger user clock for Aurora 6.250 Gbps.
    const TRGClock _clockUser6250;

    /// Timing offset of CDC trigger.
    const double _offset;

    /// All serial links.
    std::vector<TRGLink *> _links;

    /// CDC front-end boards.
    std::vector<TRGCDCFrontEnd *> _fronts;

    /// CDC trigger merger boards.
    std::vector<TRGCDCMerger *> _mergers;

    /// CDC trigger tsf boards.
    std::vector <TRGCDCTrackSegmentFinder *> _tsfboards;

    /// CDC 2D finder boards.
    std::vector <TRGCDCTracker2D *> _tracker2Ds;

    /// Track Segement Finder.
    TRGCDCTrackSegmentFinder* _tsFinder;

    /// Perfect 2D finder.
    TRGCDCPerfectFinder * _pFinder;

    /// Perfect 3D finder.
    TRGCDCPerfectFinder * _p3DFinder;

    /// Hough finder.
    TRGCDCHoughFinder * _hFinder;

    /// Hough 3D finder.
    TRGCDCHough3DFinder * _h3DFinder;

    /// 3D fitter.
    TRGCDCFitter3D * _fitter3D;

    /// EventTime
    std::vector<TRGCDCEventTime *> _eventTime;

    /// Event number
    int m_eventNum;

    /// root file
    TFile * m_file;

    /// root tree for reconstructed 3D tracks
    TTree * m_tree;

    /// root tree for MC tracks
    TTree * m_treeAllTracks;

    /// root tree for 2D tracks
    TTree * _tree2D;

    /// 3D fit
    TClonesArray * m_fitParameters;

    /// MC
    TClonesArray * m_mcParameters;

    /// MC track
    TClonesArray * m_mcTrack4Vector;

    /// MC vertex
    TClonesArray * m_mcTrackVertexVector;

    /// MC track status
    TClonesArray * m_mcTrackStatus;

    /// Event time
    TClonesArray * m_evtTime;

    /// 2D track information
    TClonesArray * _tracks2D;

    // Switch for TRG CDC input mode
    int _trgCDCDataInputMode;

    // Debugging members for firmware ROOT input.
    //int m_minCDCTdc;
    //int m_maxCDCTdc;
    //int m_minTRGTdc;
    //int m_maxTRGTdc;
    /// root tree for ROOT input
    TTree * m_treeROOTInput;
    // [0]: iLayer, [1]: iWire, [2]: CDCADC, [3]: CDCTDC, [4]: CDC FE TRG timing
    TClonesArray* m_rootCDCHitInformation;
    // [0]: iLayer, [1]: iWire, [2]: window number, [3]: priority timing
    TClonesArray* m_rootTRGHitInformation;
    // [0]: iLayer, [1]: iWire, [2]: Timing(CDC), [3]: MatchNumber
    // MatchNumber: 1: Only CDC, 2: Only TRG, 3: Both CDC & TRG
    //TClonesArray* m_CDCTRGTimeMatch;
    TClonesArray* m_rootTRGRawInformation;

    // Save functions for ROOT
    void saveCDCHitInformation(std::vector<std::vector<unsigned> >& );
    void saveTRGHitInformation(std::vector<std::vector<int > >& );
    void saveTRGRawInformation(std::vector<std::string >& );
    //void saveCDCTRGTimeMatchInformation(std::vector<std::vector<std::map<int, int > > >& );


    friend class TRGCDCModule;
};

//-----------------------------------------------------------------------------

inline
int
TRGCDC::debugLevel(void) const {
    return _debugLevel;
}

inline
int
TRGCDC::debugLevel(int a) const {
    return _debugLevel = a;
}

inline
unsigned
TRGCDC::nLocalLayers(unsigned superLayerId) const {
    std::vector<TRGCDCLayer *> * superLayer = _superLayers[superLayerId];
    if (! superLayer) return 0;
    return superLayer->size();
}

inline
const TRGCDCLayer *
TRGCDC::layer(unsigned id) const {
    return _layers[id];
}

inline
const std::vector<TRGCDCLayer *> *
TRGCDC::superLayer(unsigned id) const {
    return _superLayers[id];
}

inline
float
TRGCDC::fudgeFactor(void) const {
    return _fudgeFactor;
}

inline
float
TRGCDC::fudgeFactor(float a) {
    return _fudgeFactor = a;
}

inline
std::string
TRGCDC::versionCDC(void) const {
    return std::string(_cdcVersion);
}

inline
unsigned
TRGCDC::nWires(void) const {
    return _wires.size();
}

inline
unsigned
TRGCDC::nSuperLayers(void) const {
    return _superLayers.size();
}

inline
unsigned
TRGCDC::nStereoLayers(void) const {
    return _stereoLayers.size();
}

inline
unsigned
TRGCDC::nAxialLayers(void) const {
    return _axialLayers.size();
}

inline
unsigned
TRGCDC::nAxialSuperLayers(void) const {
    return _axialSuperLayers.size();
}

inline
unsigned
TRGCDC::nStereoSuperLayers(void) const {
    return _stereoSuperLayers.size();
}

inline
unsigned
TRGCDC::nLayers(void) const {
    return _layers.size();
}

inline
float
TRGCDC::cellWidth(unsigned a) const {
    return _width[a];
}

inline
float
TRGCDC::superLayerR(unsigned i) const {
    return _r[i];
}

inline
float
TRGCDC::superLayerR2(unsigned i) const {
    return _r2[i];
}

inline
const TRGCDCSegment &
TRGCDC::segment(unsigned id) const {
    return * _tss[id];
}

inline
const TRGCDCSegment &
TRGCDC::axialSegment(unsigned a, unsigned b) const {
    return * _tsSL[a * 2][b];
}

inline
const TRGCDCSegment &
TRGCDC::stereoSegment(unsigned a, unsigned b) const {
    return * _tsSL[a * 2 + 1][b];
}

inline
unsigned
TRGCDC::nSegments(void) const {
    return _tss.size();
}

inline
const TRGClock &
TRGCDC::systemClock(void) const {
    return _clock;
}

inline
const TRGClock &
TRGCDC::systemClockFE(void) const {
    return _clockFE;
}

inline
const TRGClock &
TRGCDC::dataClock(void) const {
    return _clockD;
}

inline
double
TRGCDC::systemOffsetMC(void) const {
    return _offset;
}

inline
std::vector<const TRGCDCSegmentHit *>
TRGCDC::segmentHits(void) const {
    std::vector<const TRGCDCSegmentHit *> t;
    t.assign(_segmentHits.begin(), _segmentHits.end());
    return t;
}

inline
std::vector<const TRGCDCSegmentHit *>
TRGCDC::segmentHits(unsigned a) const {
    std::vector<const TRGCDCSegmentHit *> t;
    t.assign(_segmentHitsSL[a].begin(), _segmentHitsSL[a].end());
    return t;
}

inline
std::vector<const TRGCDCSegmentHit *>
TRGCDC::axialSegmentHits(unsigned a) const {
    std::vector<const TRGCDCSegmentHit *> t;
    t.assign(_segmentHitsSL[a * 2].begin(), _segmentHitsSL[a * 2].end());
    return t;
}

inline
std::vector<const TRGCDCSegmentHit *>
TRGCDC::stereoSegmentHits(unsigned a) const {
    std::vector<const TRGCDCSegmentHit *> t;
    t.assign(_segmentHitsSL[a * 2 + 1].begin(),
	     _segmentHitsSL[a * 2 + 1].end());
    return t;
}

inline
const TRGCDCFrontEnd *
TRGCDC::frontEnd(unsigned a) const {
    return _fronts[a];
}

inline
const TRGCDCMerger *
TRGCDC::merger(unsigned a) const {
    return _mergers[a];
}

inline
TRGCDCTrackSegmentFinder *
TRGCDC::tsfboard(unsigned a)const{
	return _tsfboards[a];
}

inline
const TRGCDCTracker2D *
TRGCDC::tracker2D(unsigned a)const{
	return _tracker2Ds[a];
}

inline
const TRGCDCLayer *
TRGCDC::segmentLayer(unsigned id) const {
    if (id < _tsLayers.size())
        return _tsLayers[id];
    return 0;
}

inline
unsigned
TRGCDC::nSegmentLayers(void) const {
    return _tsLayers.size();
}

inline
unsigned
TRGCDC::mode(void) const {
    return _simulationMode;
}

inline
unsigned
TRGCDC::mode(unsigned a) {
    return _simulationMode = a;
}

inline
unsigned
TRGCDC::firmwareSimulationMode(void) const {
    return _firmwareSimulationMode;
}

} // namespace Belle2

#endif /* TRGCDC_FLAG_ */
