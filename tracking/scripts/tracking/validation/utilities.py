import basf2  # Import basf2 to make the Belle2 namespace available
import ROOT
from ROOT import Belle2

import numpy as np
import math
import contextlib

# Vectorised version of the error function for numpy arrays
try:
    from scipy.special import erf
except ImportError:
    # Minimal workaround that only relies on numpy and python 2.7
    # erf as a vectorized function
    # Will convert the incoming nparray to dtype 'object', if nan
    # values are contained
    # use numpy.asfarray(...) to convert back
    erf_ufunc = np.frompyfunc(math.erf, 1, 1)

    def erf(*args):
        result = erf_ufunc(*args)
        return np.asfarray(result)


# Vectorised version of the Prob function as known from TMath for numpy arrays
try:
    from scipy.stats import chi2
    # Prob(chi2, ndf) is defined as 1 - cdf (cumulative density function of the chi2 distribution with ndf)
    prob = chi2.sf
    del chi2

except ImportError:
    # Minimal workaround that only relies on numpy and python 2.7
    # prob as a vectorized function
    prob = np.frompyfunc(ROOT.TMath.Prob, 2, 1)


def is_primary(mc_particle):
    """Indicates if the given MCParticle is primary.

    Parameters
    ----------
    mc_particle : Belle2.MCParticle
        MCParticle to be checked"""

    return mc_particle.hasStatus(Belle2.MCParticle.c_PrimaryParticle)


def is_stable_in_generator(mc_particle):
    """Indicates if the given MCParticle is stable in the generator

    Parameters
    ----------
    mc_particle : Belle2.MCParticle
        MCParticle to be checked"""

    return mc_particle.hasStatus(Belle2.MCParticle.c_StableInGenerator)


def get_det_hit_ids(track_cand, det_ids=[Belle2.Const.PXD, Belle2.Const.SVD, Belle2.Const.CDC]):
    """Retrieves the hit ids contained in the track candidate for the given detector ids

    Parameters
    ----------
    track_cand : genfit::TrackCand
        Track candidate
    det_ids : list(int)
        List of the detector ids for which the hit ids should be retrieved

    Returns
    -------
    set( (int, int) )
        A set of pairs like (det_id, hit_id) representing the hit content of the track
    """
    det_hit_ids = set()
    for det_id in det_ids:
        if det_id == Belle2.Const.CDC:
            hits = track_cand.getCDCHitList()
        elif det_id == Belle2.Const.SVD:
            hits = track_cand.getSVDHitList()
        elif det_id == Belle2.Const.PXD:
            hits = track_cand.getPXDHitList()
        else:
            raise ValueError("DET ID not known.")

        # Working around a bug in ROOT where you should not access empty std::vectors
        if len(hits) != 0:
            det_hit_ids |= set((det_id, hit.getArrayIndex()) for hit in hits)

    return det_hit_ids


def calc_ndf_from_det_hit_ids(det_hit_ids,
                              ndf_by_det_id={Belle2.Const.PXD: 2,
                                             Belle2.Const.SVD: 2,
                                             Belle2.Const.CDC: 1}):
    """For a set of detector and hit ids calculate the total number of degrees of freedom

    Parameters
    ----------
    det_hit_ids : set( (int, int) )
        A set of pairs like (det_id, hit_id) representing the hit content of a track
    ndf_by_det_ids : dict(int=int)
        A map from detector ids to the number of degrees of freedom one hit in this detector represents.

    Returns
    -------
    int
        Total number of degrees of freedom represented by the hit set
    """
    return sum(ndf_by_det_id[det_id] for det_id, hit_id in det_hit_ids)


def calc_hit_efficiency(det_hit_ids,
                        mc_det_hit_ids,
                        ndf_by_det_id={Belle2.Const.PXD: 2,
                                       Belle2.Const.SVD: 2,
                                       Belle2.Const.CDC: 1}):
    """Calculates the fraction of detector and hits ids in a reference (MC) set that are also
    present in a reconstructed (PR) set.

    The ratio is given in terms of degrees of freedom the hits represent

    Parameters
    ----------
    det_hit_ids : set( (int, int) )
        A set of pairs like (det_id, hit_id) representing the hit content of a reconstructed (PR) track
    mc_det_hit_ids : set( (int, int) )
        A set of pairs like (det_id, hit_id) representing the hit content of a reference (MC) track
    ndf_by_det_ids : dict(int=int)
        A map from detector ids to the number of degrees of freedom one hit in this detector represents.

    Returns
    -------
    float
        Fraction of hits in the reference (MC) track that are also present in the reconstructed (PR) track
        in terms of number of degrees of freedom.
    """
    common_det_hit_ids = det_hit_ids & mc_det_hit_ids
    return np.divide(1.0 * calc_ndf_from_det_hit_ids(common_det_hit_ids),
                     calc_ndf_from_det_hit_ids(mc_det_hit_ids))


def getHelixFromMCParticle(mcParticle):
    position = mcParticle.getVertex()
    momentum = mcParticle.getMomentum()
    charge_sign = (-1 if mcParticle.getCharge() < 0 else 1)
    b_field = 1.5

    seed_helix = Belle2.Helix(position, momentum, charge_sign, b_field)
    return seed_helix


def getSeedTrackFitResult(trackCand):
    position = trackCand.getPosSeed()
    momentum = trackCand.getMomSeed()
    cartesian_covariance = trackCand.getCovSeed()
    charge_sign = (-1 if trackCand.getChargeSeed() < 0 else 1)
    particle_type = Belle2.Const.ParticleType(trackCand.getPdgCode())
    p_value = float('nan')
    b_field = 1.5
    cdc_hit_pattern = 0
    svd_hit_pattern = 0

    track_fit_result = Belle2.TrackFitResult(
        position,
        momentum,
        cartesian_covariance,
        charge_sign,
        particle_type,
        p_value,
        b_field,
        cdc_hit_pattern,
        svd_hit_pattern,
    )

    return track_fit_result
