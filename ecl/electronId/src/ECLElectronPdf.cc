#include <ecl/electronId/ECLElectronPdf.h>
#include <ecl/electronId/ParameterMap.h>

#include <cmath>
#include <TMath.h>

using namespace Belle2;

void  ECLElectronPdf::init(const char* parametersFileName)
{
  ParameterMap map(parametersFileName);

  for (int i = 1; i < 8; ++i) {
    Parameters& prm = m_params[i];
    prm.mu1 = map.param(name("electrons_mu1_", i));
    double m0 = prm.mu2 = map.param(name("electrons_mu2_", i));
    prm.sigma1 = map.param(name("electrons_sigma1_", i));
    double sigma = prm.sigma2 = map.param(name("electrons_sigma2_", i));
    prm.fraction = map.param(name("electrons_fraction_", i));
    double absAlpha = TMath::Abs(prm.alpha = map.param(name("electrons_alpha_", i)));
    double n = prm.nn = map.param(name("electrons_nn_", i));

    m_integral1[i] = 0.5 * (1 - TMath::Erf(- prm.mu1 / prm.sigma1 / s_sqrt2));

    double sig = TMath::Abs(sigma);
    double tmin = - m0 / sig;

    double a = TMath::Power(n / absAlpha, n) * std::exp(-0.5 * absAlpha * absAlpha);
    double b = n / absAlpha - absAlpha;
    double term1 = a * sig / (1.0 - n) * (1.0 / (TMath::Power(b - tmin, n - 1.0)) - 1.0 / (TMath::Power(n / absAlpha, n - 1.0)));
    double term2 = sig * s_sqrtPiOver2 * (1 - TMath::Erf(-absAlpha / s_sqrt2));

    m_integral2[i] = (term1 + term2);
  }
}

double ECLElectronPdf::pdf(double eop, double p) const
{
  if (p > 100.) p = 100.; // very large p can not fit into int().
  int i = int(p / 0.250);
  if (i == 0) i = 1;
  if (i > 7) i = 7;
  const Parameters& prm = m_params[i];
  double t1 = TMath::Gaus(eop, prm.mu1, prm.sigma1, true) / m_integral1[i];
  double m0 = prm.mu2;
  double sigma = prm.sigma2;
  double alpha = prm.alpha;
  double n = prm.nn;

  double t2;
  double t = (eop - m0) / sigma;
  Double_t absAlpha = TMath::Abs(alpha);

  if (t >= -absAlpha) {
    t2 = std::exp(-0.5 * t * t) ;
  } else {
    Double_t a =  TMath::Power(n / absAlpha, n) * std::exp(-0.5 * absAlpha * absAlpha);
    Double_t b = n / absAlpha - absAlpha;
    t2 = a / TMath::Power(b - t, n);
  }
  t2 /= m_integral2[i];
  return prm.fraction * t1 + (1 - prm.fraction) * t2;
}


