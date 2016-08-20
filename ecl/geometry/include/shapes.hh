#ifndef __h_shapes
#define __h_shapes
#include <string>
#include <vector>
#include <G4VSolid.hh>
#include <G4Transform3D.hh>
#include <G4LogicalVolume.hh>
#include <BelleCrystal.hh>

double cosd(double);
double sind(double);
double tand(double);

#define UNUSED __attribute__((unused))

struct TripletF {
  double m_1, m_2, m_3;
  TripletF(){}
  TripletF(const double& a1, const double& a2, const double& a3): m_1(a1), m_2(a2), m_3(a3){}
};

struct shape_t{
  int nshape;
  shape_t(){}
  virtual ~shape_t(){}
  
  G4VSolid *get_solid(const std::string &prefix, double wrapthick, G4Translate3D &shift) const;
  virtual G4VSolid *get_tesselatedsolid(const std::string &prefix, double wrapthick, G4Translate3D &shift) const = 0;
  virtual G4VSolid *get_trapezoid(const std::string &prefix, double wrapthick, G4Translate3D &shift) const = 0;
  virtual G4VSolid *get_extrudedsolid(const std::string &prefix, double wrapthick, G4Translate3D &shift) const = 0;
  virtual G4VSolid *get_bellecrystal(const std::string &prefix, double wrapthick, G4Translate3D &shift) const = 0;
  virtual bool istrap() const = 0;
};

struct cplacement_t{
  int nshape;
  double Rphi1, Rtheta, Rphi2; // Eulers' angles
  double Pr, Ptheta, Pphi; // origin position
};

std::vector<shape_t*> load_shapes(const std::string &fname);

G4VSolid* getpc_solid(const char *, int n, const TripletF *, double, double);
Point_t centerofgravity(Point_t*, Point_t*);
G4LogicalVolume *wrapped_crystal(const shape_t* s, const std::string &endcap, double wrapthickness);
G4Transform3D get_transform(const cplacement_t &t);

#endif //__h_shapes
