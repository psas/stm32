#! /usr/bin/python3

# mpu_convert.py
# 

# get division operator '/' vs. '//'
# (e.g., 1/2 == 0.5; 1//2 == 0)
from __future__ import division

__author__  = 'K Wilson'
__version__ = "0.0.1"

import numpy           as np
import csv_readmpu     as csvrd
import kw_utils        as u

def mpu_raw_gyro_to_dps(rawval, abs_fullscale):
    """
        mpu9150 gyro rate is stored as a 16 bit twos compliment value
        
        abs_fullscale: 
        The full scale ranges are (+/- 250, 500, 1000, 2000 dps) in 16 bits.
        'abs_fullscale' is one half of the total +/- range.
        for +/- 250 g the total range is [-250,250] which is 500dps. 
        
        This should mean:
        abs_fullscale                  LSB Sensitivity
            250                            0.00764(degrees/bit)/s ((131 LSB/degree)/s)
            500                            0.01527(degrees/bit)/s ((65.5 LSB/degree)/s)
            1000                           0.03049(degrees/bit)/s ((32.8 LSB/degree)/s)
            2000                           0.06098(degrees/bit)/s ((16.4 LSB/degree)/s)
        
    """
    rawval        = int(rawval)
    rawval        = u.twos_comp(rawval&0xffff,16)
    gyrodata       = 0.0
    
    if(abs_fullscale==250):
        gyrodata   = rawval * 0.00764
    elif(abs_fullscale==500):
        gyrodata   = rawval * 0.01527
    elif(abs_fullscale==1000):
        gyrodata   = rawval * 0.03049
    elif(abs_fullscale==2000):
        gyrodata   = rawval * 0.06098
    else:
        raise TypeError("abs_fullscale must be 250, 500, 1000 or 2000")

    return gyrodata

def mpu_col_gyro_to_values(col_gyro):
    """
        dps is degrees-per-second
    """
    col_xyz    = col_gyro[:,1:4]
               
    vfunc      = np.vectorize(mpu_raw_gyro_to_dps)
    
    col_xyz_dps = vfunc(col_xyz, 500)
    
    return col_xyz_dps

def mpu_raw_acc_to_ug(rawval, abs_fullscale):
    """
        mpu9150 acceleration is stored as a 16 bit signed value
        
        abs_fullscale: The RM-MPU manual is not clear on this.
        The full scale ranges are (+/- 2, 4, 8 16g) in 16 bits.
        'abs_fullscale' is one half of the total +/- range.
        for +/- 2 g the total range is [-2,2] which is 4g. 
        
        This should mean:
        abs_fullscale                  LSB Sensitivity
            2                               61ug/bit  (16384bits/g)
            4                              122ug/bit
            8                              244ug/bit
            16                             488ug/bit
        
    """
    rawval        = int(rawval)
    rawval        = u.twos_comp(rawval&0xffff,16)
    accdata       = 0.0
    
    if(abs_fullscale==2):
        accdata   = rawval * 61e-6
    elif(abs_fullscale==4):
        accdata   = rawval * 122e-6
    elif(abs_fullscale==8):
        accdata   = rawval * 244e-6
    elif(abs_fullscale==16):
        accdata   = rawval * 488.3e-6
    else:
        raise TypeError("abs_fullscale must be 2, 4, 8 or 16")

    return accdata

def mpu_col_acc_to_values(col_acc):

    col_xyz    = col_acc[:,1:4]
               
    vfunc      = np.vectorize(mpu_raw_acc_to_ug)
    
    col_xyz_ug = vfunc(col_xyz, 16)
    
    return col_xyz_ug

    
def mpu_raw_temp_to_dC(rawval):
    """
        mpu9150 temperature is stored as a 16 bit signed value
        The conversion to temperature in C is in RM-MPU9150.pdf
    """
    rawval      = int(rawval)
    tdata_twos  = u.twos_comp(rawval&0xffff,16)
    tdata       = float(tdata_twos)
    
    tdata       = (tdata/340.0) + 35.0
   
    return tdata


def mpu_col_t_to_values(col_t):

    col_t      = col_t[:,1]
               
    vfunc      = np.vectorize(mpu_raw_temp_to_dC)
    
    col_t_dC   = vfunc(col_t)
    
    return col_t_dC

if __name__ == "__main__":
    try:
        infile     = "testdata.csv"
        outfile    = open("testoutput.csv", 'w')
        
        headers    = csvrd.csv_mpu_getheaders(infile)
        data_all   = csvrd.csv_mpu_getdata(infile)
        col_time   = data_all[:,0]
        col_acc    = csvrd.csv_mpu_get_accel_xyz(infile)
        col_gyro   = csvrd.csv_mpu_get_gyro_xyz(infile)
        col_dC     = csvrd.csv_mpu_get_t_data(infile)
        
        np.set_printoptions(threshold=np.nan, precision=6, linewidth=200)
        #print(list(map(mpu_raw_temp_to_dC,col_dC[:,1])))
        #print(mpu_col_acc_to_values(col_acc))
        #print(type(mpu_col_t_to_values(col_dC)))
        #print(list(map(u.C_to_F,mpu_col_t_to_values(col_dC))) )
        #print(list(map(mpu_raw_temp_to_dC,col_dC[:,1])))
        
        print("accel(ug)")
        print("\ttime\t\tx\t\ty\t\tz")
        
        a = mpu_col_acc_to_values(col_acc)
        print(a.shape)
        q = np.atleast_3d(col_time)
        print(q.shape)
        q = np.hstack((q, a))
        
        print(q)
        
        #print("gyro dps")
        #print("\ttime\t\tx\t\ty\t\tz")
        #print(col_gyro)
        #
        #print("\ttime\t\tTemp(C)")
        #print(col_dC)
        #
        #print("Done")
        outfile.close()
        
    except KeyboardInterrupt:
        print ("\nQuitting")
        

        