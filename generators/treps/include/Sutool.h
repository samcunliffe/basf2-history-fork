// -*- C++ -*-
//
// Package:     <package>
// Module:      Sutool
//
// Description: <one line class summary>
//
// Usage:
//    <usage>
//
// Author:      Sadaharu Uehara
// Created:     Thu May  8 15:09:38 JST 1997
// $Id$
//
// Revision history
// Modified to be suitable to Belle II style
//
// $Log$

#include<TRandom.h>
#include<TLorentzVector.h>
#include<TVector3.h>
#include <fstream>
using namespace std;

#if !defined(PACKAGE_SUTOOL_H_INCLUDED)
#define PACKAGE_SUTOOL_H_INCLUDED

namespace Belle2 {

  class Sutool {
    // friend classes and functions

  public:
    // constants, enums and typedefs

    //    double rndd(void) {
    //    extern BasfranEngine* BASF_Random_Engine;
    //    double fnum = RandFlat::shoot( BASF_Random_Engine );
    //    return fnum;
    //  }

    // Constructors and destructor
    Sutool() {};
    virtual ~Sutool() {};

    // member functions
    void nextline(ifstream& infile)
    // skip to end of line
    {
      char s ;
      do { infile.get(s) ; }  while (s != '\n');
    };

    double p2bdy(double, double, double, int&);
    int pdecy(double, double*, TVector3&, TLorentzVector*, int);
    void rotate(TLorentzVector&, double,  double);
    float interp(float, float, float, float, float);
    int poisson(double);
  };


  class Interps {
  public:
    Interps(void) {};
    //  Interps( double, double, int, double* );
    Interps(double x1, double x2, int nd, double* yd)
    {
      xmin = x1; xmax = x2;
      ndiv = nd; ydata = yd;
      return;
    };

    ~Interps(void) {};

    //  double get_val( double );
    double get_val(double x)
    {
      double rn = (x - xmin) / (xmax - xmin) * (double)ndiv;
      int irn = (int)rn;
      if (irn <= 0) irn = 1;
      else if (irn >= (ndiv - 1)) irn = ndiv - 2;

      double x1 = interp(&ydata[irn - 1], rn - (double)irn);
      return x1 ;
    };

    //  void change( double, double, int, double* ) ;
    void change(double x1, double x2, int nd, double* yd)
    {
      xmin = x1; xmax = x2;
      ndiv = nd; ydata = yd;
      return;
    };

    //int check_xrange( double );
    int check_xrange(double x)
    {
      if (x < xmin) return -1 ;
      else if (x > xmax) return 1;
      else return 0;
    };


  private:
    //  double interp( double*, double );
    double interp(double* y, double x)
    {

      double a = y[1] - y[0];
      double b = y[2] - y[1];
      double c = y[3] - y[2];
      double d = b - a;
      double e = c - b;
      double f = e - d;

      double b2 = 0.25 * x * (x - 1.);
      double b3 = 2. / 3.*b2 * (x - 0.5);

      return y[1] + x * b + b2 * (d + e) + b3 * f;
    };

  public:
    double xmin, xmax;
    int ndiv;
    double* ydata;





  };

  class dbcomplex {
    double re, im;

  public:
    dbcomplex(double r = 0, double i = 0) { re = r; im = i; };
    ~dbcomplex() {};

    friend dbcomplex operator +(dbcomplex a , dbcomplex b)
    {
      return dbcomplex(a.re + b.re, a.im + b.im);
    };

    friend dbcomplex operator +(double a , dbcomplex b)
    {
      return dbcomplex(a + b.re, b.im);
    };

    friend dbcomplex operator +(dbcomplex a , double b)
    {
      return dbcomplex(a.re + b, a.im);
    };

    friend dbcomplex operator -(dbcomplex a , dbcomplex b)
    {
      return dbcomplex(a.re - b.re, a.im - b.im);
    };

    friend dbcomplex operator -(double a , dbcomplex b)
    {
      return dbcomplex(a - b.re, b.im);
    };

    friend dbcomplex operator -(dbcomplex a , double b)
    {
      return dbcomplex(a.re - b, a.im);
    };

    friend dbcomplex operator *(dbcomplex a , dbcomplex b)
    {
      return dbcomplex(a.re * b.re -  a.im * b.im, a.re * b.im + a.im * b.re);
    };

    friend dbcomplex operator *(double a , dbcomplex b)
    {
      return dbcomplex(a * b.re, a * b.im);
    };

    friend dbcomplex operator *(dbcomplex a , double b)
    {
      return dbcomplex(a.re * b, a.im * b);
    };


    friend dbcomplex operator /(dbcomplex a , double b)
    {
      return dbcomplex(a.re / b, a.im / b);
    };

    friend dbcomplex operator /(double a , dbcomplex b)
    {
      return dbcomplex(a * b.re / (b.re * b.re + b.im * b.im),
                       -a * b.im / (b.re * b.re + b.im * b.im));
    };



    friend dbcomplex operator /(dbcomplex a , dbcomplex b)
    {
      return a * (1. / b);

      /*  double r = b.re;
      double i = b.im;
      double ti, tr;

      tr = r < 0 ? -r:r;
      ti = i < 0 ? -i:i;

      if( tr<= ti ){
      ti = r/i;
      tr = i*(1 + ti*ti );
      r = a.re;
      i = a.im;
      }
      else {
      ti = -i/r;
      tr = r*( 1 + ti*ti );
      r = -a.im ;
      i = a.re ;
      }
      return dbcomplex( (r*ti*i)/tr, (i*ti-r)/tr ); */
    };

    double real(void)
    {
      return re ;
    };

    double imag(void)
    {
      return im ;
    };

    double abs(void)
    {
      return sqrt(re * re + im * im) ;
    }

    dbcomplex conjg(void)
    {
      return dbcomplex(re, -im) ;
    }
  };

}
#endif /* PACKAGE_SUTOOL_H_INCLUDED */

