#! /usr/bin/python3

# mpl_convert.py
# 

# get division operator '/' vs. '//'
# (e.g., 1/2 == 0.5; 1//2 == 0)
from __future__ import division

__author__  = 'K Wilson'
__version__ = "0.0.1"

import numpy       as np
import csv_readmpl as csvrd
import kw_utils    as u

def mpl_raw_temp_to_dC(rawval):
    """
        mpl3115a2 temperature is represented as a
        signed 8 bit integer and a fractional 4 bit
        component.
    """
    if not isinstance(rawval, np.int64):
        print(type(rawval))
        raise TypeError("rawval must be an numpy.int64")
    tdata_8       = 0.0
    tdata_8_twos  = u.twos_comp((rawval>>8)&0xff, 8)
    tdata_frac    = (rawval&0xf0)  >> 4
    tdata_8       = float(tdata_8_twos)
    if((tdata_frac & 0b1000)!=0):
        tdata_8 += 0.5
    if((tdata_frac & 0b0100)!=0):
        tdata_8 += 0.25
    if((tdata_frac & 0b0010)!=0):
        tdata_8 += 0.125
    if((tdata_frac & 0b0001)!=0):
        tdata_8 += 0.0625
    
    return tdata_8
    
    
def mpl_raw_pres_to_pascals(rawval):
    """
        mpl3115a2 pressure is represented as an 
        unsigned int concat(OUT_P_MSB, OUT_P_CSB, OUT_P_LSB)
        in 18 bytes. The bits 5 and 4 represent fractional values.
    """
    if not isinstance(rawval, np.int64):
        print(type(rawval))
        raise TypeError("rawval must be an numpy.int64")
    pdata_18       = 0.0
    pdata_18_int   = rawval        >>6
    pdata_frac     = (rawval&0x30) >> 4
    pdata_18       = float(pdata_18_int)
    if((pdata_frac&0b1) == 0b1):
        pdata_18   = pdata_18_int + 0.25
    if((pdata_frac&0b10) == 0b10):
        pdata_18   = pdata_18_int +  0.5
    return pdata_18

if __name__ == "__main__":
    try:
        infile     = "testdata.csv"
        outfile    = open("testoutput.csv", 'w')
        
        headers    =  csvrd.csv_mpl_getheaders(infile)
        data_all   = csvrd.csv_mpl_getdata(infile)
        col0_time  = data_all[:,0]
        col1_pres  = csvrd.csv_mpl_get_pressure_data(infile)
        col2_dC    = csvrd.csv_mpl_get_t_data(infile)
         # >>> time.strftime("%H:%M %Y", time.gmtime(1369870218.38))
         # '23:30 2013'

        pr_data    = list(map(mpl_raw_pres_to_pascals, col1_pres))
        t_data     = list(map(mpl_raw_temp_to_dC,      col2_dC))
        #outfile.write('# ')
        for i in headers:
            outfile.write(i)
            outfile.write('\t\t')
        outfile.write('\n')
        for i in range(0,len(col0_time)):
            print("%.6f" % col0_time[i], '\t', pr_data[i],'\t', t_data[i],'\t', file=outfile)
            
        
        outfile.close()
        
    except KeyboardInterrupt:
        print ("\nQuitting")
