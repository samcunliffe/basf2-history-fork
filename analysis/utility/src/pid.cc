#include <analysis/utility/pid.h>

#include <GFTrack.h>

#include <top/dataobjects/TOPTrack.h>
#include <arich/dataobjects/ARICHAeroHit.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>

const TOPLikelihoods* getTOPLikelihoods(const Track& track)
{
  StoreArray<TOPLikelihoods> toplogL;
  StoreArray<GFTrack>        gfTracks;
  StoreArray<Track>          tracks;

  RelationIndex<GFTrack, TOPLikelihoods> gfTracksToTOPLogL(gfTracks, toplogL);

  if (!(tracks && gfTracks && toplogL && gfTracksToTOPLogL))
    return 0;

  int trackIndex = tracks.getPtr()->IndexOf(&track);

  if (trackIndex < 0)
    return 0;

  // It is assumed that Tracks and GFTracks have the same indices
  if (!gfTracks[trackIndex])
    return 0;

  if (gfTracksToTOPLogL.getFirstElementFrom(gfTracks[trackIndex]))
    return gfTracksToTOPLogL.getFirstElementFrom(gfTracks[trackIndex])->to;

  return 0;
}

const DedxLikelihood* getDEDXLikelihood(const Track& track)
{
  StoreArray<DedxLikelihood> dedxlogL;
  StoreArray<GFTrack>        gfTracks;
  StoreArray<Track>          tracks;

  RelationIndex<GFTrack, DedxLikelihood> gfTracksToDEDXLogL(gfTracks, dedxlogL);

  if (!(tracks && gfTracks && dedxlogL && gfTracksToDEDXLogL))
    return 0;

  int trackIndex = tracks.getPtr()->IndexOf(&track);

  if (trackIndex < 0)
    return 0;

  // It is assumed that Tracks and GFTracks have the same indices
  if (!gfTracks[trackIndex])
    return 0;

  if (gfTracksToDEDXLogL.getFirstElementFrom(gfTracks[trackIndex]))
    return gfTracksToDEDXLogL.getFirstElementFrom(gfTracks[trackIndex])->to;

  return 0;
}

const TOPLikelihoods* getTOPLikelihoods(const MCParticle* particle)
{

  StoreArray<MCParticle> mcParticles;
  StoreArray<TOPTrack>  topTracks;
  StoreArray<TOPLikelihoods> toplogL;

  RelationIndex< TOPTrack, TOPLikelihoods > relTrackLikelihoods(topTracks, toplogL);
  RelationIndex<MCParticle, TOPTrack> relMCParticleToTOPTrack(mcParticles, topTracks);

  if (!(relTrackLikelihoods && relMCParticleToTOPTrack)) {
    return 0;
  }

  if (relMCParticleToTOPTrack.getFirstElementFrom(particle)) {
    const TOPTrack* track = relMCParticleToTOPTrack.getFirstElementFrom(particle)->to;

    if (relTrackLikelihoods.getFirstElementFrom(track)) {

      return relTrackLikelihoods.getFirstElementFrom(track)->to;

    }
  }

  return 0;
}


const ARICHLikelihoods* getARICHLikelihoods(const MCParticle* particle)
{
  StoreArray<ARICHLikelihoods> arichLikelihoods;
  StoreArray<ARICHAeroHit> arichAeroHits;
  StoreArray<MCParticle> mcParticles;

  RelationIndex<MCParticle, ARICHAeroHit> arichAeroHitRel(mcParticles, arichAeroHits);
  RelationIndex<ARICHAeroHit, ARICHLikelihoods> relAeroToLikelihood(arichAeroHits, arichLikelihoods);

  if (!(arichAeroHitRel && relAeroToLikelihood))
    return 0;

  if (arichAeroHitRel.getFirstElementFrom(particle)) {
    const ARICHAeroHit* track = arichAeroHitRel.getFirstElementFrom(particle)->to;
    if (relAeroToLikelihood.getFirstElementFrom(track))
      return relAeroToLikelihood.getFirstElementFrom(track)->to;
  }

  return 0;
}

double getTOPPID(int hyp1, int hyp2, const TOPLikelihoods* logL)
{
  double logl1 = 0.;
  double logl2 = 0.;

  if (hyp1 == hyp2 || hyp1 > 5 || hyp1 < 1 || hyp2 > 5 || hyp2 < 1) {
    return 0;
  }

  if (logL) {
    if (logL->getFlag()) {

      if (hyp1 == 1)
        logl1 = logL->getLogL_e();
      if (hyp1 == 2)
        logl1 = logL->getLogL_mu();
      if (hyp1 == 3)
        logl1 = logL->getLogL_pi();
      if (hyp1 == 4)
        logl1 = logL->getLogL_K();
      if (hyp1 == 5)
        logl1 = logL->getLogL_p();

      if (hyp2 == 1)
        logl2 = logL->getLogL_e();
      if (hyp2 == 2)
        logl2 = logL->getLogL_mu();
      if (hyp2 == 3)
        logl2 = logL->getLogL_pi();
      if (hyp2 == 4)
        logl2 = logL->getLogL_K();
      if (hyp2 == 5)
        logl2 = logL->getLogL_p();

      return logl1 - logl2;
    }
  }
  return 0;
}

double getARICHPID(int hyp1, int hyp2, const ARICHLikelihoods* logL)
{
  double logl1 = 0.;
  double logl2 = 0.;

  if (hyp1 == hyp2 || hyp1 > 5 || hyp1 < 1 || hyp2 > 5 || hyp2 < 1) {
    return 0;
  }

  if (logL) {
    if (logL->getFlag()) {

      if (hyp1 == 1)
        logl1 = logL->getLogL_e();
      if (hyp1 == 2)
        logl1 = logL->getLogL_mu();
      if (hyp1 == 3)
        logl1 = logL->getLogL_pi();
      if (hyp1 == 4)
        logl1 = logL->getLogL_K();
      if (hyp1 == 5)
        logl1 = logL->getLogL_p();


      if (hyp2 == 1)
        logl2 = logL->getLogL_e();
      if (hyp2 == 2)
        logl2 = logL->getLogL_mu();
      if (hyp2 == 3)
        logl2 = logL->getLogL_pi();
      if (hyp2 == 4)
        logl2 = logL->getLogL_K();
      if (hyp2 == 5)
        logl2 = logL->getLogL_p();

      return logl1 - logl2;
    }
  }
  return 0;
}

double getDEDXPID(DedxParticle hyp1, DedxParticle hyp2, const DedxLikelihood* logL)
{
  if (logL)
    return logL->getLogLikelihood(hyp1) - logL->getLogLikelihood(hyp2);

  return 0;
}
