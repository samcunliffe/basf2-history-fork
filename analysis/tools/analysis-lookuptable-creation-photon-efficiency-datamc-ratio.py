import glob
import numpy as np
import basf2 as b2

# Define bin constructors


def make_1D_bin(name, min_val, max_val):
    return {name: [min_val, max_val]}


def make_2D_bin(bin_x, bin_y):
    bin_2d = bin_x.copy()
    bin_2d.update(bin_y)
    return bin_2d


def make_3D_bin(bin_x, bin_y, bin_z):
    bin_3d = bin_x.copy()
    bin_3d.update(bin_y)
    bin_3d.update(bin_z)
    print(bin_3d)
    return bin_3d

# To make these one needs tables generated with the photon efficiency code (repository: [LINK])
# They provide you with 2D tables, for different photon energy bins.
# The energy bins are defined in the file names, but the theta, phi bins are not, so they are given in separate files.
# The naming scheme has to be exact for this to work, but luckily the code in the repository takes care of all that.
# So you only need to input the location where you want the code to look for these tables.


table_location = 'plots_lima/lima/'

# Define bin ranges. Bins may be of different size
yedges = np.load(f"{table_location}/table_pRecoilPhipRecoilTheta_pRecoilfrom0p2andpRecoilto0p4_pRecoilPhibins.npy")
xedges = np.load(f"{table_location}/table_pRecoilPhipRecoilTheta_pRecoilfrom0p2andpRecoilto0p4_pRecoilThetabins.npy")

# Define values to put into the table, that correspond to previously defined bins.
values = glob.glob(f"{table_location}/table_pRecoilPhipRecoilTheta*values.npy")
stat_up = glob.glob(f"{table_location}/table_pRecoilPhipRecoilTheta*statistical_up.npy")
stat_down = glob.glob(f"{table_location}/table_pRecoilPhipRecoilTheta*statistical_down.npy")
sys_up = glob.glob(f"{table_location}/table_pRecoilPhipRecoilTheta*systematic_up.npy")
sys_down = glob.glob(f"{table_location}/table_pRecoilPhipRecoilTheta*systematic_down.npy")

bins_p = []
bins_phi = [make_1D_bin("phi", lowbin, highbin) for lowbin, highbin in zip(yedges[:-1], yedges[1:])]
bins_theta = [make_1D_bin("theta", lowbin, highbin) for lowbin, highbin in zip(xedges[:-1], xedges[1:])]

table = []

for n, valuenp in enumerate(values):

    bins_with_text = valuenp.split('pRecoilfrom')[1].split('pRecoilto')
    lowbin_str = bins_with_text[0][:-3].replace('p', '.')
    highbin_str = bins_with_text[1][:-11].replace('p', '.')
    pbin = make_1D_bin("E", float(lowbin_str), float(highbin_str))

    value = np.load(valuenp)
    stat_err_up_table = np.load(stat_up[n])
    stat_err_down_table = np.load(stat_down[n])
    sys_err_up_table = np.load(sys_up[n])
    sys_err_down_table = np.load(sys_down[n])
    for j, ybin in enumerate(bins_phi):
        for i, xbin in enumerate(bins_theta):
            weightInfo = {}
            weightInfo["Weight"] = value[i, j]
            weightInfo["StatErrUp"] = stat_err_up_table[i, j]
            weightInfo["StatErrDown"] = stat_err_down_table[i, j]
            weightInfo["SystErrUp"] = sys_err_up_table[i, j]
            weightInfo["SystErrDown"] = sys_err_down_table[i, j]
            table.append([weightInfo, make_3D_bin(xbin, ybin, pbin)])

# And of course let's define out-of-range bin info
outOfRangeWeightInfo = {}
outOfRangeWeightInfo["Weight"] = np.nan
outOfRangeWeightInfo["StatErrUp"] = np.nan
outOfRangeWeightInfo["SystErrUp"] = np.nan
outOfRangeWeightInfo["StatErrDown"] = np.nan
outOfRangeWeightInfo["SystErrDown"] = np.nan

# Now, let's configure table creator
addtable = b2.register_module('ParticleWeightingLookUpCreator')
addtable.param('tableIDNotSpec', table)
addtable.param('outOfRangeWeight', outOfRangeWeightInfo)
addtable.param('experimentHigh', -1)
addtable.param('experimentLow', 0)
addtable.param('runHigh', -1)
addtable.param('runLow', 0)
addtable.param('tableName', "ParticleReweighting:PhotonEfficiencyDataMCRatio")

eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param('evtNumList', [10])
eventinfosetter.param('runList', [0])
eventinfosetter.param('expList', [0])

my_path = b2.create_path()
my_path.add_module(addtable)
my_path.add_module(eventinfosetter)

b2.process(my_path)
