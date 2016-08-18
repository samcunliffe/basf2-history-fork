import math
import numpy as np


def iqr(array_like):
    """Computes the interquantile range, hence the distance from the 25% to 75% quantile."""

    return np.percentile(array_like, 75) - np.percentile(array_like, 25)


def trimmed_std(array_like):
    """A trimmed estimate for the standard deviation of a normal distribution
    contaminated by outliers using the interquanitle range times the appropriate factor.
    """

    normal_iqr_to_std_factor = math.sqrt(2.0) * math.erf(0.5)
    return normal_iqr_to_std_factor * iqr(array_like)


def truncated_mean(array_like, r=0.23):
    """Calculates the truncated mean, where 2r is the fraction of data to be taken into account.

    The truncated mean is a robust estimator for the central value of a symmetric distribution.
    The (1-r) upper and the (1-r) lower values are left out and only the central remaining 2r fraction enters
    the normal calculation of the mean. The default value of r is the text book value, which produces an approximatelly
    82%-efficient estimate of the mean for normal, central value of the cauchy and the double-exponential
    distribution.
    """

    truncation = 1 - 2 * r

    if truncation >= 1 or truncation <= 0:
        raise ValueError('Value of r must be between 0 and 0.5')

    array = np.array(array_like)

    lower_percentile = 100 * truncation / 2.0
    upper_percentile = 100 * (1.0 - truncation / 2.0)

    (lower_bound, upper_bound) = np.percentile(array, (lower_percentile,
                                                       upper_percentile))

    weights = (array >= lower_bound) & (array <= upper_bound)

    truncated_array = array[weights]

    return np.mean(truncated_array)


def cubic_root(x):
    """Returns the cubic root of a number"""
    return pow(float(x), 1.0 / 3.0)


def rice_n_bin(n_data):
    """Returns the number of bins for a number of data instances according to the rice rule."""
    return np.ceil(2.0 * cubic_root(n_data))


def rice_exceptional_values(xs):
    """Returns a array of exceptionally frequent values according to the rice rule."""
    unique_xs, indices, unique_xs_count = np.unique(xs, return_inverse=True, return_counts=True)
    # Note: The indices are such that unique_xs[indices] == xs

    exceptional_xs = []

    while True:
        n_data = np.sum(unique_xs_count)
        n_exceptional = 1.0 * n_data / rice_n_bin(n_data)
        exceptional_indices = unique_xs_count > n_exceptional

        if np.any(exceptional_indices):
            exceptional_xs.extend(unique_xs[exceptional_indices])
            unique_xs_count[exceptional_indices] = 0
        else:
            break

    return np.array(sorted(exceptional_xs))


def is_binary_series(xs):
    """Deterimes of the given series only consists of true and false values"""
    is_boolean = all(isinstance(x, bool) for x in xs)
    is_one_or_zero = all(x == 0 or x == 1 or not np.isfinite(x)
                         for x in xs)

    return is_boolean or is_one_or_zero

#: Constant defining how many unique values are allowed for a discrete quantity.
default_max_n_unique_for_discrete = 20


def is_discrete_series(xs, max_n_unique=None):
    """Determine of a data series is only made of a bunch of discrete values

    Currently only test if the number of unique values is lower than
    the default_max_n_unique_for_discrete.

    Parameters
    ----------
    xs : np.array (1d)
        Data series
    max_n_unique : int
        Number of allowed distinct values for the series to be discrete.
    """

    if max_n_unique is None:
        max_n_unique = default_max_n_unique_for_discrete

    unique_xs = np.unique(xs)
    if len(unique_xs) < max_n_unique:
        return True
    elif len(unique_xs) > 150:
        return False
    else:
        exceptional_xs = rice_exceptional_values(xs)
        rest_xs = np.setdiff1d(unique_xs, exceptional_xs, assume_unique=True)
        if len(rest_xs) < max_n_unique:
            return True

    return False
