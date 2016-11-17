/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-20XX  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponent3d.h>

#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <cmath>

using namespace std;
using namespace Belle2;
namespace io = boost::iostreams;

void BFieldComponent3d::initialize()
{

  // Input field map
  if (m_mapFilename.empty()) {
    B2ERROR("The filename for the 3d magnetic field component is empty !");
    return;
  }
  string fullPath = FileSystem::findFile("/data/" + m_mapFilename);
  if (!FileSystem::fileExists(fullPath)) {
    B2ERROR("The 3d magnetic field map file '" << m_mapFilename << "' could not be found !");
    return;
  }

  // Options to reduce 3D to 2D
  if (m_mapEnable != "rphiz" && m_mapEnable != "rphi" && m_mapEnable != "phiz" && m_mapEnable != "rz") {
    B2ERROR("BField3d:: enabled coordinates must be \"rphiz\", \"rphi\", \"phiz\" or \"rz\"");
    return;
  }

  // Excluded region
  m_exRegion = true;
  if ((m_exRegionR[0] == m_exRegionR[1]) || (m_exRegionZ[0] == m_exRegionZ[1])) m_exRegion = false;

  // Print initial input parameters
  B2INFO("BField3d:: initial input parameters");
  B2INFO(Form("   map filename:          %s",                   m_mapFilename.c_str()));
  B2INFO(Form("   map dimension:         %s",                   m_mapEnable.c_str()));
  if (m_interpolate) { B2INFO(Form("   map interpolation:     on")); }
  else               { B2INFO(Form("   map interpolation:     off")); }
  B2INFO(Form("   map r pitch & range:   %.2e [%.2e, %.2e] cm", m_gridPitch[0], m_mapRegionR[0], m_mapRegionR[1]));
  B2INFO(Form("   map phi pitch:         %.2e deg",             m_gridPitch[1] * 180 / M_PI));
  B2INFO(Form("   map z pitch & range:   %.2e [%.2e, %.2e] cm", m_gridPitch[2], m_mapRegionZ[0], m_mapRegionZ[1]));
  if (m_exRegion) {
    B2INFO(Form("   map r excluded region: [%.2e, %.2e] cm",    m_exRegionR[0], m_exRegionR[1]));
    B2INFO(Form("   map z excluded region: [%.2e, %.2e] cm",    m_exRegionZ[0], m_exRegionZ[1]));
  }

  m_bmap.reserve(m_mapSize[0]*m_mapSize[1]*m_mapSize[2]);
  // Load B-field map file
  io::filtering_istream fieldMapFile;
  fieldMapFile.push(io::gzip_decompressor());
  fieldMapFile.push(io::file_source(fullPath));

  char tmp[256];
  for (int k = 0; k < m_mapSize[2]; k++) { // z
    for (int i = 0; i < m_mapSize[0]; i++) { // r
      for (int j = 0;  j < m_mapSize[1]; j++) { // phi
        double Br, Bz, Bphi;
        //r[m]  phi[deg]  z[m]  Br[T]  Bphi[T]  Bz[T]
        fieldMapFile.getline(tmp, 256);
        // sscanf(tmp+33,"%lf %lf %lf",&Br,&Bphi,&Bz);
        char* next;
        Br   = strtod(tmp + 33, &next);
        Bphi = strtod(next, &next);
        Bz   = strtod(next, NULL);
        m_bmap.push_back({ -Br, -Bphi, -Bz});
      }
    }
  }

  // Introduce error on B field
  if (m_errRegionR[0] != m_errRegionR[1]) {
    auto it = m_bmap.begin();
    for (int k = 0; k < m_mapSize[2]; k++) { // z
      double r = m_mapRegionR[0];
      for (int i = 0; i < m_mapSize[0]; i++, r += m_gridPitch[0]) { // r
        if (!(r >= m_errRegionR[0] && r < m_errRegionR[1])) { it += m_mapSize[1];  continue;}
        for (int j = 0;  j < m_mapSize[1]; j++) { // phi
          vector3_t& B = *it;
          B.x *= m_errB[0];
          B.y *= m_errB[1];
          B.z *= m_errB[2];
        }
      }
    }
  }

  m_igridPitch[0] = 1 / m_gridPitch[0];
  m_igridPitch[1] = 1 / m_gridPitch[1];
  m_igridPitch[2] = 1 / m_gridPitch[2];

  B2INFO(Form("BField3d:: final map region & pitch: r [%.2e,%.2e] %.2e, phi %.2e, z [%.2e,%.2e] %.2e",
              m_mapRegionR[0], m_mapRegionR[1], m_gridPitch[0], m_gridPitch[1],
              m_mapRegionZ[0], m_mapRegionZ[1], m_gridPitch[2]));
}


TVector3 BFieldComponent3d::calculate(const TVector3& point) const
{
  TVector3 B(0, 0, 0);

  double z = point.z();
  // Check if the point lies inside the magnetic field boundaries
  if (z <= m_mapRegionZ[0] || z >= m_mapRegionZ[1]) return B;
  // Check if the point lies in the exclude region
  if (m_exRegion && (z >= m_exRegionZ[0]) && (z < m_exRegionZ[1])) return B;

  double r2 = point.Perp2();
  // Check if the point lies inside the magnetic field boundaries
  if (r2 < m_mapRegionR[0]*m_mapRegionR[0] || r2 >= m_mapRegionR[1]*m_mapRegionR[1]) return B;
  // Check if the point lies in the exclude region
  if (m_exRegion && (r2 >= m_exRegionR[0]*m_exRegionR[0]) && (r2 < m_exRegionR[1]*m_exRegionR[1])) return B;

  if (r2 > 0) {
    double r = sqrt(r2);
    double phi = atan2(std::abs(point.y()), point.x());

    // Calculate the lower index of the point in the grid
    double wr   = (r - m_mapRegionR[0]) * m_igridPitch[0];
    double wphi =  phi                  * m_igridPitch[1];
    double wz   = (m_mapRegionZ[1] - z) * m_igridPitch[2];

    unsigned int ir   = static_cast<int>(wr);
    unsigned int iphi = static_cast<int>(wphi);
    unsigned int iz   = static_cast<int>(wz);

    if (ir + 1 >= (unsigned int)(m_mapSize[0])) return B;
    if (iphi + 1 >= (unsigned int)(m_mapSize[1])) return B;
    if (iz + 1 >= (unsigned int)(m_mapSize[2])) return B;

    wr   -= ir;
    wphi -= iphi;
    wz   -= iz;

    // Get B-field values from map
    vector3_t b = interpolate(ir, iphi, iz, wr, wphi, wz); // in cylindrical system
    double norm = -1 / r, s = point.y() * norm, c = point.x() * norm;
    vector3_t bc = {b.x* c - b.y * s, b.x* s + b.y * c, b.z};   // in cartesian system
    if (point.y() < 0) bc.y = -bc.y;
    B.SetXYZ(bc.x, bc.y, bc.z);
  } else {
    double wz   = (m_mapRegionZ[1] - z) * m_igridPitch[2];
    unsigned int iz   = static_cast<int>(wz);
    if (iz + 1 >= (unsigned int)(m_mapSize[2])) return B;
    wz   -= iz;

    // Get B-field values from map
    vector3_t b = interpolate(0, 0, iz, 0, 0, wz); // in cylindrical system
    vector3_t bc = {b.x, b.y, b.z}; // in cartesian system
    B.SetXYZ(bc.x, bc.y, bc.z);
  }
  //  B2INFO(Form("%f %f %f %g %g %20.16g", point.x(), point.y(), point.z(), B.x(), B.y(), B.z()));

  return B;
}

void BFieldComponent3d::terminate()
{
}

inline BFieldComponent3d::vector3_t operator*(double a, const BFieldComponent3d::vector3_t& v)
{
  return {v.x * a, v.y * a, v.z * a};
}

inline BFieldComponent3d::vector3_t operator*(const BFieldComponent3d::vector3_t& v, double a)
{
  return {v.x * a, v.y * a, v.z * a};
}

inline BFieldComponent3d::vector3_t operator+(const BFieldComponent3d::vector3_t& v, const BFieldComponent3d::vector3_t& u)
{
  return {v.x + u.x, v.y + u.y, v.z + u.z};
}

BFieldComponent3d::vector3_t BFieldComponent3d::interpolate(unsigned int ir, unsigned int iphi, unsigned int iz, double wr,
                                                            double wphi, double wz) const
{
  double wzd = 1 - wz, wyd = 1 - wphi;

  unsigned int strideZ = m_mapSize[0] * m_mapSize[1];
  unsigned int strideR = m_mapSize[1];

  unsigned int ind0 = iz * strideZ + ir * strideR + iphi;
  unsigned int ind1 = ind0 + strideZ;
  unsigned int ind2 = ind0 + 1;
  unsigned int ind3 = ind1 + 1;
  vector3_t i1 = m_bmap[ind0] * wzd + m_bmap[ind1] * wz;
  vector3_t i2 = m_bmap[ind2] * wzd + m_bmap[ind3] * wz;
  vector3_t w1 = i1 * wyd + i2 * wphi;

  ind0 += strideR;
  ind1 += strideR;
  ind2 += strideR;
  ind3 += strideR;
  vector3_t j1 = m_bmap[ind0] * wzd + m_bmap[ind1] * wz;
  vector3_t j2 = m_bmap[ind2] * wzd + m_bmap[ind3] * wz;
  vector3_t w2 = j1 * wyd + j2 * wphi;

  vector3_t B = w1 * (1 - wr) + w2 * wr;
  // const vector3_t &b = m_bmap[ind0];
  // B2INFO(Form("%g %g %g", b.x, b.y, b.z));

  return B;
}
