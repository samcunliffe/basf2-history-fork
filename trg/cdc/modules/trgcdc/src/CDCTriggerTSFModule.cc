#include "trg/cdc/modules/trgcdc/CDCTriggerTSFModule.h"

#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <cdc/dataobjects/CDCHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <trg/trg/Clock.h>
#include <trg/cdc/Layer.h>
#include <trg/cdc/Wire.h>
#include <trg/cdc/Segment.h>

#define P3D HepGeom::Point3D<double>

using namespace std;
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTriggerTSF);

CDCTriggerTSFModule::CDCTriggerTSFModule() : Module::Module()
{
  setDescription(
    "The Track Segment Finder module of the CDC trigger.\n"
    "Combines CDCHits from the same super layer to CDCTriggerSegmentHits.\n"
  );

  addParam("InnerTSLUTFile",
           m_innerTSLUTFilename,
           "The filename of LUT for track segments from the inner-most super layer",
           string(""));
  addParam("OuterTSLUTFile",
           m_outerTSLUTFilename,
           "The filename of LUT for track segments from the outer super layers",
           string(""));
}

void
CDCTriggerTSFModule::initialize()
{
  // register DataStore elements
  StoreArray<CDCTriggerSegmentHit>::registerPersistent();
  StoreArray<CDCHit>::required();
  // register relations
  StoreArray<CDCTriggerSegmentHit> segmentHits;
  StoreArray<CDCHit> cdcHits;
  StoreArray<MCParticle> mcparticles;
  segmentHits.registerRelationTo(cdcHits);
  mcparticles.registerRelationTo(segmentHits);

  // Prepare track segment shapes.
  // First a structure of wires is created for all layers and super layers.
  // Each track segment consists of pointers to wires in this structure.
  CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
  const unsigned nLayers = cdc.nWireLayers();
  TRGClock clockTDC = TRGClock("CDCTrigger TDC clock", 0, 500. / cdc.getTdcBinWidth());
  TRGClock clockData = TRGClock("CDCTrigger data clock", clockTDC, 1, 16);

  //...Loop over layers...
  int superLayerId = -1;
  vector<TRGCDCLayer*> superLayer;
  unsigned lastNWires = 0;
  int lastShifts = -1000;
  // separate counters for axial and stereo layers and super layers
  int ia = -1;
  int is = -1;
  int ias = -1;
  int iss = -1;
  unsigned axialStereoLayerId = 0;
  unsigned axialStereoSuperLayerId = 0;
  unsigned nWires = 0;
  for (unsigned i = 0; i < nLayers; i++) {
    const unsigned nWiresInLayer = cdc.nWiresInLayer(i);

    //...Axial or stereo?...
    int nShifts = cdc.nShifts(i);
    bool axial = (nShifts == 0);
    if (axial) ++ia;
    else ++is;
    axialStereoLayerId = (axial) ? ia : is;

    //...Is this in a new super layer?...
    if ((lastNWires != nWiresInLayer) || (lastShifts != nShifts)) {
      ++superLayerId;
      superLayer = vector<TRGCDCLayer*>();
      superLayers.push_back(superLayer);
      if (axial) ++ias;
      else ++iss;
      axialStereoSuperLayerId = (axial) ? ias : iss;
      lastNWires = nWiresInLayer;
      lastShifts = nShifts;
    }

    //...Calculate radius of adjacent field wires...
    const float swr = cdc.senseWireR(i);
    const float innerRadius = cdc.fieldWireR(i - 1);
    const float outerRadius = (i < nLayers - 1) ?
                              cdc.fieldWireR(i) :
                              swr + (swr - innerRadius);

    //...New layer...
    TRGCDCLayer* layer = new TRGCDCLayer(i,
                                         superLayerId,
                                         superLayers[superLayerId].size(),
                                         axialStereoLayerId,
                                         axialStereoSuperLayerId,
                                         cdc.zOffsetWireLayer(i),
                                         nShifts,
                                         M_PI * swr * swr / nWiresInLayer,
                                         nWiresInLayer,
                                         innerRadius,
                                         outerRadius);
    superLayer.push_back(layer);

    //...Loop over all wires in a layer...
    for (unsigned j = 0; j < nWiresInLayer; j++) {
      const P3D fp = P3D(cdc.wireForwardPosition(i, j).x(),
                         cdc.wireForwardPosition(i, j).y(),
                         cdc.wireForwardPosition(i, j).z());
      const P3D bp = P3D(cdc.wireBackwardPosition(i, j).x(),
                         cdc.wireBackwardPosition(i, j).y(),
                         cdc.wireBackwardPosition(i, j).z());
      TRGCDCWire* tw = new TRGCDCWire(nWires++, j, *layer, fp, bp, clockTDC);
      layer->push_back(tw);
    }
  }

  //...Make TSF's...
  const unsigned nWiresInTS[2] = {15, 11};
  const int shape[2][30] = {
    {
      -2, 0, // relative layer id, relative wire id
      -1, -1,
      -1, 0,
      0, -1,
      0, 0,
      0, 1,
      1, -2,
      1, -1,
      1, 0,
      1, 1,
      2, -2,
      2, -1,
      2, 0,
      2, 1,
      2, 2
    },
    {
      -2, -1,
      -2, 0,
      -2, 1,
      -1, -1,
      -1, 0,
      0, 0,
      1, -1,
      1, 0,
      2, -1,
      2, 0,
      2, 1,
      0, 0,
      0, 0,
      0, 0,
      0, 0
    }
  };
  const int layerOffset[2] = {5, 2};
  unsigned id = 0;
  unsigned idTS = 0;
  for (unsigned i = 0; i < superLayers.size(); i++) {
    unsigned tsType = (i) ? 1 : 0;

    //...TS layer... w is a central wire
    const TRGCDCCell* ww = superLayers[i][layerOffset[tsType]]->front();
    TRGCDCLayer* layer = new TRGCDCLayer(id++, *ww);
    tsLayers.push_back(layer);

    //...Loop over all wires in a central wire layer...
    const unsigned nWiresInLayer = ww->layer().nCells();
    for (unsigned j = 0; j < nWiresInLayer; j++) {
      const TRGCDCWire& w =
        (TRGCDCWire&) superLayers[i][layerOffset[tsType]]->cell(j);

      const unsigned localId = w.localId();
      const unsigned layerId = w.layerId();
      vector<const TRGCDCWire*> cells;

      for (unsigned k = 0; k < nWiresInTS[tsType]; k++) {
        const unsigned laid = layerId + shape[tsType][k * 2];
        const unsigned loid = localId + shape[tsType][k * 2 + 1];

        const TRGCDCWire& c =
          (TRGCDCWire&) superLayers[i][laid]->cell(loid);

        cells.push_back(&c);
      }

      TRGCDCSegment* ts;
      if (w.superLayerId()) {
        ts = new TRGCDCSegment(idTS++,
                               *layer,
                               w,
                               clockData,
                               m_outerTSLUTFilename,
                               cells);
      } else {
        ts = new TRGCDCSegment(idTS++,
                               *layer,
                               w,
                               clockData,
                               m_innerTSLUTFilename,
                               cells);
      }
      ts->initialize();
      layer->push_back(ts);
    }
  }
}

void
CDCTriggerTSFModule::event()
{
  StoreArray<CDCHit> cdchits;

  // fill CDCHits into track segment shapes
  // ... TODO ...

  // simulate track segments and create track segment hits
  // ... TODO ...
}

void
CDCTriggerTSFModule::terminate()
{

}
