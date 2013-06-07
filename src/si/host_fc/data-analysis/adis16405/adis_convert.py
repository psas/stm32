#! /usr/bin/python3

# adis_convert.py
# 

# get division operator '/' vs. '//'
# (e.g., 1/2 == 0.5; 1//2 == 0)
from __future__ import division

__author__  = 'K Wilson'
__version__ = "0.0.1"

import time
import numpy           as np
import csv_readadis    as csvrd
import kw_utils        as u

def adis_raw_magn_to_ugauss(rawval):
    """
        adis gyro is 14 bit twos complement
       
        0.5 mgauss per bit 500ugauss per bit
        
    """
    rawval           = int(rawval)
    rawval           = u.twos_comp(rawval&0x3fff,14)
    magndata_ugauss  = rawval * 500
    return magndata_ugauss

def adis_col_magn_to_values(col_magn):
    """
        dps is degrees-per-second
    """
    col_xyz    = col_magn[:,1:4]
               
    vfunc      = np.vectorize(adis_raw_magn_to_ugauss)
    
    col_xyz_ugauss = vfunc(col_xyz)
    
    return col_xyz_ugauss

def adis_raw_gyro_to_dps(rawval):
    """
        adis gyro is 14 bit twos complement
        
        0.05 dps per bit
        
    """
    rawval        = int(rawval)
    rawval        = u.twos_comp(rawval&0x3fff,14)
    gyrodata_dps  = rawval * 0.05
    return gyrodata_dps

def adis_col_gyro_to_values(col_gyro):
    """
        dps is degrees-per-second
    """
    col_xyz    = col_gyro[:,1:4]
               
    vfunc      = np.vectorize(adis_raw_gyro_to_dps)
    
    col_xyz_dps = vfunc(col_xyz)
    
    return col_xyz_dps


def adis_raw_acc_to_ug(rawval):
    """
        adis accel data is 14 bit twos complement.
        
        3.3mg per bit or 3330ug per bit
        return value in  ug  (micro-g)
        
    """
    rawval        = int(rawval)
    rawval        = u.twos_comp(rawval&0x3fff,14)
    accdata_ug    = rawval * 3330
   
    return accdata_ug

def adis_col_acc_to_values(col_acc):

    col_xyz    = col_acc[:,1:4]
               
    vfunc      = np.vectorize(adis_raw_acc_to_ug)
    
    col_xyz_ug = vfunc(col_xyz)
    
    return col_xyz_ug

    
def adis_raw_temp_to_dC(rawval):
    """
        mpu9150 temperature is stored as a 16 bit signed value
        The conversion to temperature in C is in RM-MPU9150.pdf
    """
    rawval      = int(rawval)
    tdata_twos  = u.twos_comp(rawval&0xffff,16)
    tdata       = float(tdata_twos)
    
    tdata       = (tdata/340.0) + 35.0
   
    return tdata


def adis_col_t_to_values(col_t):

    col_t      = col_t[:,1]
               
    vfunc      = np.vectorize(mpu_raw_temp_to_dC)
    
    col_t_dC   = vfunc(col_t)
    
    return col_t_dC

if __name__ == "__main__":
    try:
        infile     = "testdata.csv"
        outfile    = open("testoutput.csv", 'w')
        
        headers    = csvrd.csv_adis_getheaders(infile)
        data_all   = csvrd.csv_adis_getdata(infile)
        col_time   = data_all[:,0]
        col_acc    = csvrd.csv_adis_get_accel_xyz(infile)
        col_gyro   = csvrd.csv_adis_get_gyro_xyz(infile)
        col_magn   = csvrd.csv_adis_get_magn_xyz(infile)
        col_dC     = csvrd.csv_adis_get_t_data(infile)
        
        acc_xyz    = adis_col_acc_to_values(col_acc)
        gyro_xyz   = adis_col_gyro_to_values(col_gyro)
        magn_xyz   = adis_col_magn_to_values(col_magn)

        print(acc_xyz)
        print(gyro_xyz)
        print(magn_xyz)
        
#         now        = time.strftime("%c", time.gmtime())
#         outfile.write('# %s\n'% now)
#         for i in headers:
#             outfile.write(i)
#             outfile.write('\t\t')
#         outfile.write('\n')
#         for i in range(0,len(col_time)):
#             print("%.6f" % col_time[i], '\t', col_acc[i][1:4],'\t', col_gyro[i][1:4], '\t', col_magn[i][1:4], col_dC[i][1] , file=outfile)
#         
#         print("Done")
        outfile.close()
        
    except KeyboardInterrupt:
        print ("\nQuitting")
        

        