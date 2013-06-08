#! /usr/bin/python3

# kw_utils\py
# 
# Miscellaneous utilities

# get division operator '/' vs. '//'
# (e.g., 1/2 == 0.5; 1//2 == 0)
from __future__ import division

__author__  = 'K Wilson'
__version__ = "0.0.1"

def C_to_F(dC):
    """ convert Celcius to Fahrenheit """
    dC      = float(dC)
    dF      = (dC * (9.0/5.0)) + 32
    return dF

def twos_comp(rawval, nbits):
    """compute the 2's compliment in n bits"""
    rawval = int(rawval)
    if( (rawval&(1<<(nbits-1))) != 0 ):
        rawval = rawval - (1<<nbits)
    return rawval

def check_reboots(col_time):
    last_ts = 0;
    reboots   = 0;
    line    = 0;

    last_ts = col_time[0]

    for i in col_time:
        line = line+1
        if (i - last_ts) > 1:
            print("error at line: ", line, '\t', i, '\t', last_ts, '\t', i - last_ts)
            reboots = reboots+1
        last_ts =i
