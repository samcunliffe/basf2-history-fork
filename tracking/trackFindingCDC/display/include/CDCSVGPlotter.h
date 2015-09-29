#ifndef CDCSVGPLOTTER_H_
#define CDCSVGPLOTTER_H_

#include <tracking/trackFindingCDC/display/EventDataPlotter.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    template<class T>
    class StoreWrappedObjPtr;

    /**
     * Helper class to generated the svg image from the various tracking objects.
     */
    class CDCSVGPlotter {

    public:
      /**
       * Constructor.
       * @param animate Switch to make an animated event display by means of animated SVG.
       *                Animate the display by uncovering the drawn objects in order of their time of flight
       *                This can be seen in most standard browsers. Note however that you should switch of
       *                the wires in this case to reduce the rendering load.
       *                Switch to make an animated event display by means of animated SVG.
       */
      explicit CDCSVGPlotter(bool animate = false);

      /**
       * Make a copy of the current status of the plotter.
       */
      CDCSVGPlotter* clone();

      /**
       * Draws the wires.
       */
      void drawWires(const CDCWireTopology& cdcWireTopology);

      /**
       * Draws the interaction point.
       */
      void drawInteractionPoint();

      /**
       * Draws the outer CDCWall.
       * @param stroke The color to be used for drawing.
       */
      void drawOuterCDCWall(const std::string& stroke);

      /**
       * Draws the inner CDCWall.
       * @param stroke The color to be used for drawing.
       */
      void drawInnerCDCWall(const std::string& stroke);

      /**
       * Draws CDCHits.
       * @param storeArrayName The name of the StoreArray containing the CDCHits.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      void drawCDCHits(const std::string& storeArrayName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draws CDCSimHits.
       * @param storeArrayName The name of the StoreArray containing the CDCSimHits.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCSimHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCSimHit and its id to a stroke-width.
       */
      void drawCDCSimHits(const std::string& storeArrayName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draw the CDCSimHits connected in the order of their getFlightTime for each Monte Carlo particle.
       * @param storeArrayName The name of the StoreArray containing the CDCSimHits.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCSimHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCSimHit and its id to a stroke-width.
       */
      void drawCDCSimHitsConnectByToF(const std::string& storeArrayName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draw GFTrackCands.
       * @param storeArrayName The name of the StoreArray containing the GFTrackCands.
       */
      void drawGFTrackCands(const std::string& storeArrayName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draw GFTrackCandTrajectories.
       * @param storeArrayName The name of the StoreArray containing the GFTrackCands.
       */
      void drawGFTrackCandTrajectories(const std::string& storeArrayName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draw GFTracks.
       * @param storeArrayName The name of the StoreArray containing the GFTracks.
       */
      void drawGFTracks(const std::string& storeArrayName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draws GFTrackTrajectories.
       * @param storeArrayName The name of the StoreArray containing the CDCTracks.
       */
      void drawGFTrackTrajectories(const std::string& storeObjName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draw the axial to stereo segment pairs from Monte Carlo truth.
       * @param storeObjName The name of the StoreVector containing the CDCSegmentPairs.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCSegmentPair and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCSegmentPair and its id to a stroke-width.
       */
      void drawAxialStereoPairs(const std::string& storeObjName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draws CDCTracks.
       * Is only available if the local track finder is in the module chain and
       * specific compile time flags enable the transportation of this data:
       * @param storeObjName The name of the StoreVector containing the CDCTracks.
       */
      void drawCDCTracks(const std::string& storeObjName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draws CDCWireHitClusters.
       * Is only available if the local track finder is in the module chain and
       * specific compile time flags enable the transportation of this data:
       * @param storeObjName The name of the StoreVector containing the CDCWireHitClusters.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      void drawCDCWireHitClusters(const std::string& storeObjName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draws AxialStereoSegmentPairs.
       * Is only available if the local track finder is in the module chain and
       * specific compile time flags enable the transportation of this data:
       * @param storeObjName The name of the StoreVector containing the AxialStereoSegmentPairs.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      void drawAxialStereoSegmentPairs(const std::string& storeObjName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draws MCAxialAxialPairs.
       * Is only available if the local track finder is in the module chain and
       * specific compile time flags enable the transportation of this data:
       * @param storeObjName The name of the StoreVector containing the MCAxialAxialPairs.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      //does probably work correctly (check Filter)
      void drawMCAxialAxialPairs(const std::string& storeObjName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draws MCSegmentTriples.
       * Is only available if the local track finder is in the module chain and
       * specific compile time flags enable the transportation of this data:
       * @param storeObjName The name of the StoreVector containing the MCSegmentTriples.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      //does not work correctly (Filter does not accept triples as argument
      void drawMCSegmentTriples(const std::string& storeObjName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draws CDCRecoSegments.
       * Is only available if the local track finder is in the module chain and
       * specific compile time flags enable the transportation of this data:
       * @param storeObjName The name of the StoreVector containing the CDCRecoSegments.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      void drawCDCRecoSegments(const std::string& storeObjName, const std::string& stroke, const std::string& strokeWidth);

      /**
       * Draws SegmentTrajectories.
       * Is only available if the local track finder is in the module chain and
       * specific compile time flags enable the transportation of this data:
       * @param storeObjName The name of the StoreVector containing the SegmentTrajectories.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      void drawSegmentTrajectories(const std::string& storeObjName, const std::string& stroke, const std::string& strokeWidth);


      /**
       * Save the current dom object representation to disk.
       * @param svgFileName The Name of the saved svg file. Defaults to "display.svg".
       */
      const std::string saveSVGFile(const std::string& svgFileName = "display.svg");

    private:
      /**
       * Function Template for drawing the elements of a given StoreArray. Needs to know the type of the objects in the StoreArray and the right colorizer for these objects.
       * @param storeArray The StoreArray containing the objects.
       * @param storeArrayName The name of the StoreArray containing the objects.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the object and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the object and its id to a stroke-width.
       */
      template<class AColorizer, class AObject>
      void drawStoreArray(const StoreArray<AObject>& storeArray, const std::string& storeArrayName, const std::string& stroke = "",
                          const std::string& strokeWidth = "");

      /**
       * Function Template for drawing the elements of a given StoreVector. Needs to know the type of the objects in the StoreArray and the right colorizer for these objects.
       * @param storeVector The StoreWrappedObjPtr containing the objects.
       * @param storeObjName The name of the storeVector containing the objects.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the object and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the object and its id to a stroke-width.
       */
      template<class AColorizer, class AItem>
      void drawStoreVector(const StoreWrappedObjPtr<std::vector<AItem>>& storeVector, const std::string& storeObjName,
                           const std::string& stroke = "",
                           const std::string& strokeWidth = "");

      /**
       * Draws the object using the Plotter specified in m_eventdataPlotter.
       * @param obj
       * @param obj_attributes
       */
      template<class AObject>
      void draw(const AObject& obj, const std::map<std::string, std::string>& obj_attributes);

      /**
       * Draw every element of an iterable object.
       * @param iterable An iterable object (StoreArray, std::vector...).
       * @param colorizer The AColorizer class, in which the drawing attributes are stored.
       */
      template<class AIterable, class AColorizer>
      void drawIterable(const AIterable& iterable, AColorizer& colorizer);

      bool m_animate = false;
      TrackFindingCDC::EventDataPlotter m_eventdataPlotter;

    };
  }//TrackFindingCDC
}//Belle2


#endif // CDCSVGPLOTTER_H_
