#!/usr/bin/python3 

# Read the mpu log file.  
# Return a numpy array of the data 
# Return an array of header strings 

# for file opening made easier 
#from __future__ import with_statement import numpy as np 

import numpy as np
import kw_utils as u
__author__  = 'K Wilson'
__version__ = "0.0.1"

## Notes
# Adding a row to a numpy array
#   newrow = [1,2,3]
#   A = numpy.vstack([A, newrow])

def csv_mpu_getdata(filename):
    with open(filename) as f:
        # y = np.loadtxt(f, delimiter=',', dtype=float,converters={ 2: (lambda d : int(d)) }, comments='#', usecols=list(range(1,12)))
        #y = np.loadtxt(f, delimiter=',', dtype=float, comments='#', usecols=list(range(1,9)))
        y = np.loadtxt(f, delimiter=',',  usecols=list(range(1,9)), comments='#', dtype=float, converters=dict(zip((1,2,3,4,5,6,7,8), (float, u.get_hex, u.get_hex, u.get_hex, u.get_hex, u.get_hex, u.get_hex, u.get_hex))))
        np.set_printoptions(threshold=np.nan, precision=2, linewidth=200)
        f.close()
        return y

def csv_mpu_get_accel_xyz(filename):
    with open(filename) as f:
        # y = np.loadtxt(f, delimiter=',', dtype=float,converters={ 2: (lambda d : int(d)) }, comments='#', usecols=list(range(1,12)))
        y = np.loadtxt(f, delimiter=',', comments='#', usecols=list(range(1,5)), converters = dict(zip((1,2,3,4), (float,int,int,int))) ) 
        #np.set_printoptions(threshold=np.nan, precision=2, linewidth=200)
        f.close()
        return y

def csv_mpu_get_gyro_xyz(filename):
    with open(filename) as f:
        # y = np.loadtxt(f, delimiter=',', dtype=float,converters={ 2: (lambda d : int(d)) }, comments='#', usecols=list(range(1,12)))
        y = np.loadtxt(f, delimiter=',',  comments='#', usecols=(1,5,6,7), converters = dict(zip((1,5,6,7), (float,int,int,int)) ))
        #np.set_printoptions(threshold=np.nan, precision=2, linewidth=200)
        f.close()
        return y
    
def csv_mpu_get_t_data(filename):
    with open(filename) as f:
        # y = np.loadtxt(f, delimiter=',', dtype=float,converters={ 2: (lambda d : int(d)) }, comments='#', usecols=list(range(1,12)))
        y = np.loadtxt(f, delimiter=',', comments='#', usecols=(1,8),converters = dict(zip((1,8), (float,int))) )
        #np.set_printoptions(threshold=np.nan, precision=2, linewidth=200)
        f.close()
        return y

def csv_mpu_getheaders(filename):
    with open(filename) as f:
        headers = f.readline().split(',')
        headers = f.readline().split(',')
        headers = f.readline().split(',')

        j = 0
        for i in headers:
            headers[j] = i.strip()
            j = j+1
        f.close()
        return headers



if __name__ == "__main__":
    try:
        filename = 'testdata.csv'
#         headers  = csv_mpu_getheaders(filename)
#         accel    = csv_mpu_get_accel_xyz(filename)
#         gyro     = csv_mpu_get_gyro_xyz(filename)
#         dC       = csv_mpu_get_t_data(filename)
        y        = csv_mpu_getdata(filename)
        
        print(y)
        
        #print(headers)
        #print(y)

    except KeyboardInterrupt:
        print ("\nQuitting")

