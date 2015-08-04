#!/usr/bin/env python
# -*- coding: utf-8 -*-


def string(some_string):
    """
    Used to escape user strings for LaTex.
        @param some_string which is escaped
        @return string escaped version of some_string
    """
    return some_string.replace('\\', r'\textbackslash').replace('_', r'\_').replace('^', r'\^{}')


def variable(variable_string):
    """
    Adds hyphenations after brackets, and for common variables.
        @param variable_string variable name
        @param string with hyphenation hints for latex
    """
    substitutes = {
        '=': r'=\allowbreak ',
        ':': r':\allowbreak ',
        '(': r'(\allowbreak ',
        'extraInfo': r'ex\-tra\-In\-fo',
        'SignalProbability': r'Sig\-nal\-Prob\-a\-bil\-i\-ty',
        'cosAngleBetweenMomentumAndVertexVector': r'cosAngle\-Between\-Momentum\-And\-Vertex\-Vector'}
    for key, value in substitutes.iteritems():
        variable_string = variable_string.replace(key, value)
    return variable_string


def decayDescriptor(decay_string):
    """
    Prettifies the given decay string by using latex-symbols instead of plain-text
        @param decay_string string containing an decay descriptor
        @return string latex version of the decay descriptor
    """
    decay_string = decay_string.replace(':generic', '')
    substitutes = {
        '==>': '$\\to$',
        'gamma': r'$\gamma$',
        'pi+': r'$\pi^+$',
        'pi-': r'$\pi^-$',
        'pi0': r'$\pi^0$',
        'K_S0': r'$K^0_S$',
        'mu+': r'$\mu^+$',
        'mu-': r'$\mu^-$',
        'tau+': r'$\tau^+$',
        'tau-': r'$\tau^-$',
        'nu': r'$\nu$',
        'K+': r'$K^+$',
        'K-': r'$K^-$',
        'e+': r'$e^+$',
        'e-': r'$e^-$',
        'J/psi': r'$J/\psi$',
        'D+': r'$D^+$',
        'D-': r'$D^-$',
        'D0': r'$D^0$',
        'D*+': r'$D^{+*}$',
        'D*-': r'$D^{-*}$',
        'D*0': r'$D^{0*}$',
        'D_s+': r'$D^+_s$',
        'D_s-': r'$D^-_s$',
        'D_s*+': r'$D^{+*}_s$',
        'D_s*-': r'$D^{-*}_s$',
        'anti-D0': r'$\bar{D^0}$',
        'anti-D*0': r'$\bar{D^{0*}}$',
        'B+': r'$B^+$',
        'B-': r'$B^-$',
        'B0': r'$B^0$',
        'anti-B0': r'$\bar{B^0}$'}
    tex_string = decay_string
    for key, value in substitutes.iteritems():
        tex_string = tex_string.replace(key, value)
    return '\\texorpdfstring{%s}{%s}' % (tex_string, string(decay_string))


def duration(seconds):
    """
    Converts a duration given in seconds into a nice latex-style duration string
        @param seconds duration in seconds
        @return string describing a duration in a natural format
    """
    minutes = int(seconds / 60)
    hours = int(minutes / 60)
    minutes %= 60
    ms = int(seconds * 1000) % 1000
    us = int(seconds * 1000 * 1000) % 1000
    seconds = int(seconds % 60)
    string = ''
    if hours != 0:
        string += "%dh" % (hours)
    if minutes != 0:
        string += "%dm" % (minutes)
    if seconds != 0 and hours == 0:
        string += "%ds" % (seconds)
    if ms != 0 and hours == 0 and minutes == 0 and seconds == 0:
        string += "%dms" % (ms)
    if us != 0 and hours == 0 and minutes == 0 and seconds == 0 and ms == 0:
        string += "%d$\mu$s" % (us)

    if hours == 0 and minutes == 0 and seconds == 0 and ms == 0 and us == 0:
        string += '$<1\mu$s'
    return string
