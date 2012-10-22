#ifndef PID_H
#define PID_H

// Tracking Data Objects
#include <tracking/dataobjects/Track.h>

// generator related
#include <generators/dataobjects/MCParticle.h>

// PID Likelihoods
#include <top/dataobjects/TOPLikelihood.h>
#include <arich/dataobjects/ARICHLikelihoods.h>
#include <analysis/dataobjects/DedxLikelihood.h>

using namespace Belle2;

// returns the pointer to TOPLikelihood for given track
const TOPLikelihood* getTOPLikelihood(const Track& track);

// returns the pointer to DedxLikelihood for given track
const DedxLikelihood* getDEDXLikelihood(const Track& track);

// returns the pointer to TOPLikelihood for given MCParticle
const TOPLikelihood* getTOPLikelihood(const MCParticle* particle);

// returns the pointer to ARICHLikelihood for given MCParticle
const ARICHLikelihoods* getARICHLikelihoods(const MCParticle* particle);


// returns the TOP flag
int getTOPFlag(const Track& track)
{
  if (getTOPLikelihood(track))
    return getTOPLikelihood(track)->getFlag();

  return -2;
}

// returns the ARICH flag
int getARICHFlag(const MCParticle* particle)
{
  if (getARICHLikelihoods(particle))
    return getARICHLikelihoods(particle)->getFlag();

  return -2;
}


// returns the difference between Log likelihoods of 2 hypotheses based on TOP response
double getTOPPID(int hyp1, int hyp2, const TOPLikelihood* logL);

// returns the difference between Log likelihoods of 2 hypotheses based on dE/dx measurement
double getDEDXPID(DedxParticle hyp1, DedxParticle hyp2, const DedxLikelihood* logL);

// returns the difference between Log likelihoods of 2 hypotheses based on ARICH response
double getARICHPID(int hyp1, int hyp2, const ARICHLikelihoods* logL);



// returns the difference between Log likelihoods of 2 hypotheses based on TOP response
double getTOPPID(int hyp1, int hyp2, const Track& track)
{
  return getTOPPID(hyp1, hyp2, getTOPLikelihood(track));
};

// returns the difference between Log likelihoods of 2 hypotheses based on dE/dx measurement
double getDEDXPID(DedxParticle hyp1, DedxParticle hyp2, const Track& track)
{
  return getDEDXPID(hyp1, hyp2, getDEDXLikelihood(track));
};


// returns the difference between Log likelihoods of 2 hypotheses for given MCParticle based on TOP response
double getTOPPID(int hyp1, int hyp2, const MCParticle* particle)
{
  return getTOPPID(hyp1, hyp2, getTOPLikelihood(particle));
};

// returns the difference between Log likelihoods of 2 hypotheses for given MCParticle based on ARICH response
double getARICHPID(int hyp1, int hyp2, const MCParticle* particle)
{
  return getARICHPID(hyp1, hyp2, getARICHLikelihoods(particle));
};


#endif // PID_H                                                                                                                                         

