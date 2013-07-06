#ifndef EVEVISUALIZATION_H
#define EVEVISUALIZATION_H

#include <geometry/bfieldmap/BFieldMap.h>
#include <generators/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <svd/dataobjects/SVDSimHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <bklm/dataobjects/BKLMSimHit.h>
#include <eklm/dataobjects/EKLMStepHit.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <ecl/dataobjects/ECLHit.h>
#include <vxd/geometry/GeoCache.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>

#include <GFRaveVertex.h>
#include <GFTrack.h>
#include <GFTrackCand.h>

#include <TEveStraightLineSet.h>
#include <TEveTrackPropagator.h>
#include <TVector3.h>
#include <TString.h>
#include <TEveTrack.h>
#include <TEveManager.h>
#include <TEveCalo.h>

#include <string>
#include <vector>


class TEveBox;
class TEveCaloDataVec;
class TEveElementList;
class TEvePointSet;
class TEveTrackList;

namespace Belle2 {

  /** Produces visualisation for MCParticles, simhits, GFTracks, geometry and other things.
   *
   * Creates TEve objects from the given data, and adds them to the global
   * or event scene.
   *
   *  @sa DisplayModule
   */
  class EVEVisualization {

    /** Hold MC tracks and associated visualisation objects. */
    struct MCTrack {
      TEveTrack* track; /**< the actual MC track. */
      TEvePointSet* simhits; /**< simhit positions. */
    };

    /** Provide magnetic field values for TEveTrackPropagator. */
    class EveVisBField : public TEveMagField {
    public:
      EveVisBField(): TEveMagField() { }
      virtual ~EveVisBField() { }

      /** return field strength at given coordinates, using Eve conventions. */
      virtual TEveVector GetField(Float_t x, Float_t y, Float_t z) const {
        TEveVector v;

        v.Set(BFieldMap::Instance().getBField(TVector3(x, y, z)));
        v.fZ *= -1; //Eve has inverted B field convention
        v.fZ -= 1e-6; //field must not be zero!

        return v;
      }
      /** maximal field strength (is this correct?) */
      virtual Float_t GetMaxFieldMag() const { return 1.5; }
    };


  public:
    /** Constructor.
     */
    explicit EVEVisualization();
    /** Destructor. */
    ~EVEVisualization();

    /** Add TGeo geometry to Eve (only needs to be done once.) */
    void addGeometry();

    // === adding event data ===

    /** Add this GFTrack to event data.
     *
     *  Adapted from GenfitDisplay, originally written by Karl Bicker. */
    void addTrack(const GFTrack* gftrack, const TString& label = "");

    /** Add a GFTrackCand, to evaluate track finding. */
    template<class PXDType, class SVDType> void addTrackCandidate(const GFTrackCand* trackCand, const TString& label,
        const StoreArray<PXDType>& pxdhits, const StoreArray<SVDType>& svdhits, const StoreArray<CDCHit>& cdchits);

    /** Add all entries in the given 'hits' array (and the corresponding MCParticles) to the event scene. */
    template <class T> void addSimHits(const StoreArray<T>& hits) {
      const int numHits = hits.getEntries();
      for (int i = 0; i < numHits; i++) {
        const RelationsObject* rel = static_cast<RelationsObject*>(hits[i]);
        const MCParticle* mcpart = rel->getRelatedFrom<MCParticle>();

        addSimHit(hits[i], mcpart);
      }
    }

    /** Add a CDCSimHit. */
    void addSimHit(const CDCSimHit* hit, const MCParticle* particle);

    /** Add a PXDSimHit. */
    void addSimHit(const PXDSimHit* hit, const MCParticle* particle);

    /** Add a SVDSimHit. */
    void addSimHit(const SVDSimHit* hit, const MCParticle* particle);

    /** Add a BKLMSimHit. */
    void addSimHit(const BKLMSimHit* hit, const MCParticle* particle);

    /** Add a EKLMStepHit. */
    void addSimHit(const EKLMStepHit* hit, const MCParticle* particle);

    /** Add simhit as a simple point. */
    void addSimHit(const TVector3& v, const MCParticle* particle);

    /** Add a ECL hit. */
    void addSimHit(const ECLHit* hit, const MCParticle* particle);

    /** Return MCTrack for given particle, add it if it doesn't exist yet.
     *
     * If particle is NULL, a dummy MCTrack (with track=0) is created which can accept otherwise unassigned hits.
     * Returns NULL if this particle and its hits shouldn't be shown.
     **/
    MCTrack* addMCParticle(const MCParticle* particle);

    /** Create visual representation of all tracks.
     *
     *  Needs to be called after all hits / tracks are added.
     */
    void makeTracks();

    /** clear event data. */
    void clearEvent();

    /** Add a vertex point and its covariance matrix. */
    void addVertexEllip(const GFRaveVertex&, const TString&, const TString&);

    /** Add a reconstructed photon in the ECL. */
    void addRecGammas(const ECLGamma*, const TString&);

    // === configuration options ===

    /** @brief Set the display options.
     *
     * The option string lets you steer the way the events are displayed. The following
     * options are available:\n
     * \n
     * 'D': Draw detectors. This causes a simple representation for all detectors to be drawn. For
     *      planar detectors, this is a plane with the same position and orientation of the real
     *      detector plane, but with different size. For wires, this is a tube whose diameter
     *      is equal to the value measured by the wire. Spacepoint hits are not affected by this
     *      option.\n\n
     * 'H': Draw hits. This causes the hits to be visualized. Normally, the size of the hit
     *      representation is connected to the covariance matrix of the hit, scaled by the value
     *      set in setErrScale which is normally 1. See also option 'A' and 'S'. Normally used in
     *      connection with 'D'.\n\n
     * 'M': Draw track markers. Draw the intersection points between the track and the virtual
     *      (and/or real) detector planes. Can only be used in connection with 'T'.\n\n
     * 'P': Draw detector planes. Draws the virtual (and/or real) detector planes.\n\n
     * 'S': Scale manually. This leads to the spacepoint hits (and only them up to now!) being drawn
     *      as spheres with radius 0.5 scaled with the error scale factor. Can be used if the scaling
     *      with errors leads to problems.\n\n
     * 'T': Draw Track. Draw the track as straight lines between the virtual (and/or real) detector
     *      planes.\n\n
     */
    void setOptions(const std::string& opts);

    /** @brief Set the scaling factor for the visualization of the errors.*/
    void setErrScale(double errScale = 1.);

    /** If true, hits created by secondary particles (e.g. delta electrons) will be assigned to the original primary particle. */
    void setAssignToPrimaries(bool on) { m_assignToPrimaries = on; }

    /** If true, secondary MCParticles (and hits created by them) will not be shown. */
    void setHideSecondaries(bool on) { m_hideSecondaries = on; }

    /** Show full geometry instead of simplified shapes. */
    void showFullGeo(bool on) { m_fullgeo = on; }

  private:
    /** Object for the energy bar visualisation. */
    TEveCalo3D* m_calo3d;

    /** @brief Create a box around o, oriented along u and v with widths ud, vd and depth and
     *  return a pointer to the box object.
     */
    TEveBox* boxCreator(const TVector3& o, TVector3 u, TVector3 v, float ud, float vd, float depth);

    /** enable/disable rendering of the volume 'name', or only its daughters if only_daughters is set. */
    void enableVolume(const char* name, bool only_daughters = false, bool enable = true);

    /** disable rendering of the volume 'name', or only its daughters if only_daughters is set. */
    void disableVolume(const char* name, bool only_daughters = false) { enableVolume(name, only_daughters, false); }

    /** set fill color of the volume 'name' to 'col'. */
    void setVolumeColor(const char* name, Color_t col);

    /** adds given VXD hit to lines. */
    template <class SomeVXDHit> void addRecoHit(const SomeVXDHit* hit, TEveStraightLineSet* lines) {
      static VXD::GeoCache& geo = VXD::GeoCache::getInstance();

      const TVector3 local_pos(hit->getU(), hit->getV(), 0.0); //z-component is height over the center of the detector plane
      const VXD::SensorInfoBase& sensor = geo.get(hit->getSensorID());
      const TVector3 global_pos = sensor.pointToGlobal(local_pos);
      lines->AddMarker(global_pos.x(), global_pos.y(), global_pos.z());
    }

    /** specialisation for SVDCluster */
    void addRecoHit(const SVDCluster* hit, TEveStraightLineSet* lines);

    /** specialisation for CDCHit. */
    void addRecoHit(const CDCHit* hit, TEveStraightLineSet* lines);

    /** Rescale PXD/SVD errors with this factor to ensure visibility. */
    double m_errorScale;

    /** Option string for GFTrack visualisation.
     *
     * @sa setOptions
     */
    std::string m_options;

    /** Show full geometry instead of simplified shapes. */
    bool m_fullgeo;

    /** If true, hits created by secondary particles (e.g. delta electrons) will be assigned to the original primary particle. */
    bool m_assignToPrimaries;

    /** If true, secondary MCParticles (and hits created by them) will not be shown. */
    bool m_hideSecondaries;

    /** map MCParticles to MCTrack (so hits can be added to the correct track). */
    std::map<const MCParticle*, MCTrack> m_mcparticleTracks;

    /** parent object for MC tracks. */
    TEveTrackList* m_tracklist;

    /** parent object for reconstructed tracks. */
    TEveTrackList* m_gftracklist;

    /** parent object for track candidates. */
    TEveTrackList* m_trackcandlist;

    /** Track propagator for MCParticles*/
    TEveTrackPropagator* m_trackpropagator;

    /** Track propagator for GFTracks (different mainly because of drawing options) */
    TEveTrackPropagator* m_gftrackpropagator;

    /** ECL hit data. */
    TEveCaloDataVec* m_eclsimhitdata;

    /** The global magnetic field. */
    EveVisBField m_bfield;
  };

  template<class PXDType, class SVDType> void EVEVisualization::addTrackCandidate(const GFTrackCand* trackCand, const TString& label,
      const StoreArray<PXDType>& pxdhits, const StoreArray<SVDType>& svdhits, const StoreArray<CDCHit>& cdchits)
  {
    const Color_t trackCandColor = kAzure - 2;

    // parse the option string ------------------------------------------------------------------------
    bool drawHits = false;
    bool drawTrack = false;

    if (m_options != "") {
      for (size_t i = 0; i < m_options.length(); i++) {
        if (m_options.at(i) == 'H') drawHits = true;
        if (m_options.at(i) == 'T') drawTrack = true;
      }
    }
    // finished parsing the option string -------------------------------------------------------------


    //track seeds
    TVector3 track_pos = trackCand->getPosSeed();
    TVector3 track_mom = trackCand->getMomSeed();

    TEveStraightLineSet* lines = new TEveStraightLineSet("RecoHits");
    lines->SetMainColor(trackCandColor);
    lines->SetMarkerColor(trackCandColor);
    lines->SetMarkerStyle(6);
    lines->SetMainTransparency(60);

    const unsigned int numhits = trackCand->getNHits();
    if (drawHits) {
      for (unsigned int iHit = 0; iHit < numhits; iHit++) { // loop over all hits in the track

        //get hit and detector indices from candidate
        int detId;
        int hitId;
        trackCand->getHit(iHit, detId, hitId);

        //get actual hit data from associated StoreArray
        if (detId == Const::PXD) {
          if (!pxdhits)
            continue;
          const PXDType* hit = pxdhits[hitId];
          if (!hit) {
            B2ERROR("No " << PXDType::Class_Name() << " found at index " << hitId << "!");
            continue;
          }
          addRecoHit(hit, lines);
        } else if (detId == Const::SVD) {
          if (!svdhits)
            continue;
          const SVDType* hit = svdhits[hitId];
          if (!hit) {
            B2ERROR("No " << SVDType::Class_Name() << " found at index " << hitId << "!");
            continue;
          }
          addRecoHit(hit, lines);
        } else if (detId == Const::CDC) {
          if (!cdchits)
            continue;
          const CDCHit* hit = cdchits[hitId];
          if (!hit) {
            B2ERROR("No " << CDCHit::Class_Name() << " found at index " << hitId << "!");
            continue;
          }
          addRecoHit(hit, lines);
        } else {
          B2ERROR("Got invalid detector ID from track candidate, skipping hit!")
          continue;
        }
      }
    }

    if (!m_trackcandlist) {
      m_trackcandlist = new TEveTrackList("Track candidates", m_gftrackpropagator);
      m_trackcandlist->IncDenyDestroy();
    }

    // draw track if corresponding option is set ------------------------------------------
    if (drawTrack) {
      TEveRecTrack rectrack;
      rectrack.fP.Set(track_mom);
      rectrack.fV.Set(track_pos);

      TEveTrack* track_lines = new TEveTrack(&rectrack, m_gftrackpropagator);
      track_lines->SetName(label); //popup label set at end of function
      track_lines->SetPropagator(m_gftrackpropagator);
      track_lines->SetLineColor(trackCandColor);
      track_lines->SetLineWidth(1);
      track_lines->SetTitle(TString::Format("%s\n"
                                            "#hits: %u\n"
                                            "pT=%.3f, pZ=%.3f",
                                            label.Data(), numhits,
                                            track_mom.Pt(), track_mom.Pz()));

      track_lines->SetCharge((int)trackCand->getChargeSeed());


      track_lines->AddElement(lines);
      m_trackcandlist->AddElement(track_lines);
    } else {
      m_trackcandlist->AddElement(lines);
    }
  }
}
#endif
