#include <ecl/digitization/EclConfigurationPure.h>
#include <ecl/digitization/ECLSampledShaper.h>
#include <TH1F.h>
#include <algorithm>
#include <iostream>

using namespace Belle2;
using namespace Belle2::ECL;
using namespace std;

double EclConfigurationPure::m_tickPure = EclConfiguration::m_tick / EclConfiguration::m_ntrg * 8;

void EclConfigurationPure::signalsamplepure_t::InitSample(const TH1F* sampledfun, const TH1F* sampledfunDerivative)
{
  const int N = m_ns * m_nlPure;
  double r1 = 32 / m_ns;
  double r2 = EclConfiguration::m_tick / EclConfiguration::m_ntrg * 8  / EclConfigurationPure::m_tickPure ;

  ECLSampledShaper dsp(sampledfun, round(r1 / r2));
  dsp.fillarray(N, m_ft);
  double maxval = * max_element(m_ft, m_ft + N);

  for_each(m_ft1, m_ft1 + N, [maxval](double & a) { a /= maxval; });
  double sum = 0;
  for (int i = 0; i < N; i++) sum += m_ft[i];
  m_sumscale = m_ns / sum;
  //  for (int i = 0; i < N; ++i) m_ft1[i] = sampledfunDerivative->GetBinContent(i + 1);
  ECLSampledShaper dsp1(sampledfunDerivative, round(r1 / r2));
  dsp1.fillarray(N, m_ft1);
  for_each(m_ft1, m_ft1 + N, [r1, r2](double & a) { a *= r1 / r2; });
}

void EclConfigurationPure::adccountspure_t::AddHit(const double a, const double t0,
                                                   const EclConfigurationPure::signalsamplepure_t& s)
{
  total += s.Accumulate(a, t0, c);
}

double EclConfigurationPure::signalsamplepure_t::Accumulate(const double a, const double t0, double* s) const
{
  // input parameters
  // a -- signal amplitude
  // t -- signal offset
  // output parameter
  // s -- output array with added signal
  const double itick = 1 / m_tickPure;          // reciprocal to avoid division in usec^-1 (has to be evaluated at compile time)
  const double  tlen = m_nlPure - 1.0 / m_ns;   // length of the sampled signal in ADC clocks units
  const double  tmax = m_tmin + m_nsmp - 1; // upper range of the fit region

  double t = t0 * itick; // rescale time in usec to ADC clocks
  double x0 = t, x1 = t + tlen;

  if (x0 > tmax) return 0; // signal starts after the upper range of output array -- do nothing
  if (x0 < m_tmin) {
    if (x1 < m_tmin) return 0; // signal ends before lower range of output array -- do nothing
    x0 = m_tmin; // adjust signal with range of output array
  }

  int imax = m_nsmp; // length of sampled signal is long enough so
  // the last touched element is the last element
  // of the output array
  if (x1 < tmax) { // if initial time is too early we need to adjust
    // the last touched element of output array to avoid
    // out-of-bound situation in m_ft
    imax = x1 - m_tmin; // imax is always positive so floor can be
    // replace by simple typecast
    imax += 1; // like s.end()
  }

  double imind = ceil(x0 - m_tmin); // store result in double to avoid int->double conversion below
  // the ceil function today at modern CPUs is surprisingly fast (before it was horribly slow)
  int imin = imind; // starting point to fill output array
  double w = ((m_tmin - t) + imind) * double(m_ns);
  int jmin = w; // starting point in sampled signal array
  w -= jmin;

  // use linear interpolation between samples. Since signal samples
  // are aligned with output samples only two weights are need to
  // calculate to fill output array
  const double w1 = a * w, w0 = a - w1;
  double sum = 0;
  for (int i = imin, j = jmin; i < imax; i++, j += m_ns) {
    double amp = w0 * m_ft[j] + w1 * m_ft[j + 1];
    s[i] += amp;
    sum  += amp;
  }
  return sum * m_sumscale;
}
