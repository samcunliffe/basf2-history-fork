/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Milkail Remnev, Dmitry Matvienko                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 ***************************************************************************/

#ifndef ECL_CANVAS_POLAR
#define ECL_CANVAS_POLAR

#include <ecl/modules/eclDisplay/EclPainter.h>
#include <TH2.h>
#include <TCrown.h>
#include <TText.h>
#include <TAxis.h>

namespace Belle2 {
  /**
   * @brief Painter for EclData, polar energy/event_count distribution.
   */
  class EclPainterPolar : public EclPainter {
  public:
    /**
     * @brief Type for polar histogram.
     */
    enum Type {PHI, THETA};

    /**
     * @brief Constructor for EclPainter subclass.
     */
    EclPainterPolar(EclData* data, Type type);
    /**
     * @brief Destructor for EclPainter subclass.
     */
    ~EclPainterPolar();

  private:
    /// Type for polar histogram.
    Type m_type;
    /// Histogram that generates Z-axis.
    TH2F* m_hist;
    /// Phi (or theta) segments of the ECL.
    TCrown** m_segs;
    /// Labels for phi segments.
    TText** m_labels;

    /**
     * @brief Initialize histogram.
     */
    void initHisto();
    /**
     * @brief Convert ECL channel id to id of the phi (theta) segment.
     */
    int channelToSegId(int channel);
    /**
     * @brief Update titles of the histogram.
     */
    void setTitles();

  public:
    /**
     * @brief Sets the information to be displayed in the provided
     * MultilineWidget
     * @param px X coordinate of mouse cursor.
     * @param py Y coordinate of mouse cursor.
     */
    virtual void getInformation(int px, int py, MultilineWidget* panel);

    /**
     * @brief Return subtype of ECLPainterPolar.
     */
    Type getType();

    /**
     * @brief Redraw the canvas.
     */
    virtual void Draw();
  };
}

#endif // ECL_CANVAS_POLAR
