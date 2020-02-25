# !/usr/bin/env python3
# -*- coding: utf-8 -*-

import ROOT as R


def create1DHist(name, title, n_bins, x_min, x_max, x_label, y_label):
    h = R.TH1F(name, title, n_bins, x_min, x_max)
    h.GetXaxis().SetTitle(x_label)
    h.GetYaxis().SetTitle(y_label)
    return h


def addDetails(h, descr, check, contact_str, isShifter):
    h.GetListOfFunctions().Add(R.TNamed("Description", descr))
    h.GetListOfFunctions().Add(R.TNamed("Check", check))
    h.GetListOfFunctions().Add(R.TNamed("Contact", contact_str))
    if isShifter:
        h.GetListOfFunctions().Add(R.TNamed("MetaOptions", "shifter"))

# constants
SVDContact = "SVD Software Group, svd-software@belle2.org"


# selection of different parts of detector
cut_L3 = R.TCut('layer==3')
cut_L4 = R.TCut('layer==4')
cut_L5 = R.TCut('layer==5')
cut_L6 = R.TCut('layer==6')
cut_L456 = R.TCut('(layer==4)||(layer==5)||(layer==6)')
cut_s = R.TCut('sensor_type==0')  # slanted
cut_b = R.TCut('sensor_type==1')  # barrel
cut_U = R.TCut('strip_dir==0')  # U_P
cut_V = R.TCut('strip_dir==1')  # V_N

# default granurality
granulesD = ((cut_L3+cut_b, 'L3_barrel'),
             (cut_L456+cut_b, 'L456_barrel'),
             (cut_L456+cut_s, 'L456_slanted'))


# granurality taking into account layers and type of sensor;
granulesLayersTypes = ((cut_L3+cut_b, 'L3_barrel'),
                       (cut_L4+cut_b, 'L4_barrel'),
                       (cut_L4+cut_s, 'L4_slanted'),
                       (cut_L5+cut_b, 'L5_barrel'),
                       (cut_L5+cut_s, 'L5_slanted'),
                       (cut_L6+cut_b, 'L6_barrel'),
                       (cut_L6+cut_s, 'L6_slanted'))


def ploter(name, title, nbins, xmin, xmax, x_label, y_label,
           granules,
           tree, expr, cut,
           descr, check, contact_str=SVDContact, isShifter=False):
    for g in granules:
        hName = f'{name}_{g[1]}'
        h = create1DHist(hName, title, nbins, xmin, xmax, x_label, y_label)
        tree.Draw(f'{expr}>>{hName}', g[0]+R.TCut(cut), 'goff')
        addDetails(h, descr, check, contact_str, isShifter)
        h.SetTitle(f'{title} ({g[1]})')
        h.Write(hName)
