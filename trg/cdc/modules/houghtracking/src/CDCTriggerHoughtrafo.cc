/*************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013-2014  Belle II Collaboration                         *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Michael Schnell, Sara Neuhaus                            *
*                                                                        *
**************************************************************************/

#include <trg/cdc/modules/houghtracking/CDCTriggerHoughtrackingModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>

#include <cmath>

#include <root/TMatrix.h>

/* defines */
#define CDC_SUPER_LAYERS 9

using namespace std;
using namespace Belle2;

unsigned short
CDCTriggerHoughtrackingModule::countSL(bool* layer)
{
  unsigned short lcnt = 0;
  for (int i = 0; i < CDC_SUPER_LAYERS; ++i) {
    if (layer[i] == true) ++lcnt;
  }
  return lcnt;
}

/*
* Find the intercept in hough space.
* In each iteration the Hough plane is divided in quarters.
* For each quarter containing enough hits, the process is repeated
* until the quarters correspond to single Hough cells.
* Return zero on success.
* x = m
* y = a
*/
int
CDCTriggerHoughtrackingModule::fastInterceptFinder(cdcMap& hits,
                                                   double x1_s, double x2_s,
                                                   double y1_s, double y2_s,
                                                   unsigned iterations,
                                                   unsigned ix_s, unsigned iy_s)
{
  string indent = "";
  for (unsigned i = 0; i < iterations; ++i) indent += " ";
  B2DEBUG(150, indent << "intercept finder iteration " << iterations
          << " x1 " << x1_s * 180 / M_PI << " x2 " << x2_s * 180 / M_PI
          << " y1 " << y1_s << " y2 " << y2_s
          << " ix " << ix_s << " iy " << iy_s);

  int i, j, iHit;
  double unitx, unity;
  double y1, y2;
  double m, a;
  cdcPair hp;
  unsigned short iSL;
  // list of hit indices crossing the current rectangle
  vector<unsigned> idx_list;

  // limits of divided rectangles
  double x1_d, x2_d, y1_d, y2_d;

  // cell indices in full plane
  unsigned ix, iy;

  unitx = ((x2_s - x1_s) / 2.0);
  unity = ((y2_s - y1_s) / 2.0);

  StoreObjPtr<TMatrix> plane("HoughPlane");

  // divide x axis in half
  for (i = 0; i < 2 ; ++i) {
    // divide y axis in half
    for (j = 0; j < 2; ++j) {
      x1_d = x1_s + i * unitx;
      x2_d = x1_s + (i + 1) * unitx;
      y1_d = y1_s + j * unity;
      y2_d = y1_s + (j + 1) * unity;

      ix = 2 * ix_s + i;
      iy = 2 * iy_s + j;

      // skip extra cells outside of the Hough plane
      if (x2_d <= -M_PI || x1_d >= M_PI || y2_d <= -maxR || y1_d >= maxR) {
        B2DEBUG(150, indent << "skip Hough cell outside of plane limits");
        continue;
      }

      // if the cell size in phi is too large, the hit calculation is not reliable
      // -> continue to next iteration without hit check
      if (iterations != maxIterations && unitx > M_PI / 2.) {
        fastInterceptFinder(hits, x1_d, x2_d, y1_d, y2_d, iterations + 1, ix, iy);
        continue;
      }

      idx_list.clear();
      bool layerHit[CDC_SUPER_LAYERS] = {false}; /* For layer filter */
      for (auto it = hits.begin(); it != hits.end(); ++it) {
        iHit = it->first;
        hp = it->second;
        iSL = hp.first;
        m = hp.second.X();
        a = hp.second.Y();
        y1 = m * sin(x1_d) - a * cos(x1_d);
        y2 = m * sin(x2_d) - a * cos(x2_d);
        // skip decreasing half of the sine (corresponds to curl back half of the track)
        if (iterations == maxIterations && y1 > y2) continue;
        if (!((y1 > y2_d && y2 > y2_d) || (y1 < y1_d && y2 < y1_d))) {
          if (iterations == maxIterations) {
            idx_list.push_back(iHit);
          }
          layerHit[iSL] = true; /* layer filter */
        }
      }
      unsigned short nSL = countSL(layerHit);
      B2DEBUG(150, indent << "i " << i << " j " << j
              << " layerHit " << int(layerHit[0]) << int(layerHit[2])
              << int(layerHit[4]) << int(layerHit[6]) << int(layerHit[8])
              << " nSL " << nSL);
      if (nSL >= m_minHits) {
        if (iterations != maxIterations) {
          fastInterceptFinder(hits, x1_d, x2_d, y1_d, y2_d, iterations + 1, ix, iy);
        } else {
          TVector2 v1(x1_d, y1_d);
          TVector2 v2(x2_d, y2_d);
          houghCand.push_back(CDCTriggerHoughCand(idx_list, make_pair(v1, v2),
                                                  nSL, houghCand.size()));
          if (m_storePlane > 0) {
            (*plane)[ix - (nCells - m_nCellsPhi) / 2][iy - (nCells - m_nCellsR) / 2] = nSL;
          }
        }
      } else if (m_storePlane > 1) {
        // to store the full Hough plane, we need to continue the iteration
        // to minimal cell size everywhere
        for (unsigned min = m_minHits - 1; min > 0; --min) {
          if (nSL >= min) {
            if (iterations != maxIterations) {
              fastInterceptFinder(hits, x1_d, x2_d, y1_d, y2_d, iterations + 1, ix, iy);
            } else {
              (*plane)[ix - (nCells - m_nCellsPhi) / 2][iy - (nCells - m_nCellsR) / 2] = nSL;
            }
            break;
          }
        }
      }
    }
  }

  return 0;
}

/*
* Peak finding method: connected regions & center of gravity
*/
void
CDCTriggerHoughtrackingModule::connectedRegions()
{
  vector<vector<CDCTriggerHoughCand>> regions;
  vector<CDCTriggerHoughCand> cpyCand = houghCand;

  // debug: print candidate list
  B2DEBUG(50, "houghCand number " << cpyCand.size());
  for (unsigned icand = 0; icand < houghCand.size(); ++icand) {
    coord2dPair coord = houghCand[icand].getCoord();
    B2DEBUG(100, houghCand[icand].getID()
            << " nSL " << houghCand[icand].getSLcount()
            << " x1 " << coord.first.X() << " x2 " << coord.second.X()
            << " y1 " << coord.first.Y() << " y2 " << coord.second.Y());
  }

  // combine connected cells to regions
  while (cpyCand.size() > 0) {
    B2DEBUG(100, "make new region");
    vector<CDCTriggerHoughCand> region;
    vector<CDCTriggerHoughCand> rejected;
    CDCTriggerHoughCand start = cpyCand[0];
    cpyCand.erase(cpyCand.begin());
    region.push_back(start);
    addNeighbors(start, cpyCand, region, rejected, start.getSLcount());
    regions.push_back(region);
    for (auto cand = cpyCand.begin(); cand != cpyCand.end();) {
      if (inList(*cand, region) || inList(*cand, rejected))
        cpyCand.erase(cand);
      else
        ++cand;
    }
  }

  // find center of gravity for each region
  for (unsigned ir = 0; ir < regions.size(); ++ir) {
    B2DEBUG(50, "region " << ir << " (" << regions[ir].size() << " cells).");
    // skip regions with size below cut
    if (regions[ir].size() < m_minCells) {
      B2DEBUG(50, "Skipping region with " << regions[ir].size() << " cells.");
      continue;
    }
    double xfirst = regions[ir][0].getCoord().first.X();
    double x = 0;
    double y = 0;
    int n = 0;
    vector<unsigned> mergedList;
    for (unsigned ir2 = 0; ir2 < regions[ir].size(); ++ir2) {
      coord2dPair hc = regions[ir][ir2].getCoord();
      B2DEBUG(100, "  " << regions[ir][ir2].getID()
              << " nSL " << regions[ir][ir2].getSLcount()
              << " x1 " << hc.first.X() << " x2 " << hc.second.X()
              << " y1 " << hc.first.Y() << " y2 " << hc.second.Y());
      x += (hc.first.X() + hc.second.X());
      if (xfirst - hc.first.X() > M_PI)
        x += 4 * M_PI;
      else if (hc.first.X() - xfirst > M_PI)
        x -= 4 * M_PI;
      y += (hc.first.Y() + hc.second.Y());
      n += 1;
      vector<unsigned> idList = regions[ir][ir2].getIdList();
      mergeIdList(mergedList, mergedList, idList);
    }
    x *= 0.5 / n;
    if (x > M_PI)
      x -= 2 * M_PI;
    else if (x <= -M_PI)
      x += 2 * M_PI;
    y *= 0.5 / n;
    B2DEBUG(50, "x " << x << " y " << y);
    houghTrack.push_back(CDCTriggerHoughTrack(mergedList, TVector2(x, y)));
  }
}

void
CDCTriggerHoughtrackingModule::addNeighbors(const CDCTriggerHoughCand& center,
                                            const vector<CDCTriggerHoughCand>& candidates,
                                            vector<CDCTriggerHoughCand>& merged,
                                            vector<CDCTriggerHoughCand>& rejected,
                                            unsigned short nSLmax) const
{
  for (unsigned icand = 0; icand < candidates.size(); ++icand) {
    B2DEBUG(120, "compare center " << center.getID()
            << " to " << candidates[icand].getID());
    if (inList(candidates[icand], merged) || inList(candidates[icand], rejected)) {
      B2DEBUG(120, "  " << candidates[icand].getID() << " already in list");
      continue;
    }
    if (connected(center, candidates[icand])) {
      if (m_onlyLocalMax && candidates[icand].getSLcount() < nSLmax) {
        B2DEBUG(100, "  lower than highest SLcount, rejected");
        rejected.push_back(candidates[icand]);
      } else if (m_onlyLocalMax && candidates[icand].getSLcount() > nSLmax) {
        B2DEBUG(100, "  new highest SLcount, clearing list");
        nSLmax = candidates[icand].getSLcount();
        for (unsigned imerged = 0; imerged < merged.size(); ++imerged) {
          rejected.push_back(merged[imerged]);
        }
        merged.clear();
        merged.push_back(candidates[icand]);
      } else if (m_onlyLocalMax && candidates[icand].getSLcount() > center.getSLcount()) {
        B2DEBUG(100, "  connected to rejected cell, skip");
        continue;
      } else {
        B2DEBUG(100, "  connected");
        merged.push_back(candidates[icand]);
      }
      vector<CDCTriggerHoughCand> cpyCand = candidates;
      cpyCand.erase(cpyCand.begin() + icand);
      addNeighbors(candidates[icand], cpyCand, merged, rejected, nSLmax);
    }
  }
}

bool
CDCTriggerHoughtrackingModule::inList(const CDCTriggerHoughCand& a,
                                      const vector<CDCTriggerHoughCand>& list) const
{
  for (unsigned i = 0; i < list.size(); ++i) {
    if (a == list[i]) return true;
  }
  return false;
}

bool
CDCTriggerHoughtrackingModule::connected(const CDCTriggerHoughCand& a,
                                         const CDCTriggerHoughCand& b) const
{
  double ax1 = a.getCoord().first.X();
  double ax2 = a.getCoord().second.X();
  double ay1 = a.getCoord().first.Y();
  double ay2 = a.getCoord().second.Y();
  double bx1 = b.getCoord().first.X();
  double bx2 = b.getCoord().second.X();
  double by1 = b.getCoord().first.Y();
  double by2 = b.getCoord().second.Y();
  // direct neighbors
  bool direct = ((ax2 == bx1 && ay1 == by1) || // right
                 (ax1 == bx2 && ay1 == by1) || // left
                 (ax1 == bx1 && ay2 == by1) || // above
                 (ax1 == bx1 && ay1 == by2) || // below
                 (ax1 + 2. * M_PI == bx2 && ay1 == by1) ||
                 (ax2 == bx1 + 2. * M_PI && ay1 == by1));
  // diagonal connections
  bool diagRise = ((ax2 == bx1 && ay2 == by1) || // right above
                   (ax1 == bx2 && ay1 == by2) || // left below
                   (ax1 + 2. * M_PI == bx2 && ay1 == by2) ||
                   (ax2 == bx1 + 2. * M_PI && ay2 == by1));
  bool diagFall = ((ax1 == bx2 && ay2 == by1) || // left above
                   (ax2 == bx1 && ay1 == by2) || // right below
                   (ax2 == bx1 + 2. * M_PI && ay1 == by2) ||
                   (ax1 + 2. * M_PI == bx2 && ay2 == by1));
  if (m_connect == 4) return direct;
  else if (m_connect == 6) return (direct || diagRise);
  else if (m_connect == 8) return (direct || diagRise || diagFall);
  else B2WARNING("Unknown option for connect " << m_connect << ", using default.");
  return (direct || diagRise);
}

/*
* Merge Id lists.
*/
void
CDCTriggerHoughtrackingModule::mergeIdList(std::vector<unsigned>& merged,
                                           std::vector<unsigned>& a,
                                           std::vector<unsigned>& b)
{
  bool found;

  for (auto it = a.begin(); it != a.end(); ++it) {
    found = false;
    for (auto it_in = merged.begin(); it_in != merged.end(); ++it_in) {
      if (*it_in == *it) {
        found = true;
        break;
      }
    }
    if (!found) {
      merged.push_back(*it);
    }
  }

  for (auto it = b.begin(); it != b.end(); ++it) {
    found = false;
    for (auto it_in = merged.begin(); it_in != merged.end(); ++it_in) {
      if (*it_in == *it) {
        found = true;
        break;
      }
    }
    if (!found) {
      merged.push_back(*it);
    }
  }
}
