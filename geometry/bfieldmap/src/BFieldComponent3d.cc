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
  B2INFO("Memory consumption: " << m_bmap.size()*sizeof(vector3_t) / (1024 * 1024.) << " Mb");
}

/** cos(i*pi/90) i=0..45 */
const double cosTable45[] = {
  1.00000000000000000000, 0.99939082701909573001, 0.99756405025982424761, 0.99452189536827333692,
  0.99026806874157031508, 0.98480775301220805937, 0.97814760073380563793, 0.97029572627599647231,
  0.96126169593831886192, 0.95105651629515357212, 0.93969262078590838405, 0.92718385456678740081,
  0.91354545764260089550, 0.89879404629916699278, 0.88294759285892694203, 0.86602540378443864676,
  0.84804809615642597039, 0.82903757255504169201, 0.80901699437494742410, 0.78801075360672195669,
  0.76604444311897803520, 0.74314482547739423501, 0.71933980033865113936, 0.69465837045899728666,
  0.66913060635885821383, 0.64278760968653932632, 0.61566147532565827967, 0.58778525229247312917,
  0.55919290347074683016, 0.52991926423320495405, 0.50000000000000000000, 0.46947156278589077596,
  0.43837114678907741745, 0.40673664307580020775, 0.37460659341591203541, 0.34202014332566873304,
  0.30901699437494742410, 0.27563735581699918565, 0.24192189559966772256, 0.20791169081775933710,
  0.17364817766693034885, 0.13917310096006544411, 0.10452846326765347140, 0.06975647374412530078,
  0.03489949670250097165, 0.00000000000000000000
};

/**
 * The function getPhiIndxWeightSinCos calculate Phi index using
 * binary division. Since in the current field map we have only 90
 * sectors it has to converge only in 6 bisections. Return how
 * far is point from the sector border to use this as weight in the
 * interpolation as well as sin and cos of the point.
 *
 * @param x x coordinate
 * @param y y coordinate
 * @param r radius coordinate to calculate sin and cos r = sqrt(x^2+y^2)
 * @param w angular distance of the point to the border i in sector units [0..1)
 * @param s sin
 * @param c cos
 * @return index of phi sector from 0 to 89
 */
inline int getPhiIndxWeightSinCos(double y, double x, double r, double& w, double& s, double& c)
{
  double ir = 1 / r;
  s = y * ir, c = x * ir;
  int indx = 0;
  if (x <= 0) { indx += 45; double t = -x; x = y; y = t; }
  bool xgty = x > y;
  if (!xgty) swap(x, y);

  auto sin45 = [](int i)->double{return cosTable45[45 - i];};
  auto cos45 = [](int i)->double{return cosTable45[i];};

  int f = 0, count = 23;
  auto bisect = [&]() -> void {
    int step = count >> 1;
    int i = f + step;
    double v = y * cos45(i) - x * sin45(i);
    if (v >= 0.0)
    {
      f = i + 1;
      count -= step + 1;
    } else {
      count = step;
    }
  };
  int loop = 0;
  while (count > 0 && ++loop < 6) bisect();

  // getw is an approximation of arcsin(phi)*90/pi in range -pi/90<phi<pi/90 better than 1e-15
  auto getw = [](double s)->double {
    static const double c[] = {28.64788975654112, 4.774648293412141, 2.148589615676091, 1.281305635721853};
    double s2 = s * s;
    return s * (c[0] + s2 * (c[1] + s2 * (c[2] + s2 * (c[3]))));
  };

  int i = f - 1;
  w = getw((y * cos45(i) - x * sin45(i)) * ir);
  if (xgty) {
    indx += i;
  } else {
    indx += 44 - i;
    w = 1 - w;
  }
  return indx;
}

namespace {
  /** third order atan2 approximation following
   * http://dx.doi.org/10.1109/CCECE.2006.277814 with a maximum error of
   * 0.0015 radian (0.096 degree)
   */
  double fast_atan2(double y, double x)
  {
    const double ax{std::abs(x)}, ay{std::abs(y)};
    const double z1 = (ay - ax) / (ay + ax);
    const double az1 = std::abs(z1);
    const double phi = M_PI / 4 * (z1 + 1)  - z1 * (az1 - 1) * (0.2447 + 0.0663 * az1);
    if (y >= 0 && x >= 0) return phi;
    if (y >= 0 && x <= 0) return M_PI - phi;
    if (y <= 0 && x <= 0) return -M_PI + phi;
    return -phi;
  }
  // the same as fast_atan2 but branchless
  double fast_atan2_branchless(double y, double x)
  {
    const double pi4 = M_PI / 4, pi2 = M_PI / 2;
    const double ax{std::abs(x)}, ay{std::abs(y)};
    const double z1 = (ax - ay) / (ay + ax);
    const double az1 = std::abs(z1);
    double phi = (z1 + 1) * pi4  - (z1 * (az1 - 1)) * (0.2447 + 0.0663 * az1);
    phi = pi2 - copysign(phi, x);
    return copysign(phi, y);
  }

  /** third order quadrant based atan2 approximation following
   * http://dx.doi.org/10.1109/CCECE.2006.277814 with a maximum error of
   * 0.0015 radian (0.096 degree) and adding rational approximation for small
   * magnitude of y/x following http://dx.doi.org/10.1109/MSP.2006.1628884
   * (combining eq. 10 and eq. 9 in the same way as done in the paper for eq.
   * 11 and eq. 5 in eq. 12)
   */
  double fast_atan2_rational(double y, double x)
  {
    const double ax{std::abs(x)}, ay{std::abs(y)};
    const double z1 = (ay - ax) / (ay + ax);
    const double az1 = std::abs(z1);
    double phi;
    // close to az1=0 the rational approximation yields a smaller error then
    // the polynomial.  the turnover has been estimated by scanning through the
    // valid range 0-1 and finding the spot with the smallest mean(abs(error)).
    if (az1 < 0.232) {
      phi = M_PI / 4 + z1 / (1 + 0.28086 * z1 * z1);
    } else {
      phi = M_PI / 4 * (z1 + 1)  - z1 * (az1 - 1) * (0.2447 + 0.0663 * az1);
    }
    if (y >= 0 && x >= 0) return phi;
    if (y >= 0 && x <= 0) return M_PI - phi;
    if (y <= 0 && x <= 0) return -M_PI + phi;
    return -phi;
  }

  template<int npol>
  double fast_atan2_minimax(double y, double x)
  {
    const double pi4 = M_PI / 4, pi2 = M_PI / 2;
    const double ax = std::abs(x), ay = std::abs(y);
    const double z = (ax - ay) / (ay + ax);
    const double v = std::abs(z), v2 = v * v;
    double p0, p1;
    if (npol == 4) {
      // 4th order polynomial minimax approximation
      // max error <=2.57373e-05 rad (0.00147464 deg)
      static const double c4[] = {0.2132675884368258, 0.23481662556227,
                                  -0.2121597649928347, 0.0485854027042442
                                 };
      p0 = c4[0] + v2 * c4[2];
      p1 = c4[1] + v2 * c4[3];
    } else if (npol == 5) {
      // 5th order polynomial minimax approximation
      // max error <=7.57429e-06 rad (0.000433975 deg)
      static const double c5[] = {0.2141439315495107, 0.2227491783659812, -0.1628994411740733,
                                  -0.02778537455524869, 0.03962954416153075
                                 };
      p0 = c5[0] + v2 * (c5[2] + v2 * c5[4]);
      p1 = c5[1] + v2 * (c5[3]);
    } else if (npol == 6) {
      // 6th order polynomial minimax approximation
      // max error <=4.65134e-07 rad (2.66502e-05 deg)
      static const double c6[] = {0.2145843590230225, 0.2146820003230985, -0.116250549812964,
                                  -0.1428509550362758, 0.1660612278047719, -0.05086851503449636
                                 };
      p0 = c6[0] + v2 * (c6[2] + v2 * (c6[4]));
      p1 = c6[1] + v2 * (c6[3] + v2 * (c6[5]));
    }
    double phi = (z + 1) * pi4  - (z * (v - 1)) * (p0 + v * p1);
    phi = pi2 - copysign(phi, x);
    return copysign(phi, y);
  }

  template<int nfun>
  double opt_atan2(double y, double x)
  {
    if (nfun == 0) return fast_atan2(y, x);
    else if (nfun == 1) return fast_atan2_branchless(y, x);
    else if (nfun == 2) return fast_atan2_rational(y, x);
    else if (nfun == 3) return fast_atan2_minimax<4>(y, x);
    else if (nfun == 4) return fast_atan2_minimax<5>(y, x);
    else if (nfun == 5) return fast_atan2_minimax<6>(y, x);
    return atan2(y, x);
  }

  inline int getPhiIndxWeightSinCos2(double y, double x, double r, double& w, double& s, double& c)
  {
    double phi = opt_atan2<2>(y, x);
    double ir = 1 / r;
    s = y * ir, c = x * ir;
    w = phi * (90 / M_PI);
    int i = w;
    //    i = min(89,i);
    w -= i;
    return i;
  }
}

TVector3 BFieldComponent3d::calculate(const TVector3& point) const
{
  TVector3 B(0, 0, 0);

  double z = point.z();
  // Check if the point lies inside the magnetic field boundaries
  if (z <= m_mapRegionZ[0] || z >= m_mapRegionZ[1]) return B;

  double r2 = point.Perp2();
  // Check if the point lies inside the magnetic field boundaries
  if (r2 < m_mapRegionR[0]*m_mapRegionR[0] || r2 >= m_mapRegionR[1]*m_mapRegionR[1]) return B;
  // Check if the point lies in the exclude region
  if (m_exRegion && (z >= m_exRegionZ[0]) && (z < m_exRegionZ[1]) &&
      (r2 >= m_exRegionR[0]*m_exRegionR[0]) && (r2 < m_exRegionR[1]*m_exRegionR[1])) return B;

  if (r2 > 0) {
    double r = sqrt(r2);

    // Calculate the lower index of the point in the grid
    double wr   = (r - m_mapRegionR[0]) * m_igridPitch[0];
    double wz   = (m_mapRegionZ[1] - z) * m_igridPitch[2];

    double wphi, s, c;
    unsigned int ir   = static_cast<int>(wr);
    unsigned int iphi = getPhiIndxWeightSinCos2(std::abs(point.y()), point.x(), r, wphi, s, c);
    unsigned int iz   = static_cast<int>(wz);

    wr -= ir;
    wz -= iz;

    // Get B-field values from map
    vector3_t b = interpolate(ir, iphi, iz, wr, wphi, wz); // in cylindrical system
    vector3_t bc = { -(b.x * c - b.y * s), -(b.x * s + b.y * c), b.z}; // in cartesian system
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

  return B;
}

void BFieldComponent3d::terminate()
{
}

inline BFieldComponent3d::vector3_t operator*(const BFieldComponent3d::vector3_t& v, double a)
{
  return {v.x * a, v.y * a, v.z * a};
}

inline BFieldComponent3d::vector3_t operator+(const BFieldComponent3d::vector3_t& v, const BFieldComponent3d::vector3_t& u)
{
  return {v.x + u.x, v.y + u.y, v.z + u.z};
}

BFieldComponent3d::vector3_t BFieldComponent3d::interpolate(unsigned int ir, unsigned int iphi, unsigned int iz,
                                                            double wr1, double wphi1, double wz1) const
{
  const unsigned int strideZ = m_mapSize[0] * m_mapSize[1];
  const unsigned int strideR = m_mapSize[1];

  double wz0 = 1 - wz1, wr0 = 1 - wr1, wphi0 = 1 - wphi1;
  unsigned int j000 = iz * strideZ + ir * strideR + iphi;
  unsigned int j001 = j000 + 1;
  unsigned int j010 = j000 + strideR;
  unsigned int j011 = j001 + strideR;
  unsigned int j100 = j000 + strideZ;
  unsigned int j101 = j001 + strideZ;
  unsigned int j110 = j010 + strideZ;
  unsigned int j111 = j011 + strideZ;
  double w00 = wphi0 * wr0, w10 = wphi0 * wr1, w01 = wphi1 * wr0, w11 = wphi1 * wr1;
  const vector<vector3_t>& B = m_bmap;
  vector3_t b =
    (B[j000] * w00 + B[j001] * w01 + B[j010] * w10 + B[j011] * w11) * wz0 +
    (B[j100] * w00 + B[j101] * w01 + B[j110] * w10 + B[j111] * w11) * wz1;
  return b;
}
