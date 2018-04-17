/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/TrackPrinter.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <TMultiGraph.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string TrackPrinter::getDescription()
{
  return "Findlet for printing out CDCtracks";
}

void TrackPrinter::apply(std::vector<CDCTrack>& tracks)
{
  static int nevent(0);
  TCanvas canv("trackCanvas", "CDC tracks in an event", 0, 0, 800, 600);
  TMultiGraph* mg = new TMultiGraph();
  for (CDCTrack& track : tracks) {
    TGraph* gr = new TGraph();
    gr->SetLineWidth(2);
    gr->SetLineColor(9);
    int pointNumber = 1;
    for (CDCRecoHit3D& hit : track) {
      Vector3D pos = hit.getRecoPos3D();
      const double R = std::sqrt(pos.x() * pos.x() + pos.y() * pos.y());
      const double Z = pos.z();
      gr->SetPoint(pointNumber, Z, R);
      pointNumber++;
    }
    mg->Add(gr);
  }
  canv.cd();
  mg->Draw("APL*");
  canv.SaveAs(Form("CDCtracks_%i.png", nevent));
  nevent++;
}
