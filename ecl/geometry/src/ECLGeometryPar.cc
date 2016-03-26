/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/geometry/ECLGeometryPar.h>
#include <framework/logging/Logger.h>

#include <G4VTouchable.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4NavigationHistory.hh>

#include "TVector3.h"

using namespace std;
using namespace Belle2::ECL;

ECLGeometryPar* ECLGeometryPar::m_B4ECLGeometryParDB = 0;

class Mapping_t {
public:
  static void Mapping(int id, int& ThetaId, int& PhiId)
  {
    ThetaId = m_Theta[((unsigned int)id) >> 4];
    PhiId = id - m_dTheta[ThetaId] * 16 - ThetaId * 128;
  }

  static void Mapping(int id, int& ThetaId, int& PhiId, int& nrep, int& indx)
  {
    Mapping(id, ThetaId, PhiId);

    int off = m_offsets[ThetaId];
    int i = m_tbl[ThetaId];

    int r = m_recip[i];
    int d = m_denom[i];

    nrep = (PhiId * r) >> m_RECIPROCAL_SHIFT;
    indx = off + (PhiId - nrep * d);
  }

  static int CellID(int ThetaId, int PhiId)
  {
    return PhiId + m_dTheta[ThetaId] * 16 + ThetaId * 128;
  }

  static int Offset(int ThetaId)
  {
    return m_dTheta[ThetaId] + ThetaId * 8;
  }

  static int Indx2ThetaId(int indx)
  {
    return m_Theta[indx];
  }

  static int ThetaId2NCry(int ThetaId)  // Theta Id to the number of crystals @ this Id
  {
    return m_denom[m_tbl[ThetaId]];
  }

private:
  static const char m_dTheta[69];
  static const unsigned char m_Theta[546], m_tbl[69], m_offsets[69];

  static const unsigned char m_RECIPROCAL_SHIFT = 16;
  static const unsigned int m_recip[5], m_denom[5];
};

const unsigned char Mapping_t::m_Theta[546] = {
  0, 0, 0,  1, 1, 1,
  2, 2, 2, 2,  3, 3, 3, 3,  4, 4, 4, 4,
  5, 5, 5, 5, 5, 5,  6, 6, 6, 6, 6, 6,  7, 7, 7, 7, 7, 7,
  8, 8, 8, 8, 8, 8,  9, 9, 9, 9, 9, 9,  10, 10, 10, 10, 10, 10,
  11, 11, 11, 11, 11, 11, 11, 11, 11,  12, 12, 12, 12, 12, 12, 12, 12, 12,

  13, 13, 13, 13, 13, 13, 13, 13, 13,  14, 14, 14, 14, 14, 14, 14, 14, 14,  15, 15, 15, 15, 15, 15, 15, 15, 15,
  16, 16, 16, 16, 16, 16, 16, 16, 16,  17, 17, 17, 17, 17, 17, 17, 17, 17,  18, 18, 18, 18, 18, 18, 18, 18, 18,
  19, 19, 19, 19, 19, 19, 19, 19, 19,  20, 20, 20, 20, 20, 20, 20, 20, 20,  21, 21, 21, 21, 21, 21, 21, 21, 21,
  22, 22, 22, 22, 22, 22, 22, 22, 22,  23, 23, 23, 23, 23, 23, 23, 23, 23,  24, 24, 24, 24, 24, 24, 24, 24, 24,
  25, 25, 25, 25, 25, 25, 25, 25, 25,  26, 26, 26, 26, 26, 26, 26, 26, 26,  27, 27, 27, 27, 27, 27, 27, 27, 27,
  28, 28, 28, 28, 28, 28, 28, 28, 28,  29, 29, 29, 29, 29, 29, 29, 29, 29,  30, 30, 30, 30, 30, 30, 30, 30, 30,
  31, 31, 31, 31, 31, 31, 31, 31, 31,  32, 32, 32, 32, 32, 32, 32, 32, 32,  33, 33, 33, 33, 33, 33, 33, 33, 33,
  34, 34, 34, 34, 34, 34, 34, 34, 34,  35, 35, 35, 35, 35, 35, 35, 35, 35,  36, 36, 36, 36, 36, 36, 36, 36, 36,
  37, 37, 37, 37, 37, 37, 37, 37, 37,  38, 38, 38, 38, 38, 38, 38, 38, 38,  39, 39, 39, 39, 39, 39, 39, 39, 39,
  40, 40, 40, 40, 40, 40, 40, 40, 40,  41, 41, 41, 41, 41, 41, 41, 41, 41,  42, 42, 42, 42, 42, 42, 42, 42, 42,
  43, 43, 43, 43, 43, 43, 43, 43, 43,  44, 44, 44, 44, 44, 44, 44, 44, 44,  45, 45, 45, 45, 45, 45, 45, 45, 45,
  46, 46, 46, 46, 46, 46, 46, 46, 46,  47, 47, 47, 47, 47, 47, 47, 47, 47,  48, 48, 48, 48, 48, 48, 48, 48, 48,
  49, 49, 49, 49, 49, 49, 49, 49, 49,  50, 50, 50, 50, 50, 50, 50, 50, 50,  51, 51, 51, 51, 51, 51, 51, 51, 51,
  52, 52, 52, 52, 52, 52, 52, 52, 52,  53, 53, 53, 53, 53, 53, 53, 53, 53,  54, 54, 54, 54, 54, 54, 54, 54, 54,
  55, 55, 55, 55, 55, 55, 55, 55, 55,  56, 56, 56, 56, 56, 56, 56, 56, 56,  57, 57, 57, 57, 57, 57, 57, 57, 57,
  58, 58, 58, 58, 58, 58, 58, 58, 58,

  59, 59, 59, 59, 59, 59, 59, 59, 59,  60, 60, 60, 60, 60, 60, 60, 60, 60,
  61, 61, 61, 61, 61, 61,  62, 62, 62, 62, 62, 62,
  63, 63, 63, 63, 63, 63,  64, 64, 64, 64, 64, 64,
  65, 65, 65, 65, 65, 65,
  66, 66, 66, 66,  67, 67, 67, 67,  68, 68, 68, 68
};

const char Mapping_t::m_dTheta[69] = {
  0, -5, -10, -14, -18, -22, -24, -26, -28, -30, -32, -34, -33, // forward

  -32, -31, -30, -29, -28, -27, -26, -25, -24,
  -23, -22, -21, -20, -19, -18, -17, -16, -15,
  -14, -13, -12, -11, -10,  -9,  -8,  -7,  -6,
  -5,  -4,  -3,  -2,  -1,   0,   1,   2,   3,
  4,   5,   6,   7,   8,   9,  10,  11,  12, 13,

  14, 15, 16, 14, 12, 10, 8, 6, 2, -2 // backward
};

const unsigned char Mapping_t::m_tbl[69] = { // pointer to the denominator/reciprocal arrays or how many crystals in a phi sector
  1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, // forward

  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  4, 4, 3, 3, 3, 3, 3, 2, 2, 2           // backward
};

const unsigned char Mapping_t::m_offsets[69] = {
  0,  3,  6, 10, 14, 18, 24, 30, 36, 42, 48, 54, 63, // forward

  132, 134, 136, 138, 140, 142, 144, 146, 148,
  150, 152, 154, 156, 158, 160, 162, 164, 166,
  168, 170, 172, 174, 176, 178, 180, 182, 184,
  186, 188, 190, 192, 194, 196, 198, 200, 202,
  204, 206, 208, 210, 212, 214, 216, 218, 220, 222,

  72, 81, 90, 96, 102, 108, 114, 120, 124, 128       // backward
};

#define pack(x) ((1<<Mapping_t::m_RECIPROCAL_SHIFT)/x+1)
const unsigned int Mapping_t::m_recip[] = {pack(2), pack(3), pack(4), pack(6), pack(9)};
const unsigned int Mapping_t::m_denom[] = {    (2), (3), (4), (6), (9)};
#undef pack


ECLGeometryPar* ECLGeometryPar::Instance()
{
  if (!m_B4ECLGeometryParDB) m_B4ECLGeometryParDB = new ECLGeometryPar();
  return m_B4ECLGeometryParDB;
}

ECLGeometryPar::ECLGeometryPar()
{
  clear();
  read();
}

ECLGeometryPar::~ECLGeometryPar()
{
  if (m_B4ECLGeometryParDB) {
    delete m_B4ECLGeometryParDB;
    B2INFO("m_B4ECLGeometryParDB deleted ");
  }
}

void ECLGeometryPar::clear()
{
  m_ini_cid = -1;
  mPar_cellID = 0;
  mPar_thetaID = 0;
  mPar_phiID = 0;
}

const double sectorPhi[2] = {M_PI / 36, M_PI / 8};

// There is no way to get world coordinates of a local point of a physical volume in Geant.
// The only way to check geometry is to trace particle and check volumes it crosses.
// Here particle gun parameters are produced to check crystal positions with the center @ r0 and direction n
void ParticleGunParameters(const G4String& comment, const G4ThreeVector& n, const G4ThreeVector& r0, double dphi)
{
  cout << comment << endl;
  cout << "Center position = " << r0 << ", Direction = " << n << endl;
  // closest point to z-axis
  double t = -(n.z() * r0.z() - n * r0) / (n.z() * n.z() - n * n);
  G4ThreeVector r = n * t + r0;
  cout << "Closest point to z-axis = " << r << endl; // at the moment I do not see tilt in phi
  const double r2d = 180 / M_PI;
  double th = r2d * n.theta();
  double phi = r2d * n.phi();
  double z = r.z();
  dphi *= r2d;
  cout << "    'thetaParams': [" << th << ", " << th << "]," << endl;
  cout << "    'phiParams': [" << phi << "+0*" << dphi << ", " << phi << "+0*" << dphi << "]," << endl;
  cout << "    'zVertexParams': [" << z << ", " << z << "]" << endl;
}

void ECLGeometryPar::read()
{
  m_crystals.clear();
  m_crystals.reserve(46 * 2 + 132); // 10752 bytes

  G4PhysicalVolumeStore* store = G4PhysicalVolumeStore::GetInstance();
  G4ThreeVector a0(0, 0, 0), a1(0, 0, 1);

  // Endcap sectors are rotated since behaviour of G4Replica class. It
  // requires a physical volume during phi replication to be symmetric
  // around phi=0. So we need to rotate it by -((2*pi)/16)/2 angle at the
  // geometry description are return back here.
  G4RotationMatrix H = CLHEP::HepRotationZ(sectorPhi[1] * 0.5);

  G4String tnamef("eclFwdFoilPhysical_");
  for (int i = 1; i <= 72; i++) {
    G4String vname(tnamef); vname += to_string(i);
    G4VPhysicalVolume* v0 = store->GetVolume(vname);
    G4RotationMatrix m0 = v0->GetObjectRotationValue();
    G4ThreeVector d0 = v0->GetObjectTranslation();
    CrystalGeom_t c0 = {(1 / CLHEP::cm)* (H * d0), H* (m0 * a1)};
    m_crystals.push_back(c0);
    // const CrystalGeom_t &t = m_crystals.back();
    // ParticleGunParameters(string("Forward ")+to_string(i),t.dir,t.pos,sectorPhi[1]);
  }

  G4String tnameb("eclBwdFoilPhysical_");
  for (int i = 73; i <= 132; i++) {
    G4String vname(tnameb); vname += to_string(i);
    G4VPhysicalVolume* v0 = store->GetVolume(vname);
    G4RotationMatrix m0 = v0->GetObjectRotationValue();
    G4ThreeVector d0 = v0->GetObjectTranslation();
    CrystalGeom_t c0 = {(1 / CLHEP::cm)* (H * d0), H* (m0 * a1)};
    m_crystals.push_back(c0);
    // const CrystalGeom_t &t = m_crystals.back();
    // ParticleGunParameters(string("Backward ")+to_string(i),t.dir,t.pos,sectorPhi[1]);
  }

  // since barrel sector is symmetric around phi=0 we need to
  // translate crystal with negative phi back to positive rotating
  // crystal position by (2*M_PI/72) angle
  G4RotationMatrix S = CLHEP::HepRotationZ(sectorPhi[0]);

  // get barrel sector (between two septums) transformation
  G4VPhysicalVolume* bs = store->GetVolume("eclBarrelCrystalSectorPhysical");
  G4RotationMatrix mbs = bs->GetObjectRotationValue();
  G4ThreeVector dbs = bs->GetObjectTranslation();

  G4String tnameb0("eclBarrelFoilPhysical_0_"), tnameb1("eclBarrelFoilPhysical_1_");
  for (int i = 1; i <= 46; i++) {
    G4String istr(to_string(i));

    G4String vname0(tnameb0); vname0 += istr;
    G4VPhysicalVolume* v0 = store->GetVolume(vname0);
    G4RotationMatrix m0 = v0->GetObjectRotationValue();
    G4ThreeVector d0 = v0->GetObjectTranslation();
    CrystalGeom_t c0 = {(1 / CLHEP::cm)* (mbs * d0 + dbs), mbs* (m0 * a1)};
    m_crystals.push_back(c0);
    // const CrystalGeom_t &t0 = m_crystals.back();
    // ParticleGunParameters(string("Barrel0 ")+to_string(i),t0.dir,t0.pos,sectorPhi[1]);

    G4String vname1(tnameb1); vname1 += istr;
    G4VPhysicalVolume* v1 = store->GetVolume(vname1);
    G4RotationMatrix m1 = v1->GetObjectRotationValue();
    G4ThreeVector d1 = v1->GetObjectTranslation();
    CrystalGeom_t c1 = {(1 / CLHEP::cm)* (S * (mbs * d1 + dbs)), S* (mbs * (m1 * a1))};
    m_crystals.push_back(c1);
    // const CrystalGeom_t &t1 = m_crystals.back();
    // ParticleGunParameters(string("Barrel1 ")+to_string(i),t1.dir,t1.pos,sectorPhi[1]);
  }
  B2INFO("ECLGeometryPar::read() initialized with " << m_crystals.size() << " crystals.");
}

TVector3 geom_pos, geom_vec;

void ECLGeometryPar::InitCrystal(int cid)
{
  int thetaid, phiid, nreplica, indx;
  Mapping_t::Mapping(cid, thetaid, phiid, nreplica, indx);
  int isect = indx > 131 ? 0 : 1;
  const CrystalGeom_t& t = m_crystals[indx];
  double phi = nreplica * sectorPhi[isect], s, c;
  sincos(phi, &s, &c);
  // cout<<cid<<" "<<thetaid<<" "<<phiid<<" "<<nreplica<<" "<<indx<<" "<<isect<<" "<<sectorPhi[isect]<<" "<<phi<<" "<<s<<" "<<c<<endl;

  double xp = c * t.pos.x() - s * t.pos.y();
  double yp = s * t.pos.x() + c * t.pos.y();
  geom_pos.SetXYZ(xp, yp, t.pos.z());

  double xv = c * t.dir.x() - s * t.dir.y();
  double yv = s * t.dir.x() + c * t.dir.y();
  geom_vec.SetXYZ(xv, yv, t.dir.z());

  m_ini_cid = cid;
}

const TVector3& ECLGeometryPar::GetCrystalPos(int cid)
{
  if (cid != m_ini_cid) InitCrystal(cid);
  return geom_pos;
}

const TVector3& ECLGeometryPar::GetCrystalVec(int cid)
{
  if (cid != m_ini_cid) InitCrystal(cid);
  return geom_vec;
}

int ECLGeometryPar::GetCellID(int ThetaId, int PhiId)
{
  return Mapping_t::CellID(ThetaId, PhiId);
}

void ECLGeometryPar::Mapping(int cid)
{
  mPar_cellID = cid;
  Mapping_t::Mapping(mPar_cellID, mPar_thetaID, mPar_phiID);
}

int ECLGeometryPar::TouchableToCellID(const G4VTouchable* touch)
{
  //  touch->GetCopyNumber() is a virtual call, avoid it by using
  //  directly G4NavigationHistory so we will have only one virtual
  //  call instead of three here
  const G4NavigationHistory* h = touch->GetHistory();
  int hd = h->GetDepth();
  int indx     = h->GetReplicaNo(hd - 0); // index of each volume is set at physical volume creation
  int NReplica = h->GetReplicaNo(hd - 2); // go up in volume hierarchy
  //  int indx     = touch->GetCopyNumber(0); // index of each volume is set at physical volume creation
  //  int NReplica = touch->GetCopyNumber(2); // go up in volume hierarchy

  int ThetaId  = Mapping_t::Indx2ThetaId(indx);
  int NCryst   = Mapping_t::ThetaId2NCry(ThetaId); // the number of crystals in a sector at given ThetaId
  int Offset   = Mapping_t::Offset(ThetaId);
  int PhiId    = NReplica * NCryst + (indx - Offset);
  if (NCryst == 2) { // barrel
    PhiId -= h->GetReplicaNo(hd - 1);
    //    PhiId -= touch->GetCopyNumber(1);
    if (PhiId == -1) PhiId = 143; // this can occure only @ NReplica==0
  }
  int cellID = Offset * 16 + PhiId;
  //  cout<<"ECLGeometryPar::TouchableToCellID "<<indx<<" "<<NReplica<<" "<<ThetaId<<" "<<NCryst<<" "<<Offset<<" "<<PhiId<<endl;

  // test of the position and direction of crystal
  if (0) {
    G4AffineTransform t = h->GetTopTransform();
    G4ThreeVector o(0, 0, 0), n(0, 0, 1);
    G4ThreeVector ro = t.Inverse().TransformPoint(o);
    G4ThreeVector rn = t.Inverse().TransformAxis(n);

    InitCrystal(cellID);
    ro *= 1 / CLHEP::cm;
    double drx = geom_pos.X() - ro.x(), dry = geom_pos.Y() - ro.y(), drz = geom_pos.Z() - ro.z();
    double dnx = geom_vec.X() - rn.x(), dny = geom_vec.Y() - rn.y(), dnz = geom_vec.Z() - rn.z();

    G4ThreeVector dr(drx, dry, drz), dn(dnx, dny, dnz);
    if (dr.mag() > 1e-10 || dn.mag() > 1e-10) {
      cout << NReplica << " " << indx << " " << PhiId << " " << ro << " " << rn << " " << dr << " " << dn << endl;
    }
  }

  return cellID;
}

int ECLGeometryPar::ECLVolumeToCellID(const G4VTouchable* touch)
{
  int depth = touch->GetHistoryDepth();
  if ((depth != 5) && (depth != 6)) {
    B2WARNING("ECLGeometryPar::ECLVolumeToCellID: History depth = " << depth << " is out of range: should be 5 or 6.");
    return -1;
  }
  const G4String& vname = touch->GetVolume()->GetName();
  std::size_t pos = vname.find("CrystalPhysical_");
  if (pos == string::npos) {
    B2WARNING("ECLGeometryPar::ECLVolumeToCellID: Volume name does not match pattern. NAME=" << vname);
    return -1;
  }
  return TouchableToCellID(touch);
}

EclNbr::EclNbr() :
  m_nbrs(*new std::vector< Identifier >)
{
  mNbr_cellID = 0;
  mNbr_thetaID = 0;
  mNbr_phiID = 0;
}

EclNbr::EclNbr(const EclNbr& aNbr) :
  m_nbrs(*new std::vector< Identifier > (aNbr.m_nbrs)) ,
  m_nearSize(aNbr.m_nearSize)
{
  mNbr_cellID = 0;
  mNbr_thetaID = 0;
  mNbr_phiID = 0;
}

EclNbr::EclNbr(
  const std::vector< Identifier >&           aNbrs     ,
  const std::vector< Identifier >::size_type aNearSize
) :
  m_nbrs(*new std::vector< Identifier > (aNbrs)) ,
  m_nearSize(aNearSize)
{
  // sort vector separately for near, nxt-near nbrs
  std::sort(m_nbrs.begin() , m_nbrs.begin() + aNearSize , std::less< Identifier >()) ;
  std::sort(m_nbrs.begin() + aNearSize ,   m_nbrs.end() , std::less< Identifier >()) ;
}

EclNbr::~EclNbr()
{
  delete &m_nbrs ;
}
std::ostream& operator<<(std::ostream& os, const EclNbr& aNbr)
{
  os << "N(" ;
  unsigned short i(0) ;
  for (std::vector< EclNbr::Identifier >::const_iterator
       iNbr(aNbr.nbrs().begin()) ;
       iNbr != aNbr.nbrs().end() ; ++iNbr) {
    ++i;
    if (iNbr != aNbr.nbrs().begin() && i != aNbr.nearSize() + 1) os << "," ;
    if (i == aNbr.nearSize() + 1) os << "|" ;
    os << std::setw(4) << (*iNbr) ;
  }
  os << ")" ;
  return os ;
}


void EclNbr::printNbr()
{
  unsigned short Nri(0) ;
  cout << "(";
  for (std::vector< EclNbr::Identifier >::const_iterator
       iNbr(m_nbrs.begin()) ;
       iNbr != m_nbrs.end() ; ++iNbr) {
    ++Nri;
    if (iNbr != m_nbrs.begin() && Nri != m_nearSize + 1) cout << "," ;
    if (Nri == m_nearSize + 1) cout << "|" ;
    cout << std::setw(4) << (*iNbr) ;
  }
  cout << ")" << endl;
}
//
// assignment operators


EclNbr& EclNbr::operator=(const EclNbr& aNbr)
{
  if (this != &aNbr) {
    mNbr_cellID  = aNbr.mNbr_cellID;
    mNbr_thetaID = aNbr.mNbr_thetaID;
    mNbr_phiID   = aNbr.mNbr_phiID;

    m_nbrs     = aNbr.m_nbrs     ;
    m_nearSize = aNbr.m_nearSize ;
  }
  return *this ;
}

//
// member functions
//

//
// const member functions
//

const std::vector< EclNbr::Identifier >&
EclNbr::nbrs()      const
{
  return m_nbrs ;
}

const std::vector< EclNbr::Identifier >::const_iterator
EclNbr::nearBegin() const
{
  return m_nbrs.begin() ;
}

const std::vector< EclNbr::Identifier >::const_iterator
EclNbr::nearEnd()   const
{
  return m_nbrs.begin() + m_nearSize ;
}

const std::vector< EclNbr::Identifier >::const_iterator
EclNbr::nextBegin() const
{
  return m_nbrs.begin() + m_nearSize ;
}

const std::vector< EclNbr::Identifier >::const_iterator
EclNbr::nextEnd()   const
{
  return m_nbrs.end() ;
}

std::vector< EclNbr::Identifier >::size_type
EclNbr::nearSize()  const
{
  return m_nearSize ;
}

std::vector< EclNbr::Identifier >::size_type
EclNbr::nextSize()  const
{
  return (m_nbrs.size() - m_nearSize) ;
}


int EclNbr::GetCellID(int ThetaId, int PhiId)
{
  mNbr_cellID = Mapping_t::CellID(ThetaId, PhiId);
  mNbr_thetaID = ThetaId;
  mNbr_phiID =  PhiId ;
  return mNbr_cellID;
}

void EclNbr::Mapping(int cid)
{
  mNbr_cellID = cid;
  Mapping_t::Mapping(mNbr_cellID, mNbr_thetaID, mNbr_phiID);
}

EclNbr
EclNbr::getNbr(const Identifier aCellId)
{
  // generate nbr lists. always easier here to work with theta-phi

  const int cellID = aCellId;
  Mapping(cellID);
  const int thetaId = GetThetaID();
  const int phiId = GetPhiID();
  std::vector< EclNbr::Identifier >::size_type nearSize(0);
  std::vector< EclNbr::Identifier > vNbr;

  vNbr.reserve(24) ;   // except for extreme endcaps, always 24

  int t00 = thetaId;
  int tm1 = thetaId - 1;
  int tm2 = thetaId - 2;
  int tp1 = thetaId + 1;
  int tp2 = thetaId + 2;

  if (aCellId > 1151 && aCellId < 7776) {
    // barrel
    //
    //   12 13 14 15 16      ^ theta
    //   11  2  3  4 17      |
    //   10  1  0  5 18      +--> phi    X--+    view from inside
    //    9  8  7  6 19                     |    (foot pointing e- dir)
    //   24 23 22 21 20                     Z
    int f00 = phiId;
    int fm1 = (phiId + 143) % 144;
    int fp1 = (phiId + 1) % 144;
    int fm2 = (phiId + 142) % 144;
    int fp2 = (phiId + 2) % 144;

    vNbr.push_back(GetCellID(t00 , fm1));
    vNbr.push_back(GetCellID(tp1 , fm1));
    vNbr.push_back(GetCellID(tp1 , f00));
    vNbr.push_back(GetCellID(tp1 , fp1));
    vNbr.push_back(GetCellID(t00 , fp1));
    vNbr.push_back(GetCellID(tm1 , fp1));
    vNbr.push_back(GetCellID(tm1 , f00));
    vNbr.push_back(GetCellID(tm1 , fm1));

    nearSize = vNbr.size();

    vNbr.push_back(GetCellID(tm1 , fm2));
    vNbr.push_back(GetCellID(t00 , fm2));
    vNbr.push_back(GetCellID(tp1 , fm2));
    vNbr.push_back(GetCellID(tp2 , fm2));
    vNbr.push_back(GetCellID(tp2 , fm1));
    vNbr.push_back(GetCellID(tp2 , f00));
    vNbr.push_back(GetCellID(tp2 , fp1));
    vNbr.push_back(GetCellID(tp2 , fp2));
    vNbr.push_back(GetCellID(tp1 , fp2));
    vNbr.push_back(GetCellID(t00 , fp2));
    vNbr.push_back(GetCellID(tm1 , fp2));
    vNbr.push_back(GetCellID(tm2 , fp2));
    vNbr.push_back(GetCellID(tm2 , fp1));
    vNbr.push_back(GetCellID(tm2 , f00));
    vNbr.push_back(GetCellID(tm2 , fm1));
    vNbr.push_back(GetCellID(tm2 , fm2));
  }//if( aCellId > 1152 && aCellId < 7777 )
  else {
    // endcap -- not always 24!
    int n00 = 1000;
    int np1 = 1000;
    int np2 = 1000;
    int nm1 = 1000;
    int nm2 = 1000;
    if (aCellId < 1153) { // forward
      const EclIdentifier mPerRingForward[]
        = { 48, 48, 64, 64, 64, 96, 96, 96, 96, 96, 96, 144, 144, 144, 144 };
      if (thetaId > 1) nm2 = mPerRingForward[ thetaId - 2 ];
      if (thetaId > 0) nm1 = mPerRingForward[ thetaId - 1 ];
      n00 = mPerRingForward[ thetaId     ];
      np1 = mPerRingForward[ thetaId + 1 ];
      np2 = mPerRingForward[ thetaId + 2 ];
    } else { // backward
      const EclIdentifier mPerRingBackward[]
        = { 64, 64, 64, 96, 96, 96, 96, 96, 144, 144, 144, 144 };
      if (thetaId < 67) np2 = mPerRingBackward[ 66 - thetaId ];
      if (thetaId < 68) np1 = mPerRingBackward[ 67 - thetaId ];
      n00 = mPerRingBackward[ 68 - thetaId ];
      nm1 = mPerRingBackward[ 69 - thetaId ];
      nm2 = mPerRingBackward[ 70 - thetaId ];
    }
    // f-- are phi's, t-- are thetas
    // all calculations should be integer arith - pcs
    // f(th,phi)
    // criterion: center -> next bin
    int f0000 = phiId;
    int fp100 = (f0000 * np1 + np1 / 2) / n00;
    int fp200 = (f0000 * np2 + np2 / 2) / n00;
    int fm100 = (f0000 * nm1 + nm1 / 2) / n00;
    int fm200 = (f0000 * nm2 + nm2 / 2) / n00;

    int f00m1 = (f0000 + n00 - 1) % n00;  // should be exact
    int f00m2 = (f0000 + n00 - 2) % n00;
    int f00p1 = (f0000 + 1) % n00;
    int f00p2 = (f0000 + 2) % n00;

    int fp1m1 = (fp100 + np1 - 1) % np1;
    int fp1m2 = (fp100 + np1 - 2) % np1;
    int fp1p1 = (fp100 + 1) % np1;
    int fp1p2 = (fp100 + 2) % np1;

    int fm1m1 = (fm100 + nm1 - 1) % nm1;
    int fm1m2 = (fm100 + nm1 - 2) % nm1;
    int fm1p1 = (fm100 + 1) % nm1;
    int fm1p2 = (fm100 + 2) % nm1;

    int fp2m1 = (fp200 + np2 - 1) % np2;
    int fp2m2 = (fp200 + np2 - 2) % np2;
    int fp2p1 = (fp200 + 1) % np2;
    int fp2p2 = (fp200 + 2) % np2;

    int fm2m1 = (fm200 + nm2 - 1) % nm2;
    int fm2m2 = (fm200 + nm2 - 2) % nm2;
    int fm2p1 = (fm200 + 1) % nm2;
    int fm2p2 = (fm200 + 2) % nm2;
    int delta = n00 / 16;
//     int sector = phiId/delta;  // 0..15
    int nth = phiId % delta;

    switch (thetaId) {
      case 0:
        if (nth == 1)
          fp2p2 = 1000;
        break;
      case 1:
        if (nth == 1) {
          fp2p2 = 1000;
          fp1p2 = fp1p1;
          fp1p1 = 1000;
        }
        break;
      case 2:
        if ((nth == 0) || (nth == 1)) {
          fm2p2 = 1000;
          fm1p2 = fm1p1;
          fm1p1 = 1000;
        } else if ((nth == 2) || (nth == 3)) {
          fm2m2 = 1000;
          fm1m2 = fm1m1;
          fm1m1 = 1000;
        }
        break;
      case 3:
        if ((nth == 0) || (nth == 3)) {
          fm2p2 = fm2m2 = 1000;
        } else if (nth == 1) {
          fm2p2 = 1000;
        } else if (nth == 2) {
          fm2m2 = 1000;
        }
        break;
      case 5:
        if ((nth == 2) || (nth == 5)) {
          fm2m2 = 1000;
          fm1m2 = fm1m1;
          fm1m1 = 1000;
        } else {
          fm2p2 = 1000;
          fm1p2 = fm1p1;
          fm1p1 = 1000;
        }
        break;
      case 6:
        fm2p2 = 1000;
        if ((nth == 0) || (nth == 2) || (nth == 3) || (nth == 5)) {
          fm2m2 = 1000;
        }
        break;
      case 11:
        if ((nth == 2) || (nth == 5) || (nth == 8)) {
          fm2m2 = 1000;
          fm1m2 = fm1m1;
          fm1m1 = 1000;
        } else {
          fm2p2 = 1000;
          fm1p2 = fm1p1;
          fm1p1 = 1000;
        }
        break;
      case 12:
        fm2p2 = 1000;
        if ((nth == 0) || (nth == 2) || (nth == 3)
            || (nth == 5) || (nth == 6) || (nth == 8))
          fm2m2 = 1000;
        break;
      case 65:
        if ((nth == 2) || (nth == 5)) {
          fp2m2 = 1000;
          fp1m2 = fp1m1;
          fp1m1 = 1000;
        } else {
          fp2p2 = 1000;
          fp1p2 = fp1p1;
          fp1p1 = 1000;
        }
        break;
      case 64:
        fp2p2 = 1000;
        if ((nth ==  0) || (nth == 2) || (nth == 3) || (nth == 5))
          fp2m2 = 1000;
        break;
      case 60:
        if ((nth == 2) || (nth == 5) || (nth == 8)) {
          fp2m2 = 1000;
          fp1m2 = fp1m1;
          fp1m1 = 1000;
        } else {
          fp2p2 = 1000;
          fp1p2 = fp1p1;
          fp1p1 = 1000;
        }
        break;
      case 59:
        fp2p2 = 1000;
        if ((nth ==  0) || (nth == 2) || (nth == 3) || (nth == 5)
            || (nth == 6) || (nth == 8))
          fp2m2 = 1000;
        break;
    }//switch

    // insert near-nbrs
    vNbr.push_back(GetCellID(t00, f00m1));
    vNbr.push_back(GetCellID(t00, f00p1));
    if (nm1 < 999) {
      vNbr.push_back(GetCellID(tm1 , fm100));
      if (fm1m1 < 999)
        vNbr.push_back(GetCellID(tm1 , fm1m1));
      if (fm1p1 < 999)
        vNbr.push_back(GetCellID(tm1 , fm1p1));
    }
    if (np1 < 999) {
      vNbr.push_back(GetCellID(tp1 , fp100));
      if (fp1m1 < 999)
        vNbr.push_back(GetCellID(tp1 , fp1m1));
      if (fp1p1 < 999)
        vNbr.push_back(GetCellID(tp1 , fp1p1));
    }
    nearSize = vNbr.size() ;

    // now on to next-near neighbors
    if (nm2 < 999) {
      vNbr.push_back(GetCellID(tm2 , fm200));
      if (fm2m1 < 999)
        vNbr.push_back(GetCellID(tm2 , fm2m1));
      if (fm2p1 < 999)
        vNbr.push_back(GetCellID(tm2 , fm2p1));
      if (fm2m2 < 999)
        vNbr.push_back(GetCellID(tm2 , fm2m2));
      if (fm2p2 < 999)
        vNbr.push_back(GetCellID(tm2 , fm2p2));
    }
    if (nm1 < 999) {
      if (fm1m2 < 999)
        vNbr.push_back(GetCellID(tm1 , fm1m2));
      if (fm1p2 < 999)
        vNbr.push_back(GetCellID(tm1 , fm1p2));
    }
    vNbr.push_back(GetCellID(t00 , f00m2));
    vNbr.push_back(GetCellID(t00 , f00p2));
    if (np1 < 999) {
      if (fp1m2 < 999)
        vNbr.push_back(GetCellID(tp1, fp1m2));
      if (fp1p2 < 999)
        vNbr.push_back(GetCellID(tp1, fp1p2));
    }
    if (np2 < 999) {
      vNbr.push_back(GetCellID(tp2, fp200));
      if (fp2m1 < 999)
        vNbr.push_back(GetCellID(tp2, fp2m1));
      if (fp2p1 < 999)
        vNbr.push_back(GetCellID(tp2, fp2p1));
      if (fp2m2 < 999)
        vNbr.push_back(GetCellID(tp2, fp2m2));
      if (fp2p2 < 999)
        vNbr.push_back(GetCellID(tp2, fp2p2));
    }
  }//else( aCellId > 1152 && aCellId < 7777 )
  return
    EclNbr(vNbr, nearSize);
}
