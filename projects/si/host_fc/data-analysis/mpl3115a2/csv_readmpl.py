#!/usr/bin/python3 

# Read the mpl log file.  
# Return a numpy array of the data 
# Return an array of header strings 

# for file opening made easier 
#from __future__ import with_statement import numpy as np 

import numpy as np

__author__  = 'K Wilson'
__version__ = "0.0.1"

## Notes
# Adding a row to a numpy array
#   newrow = [1,2,3]
#   A = numpy.vstack([A, newrow])

def csv_mpl_getdata(filename):
    with open(filename) as f:
        # y = np.loadtxt(f, delimiter=',', dtype=float,converters={ 2: (lambda d : int(d)) }, comments='#', usecols=list(range(1,12)))
        y = np.loadtxt(f, delimiter=',', dtype=float, comments='#', usecols=list(range(1,3)))
        np.set_printoptions(threshold=np.nan, precision=2, linewidth=200)
        f.close()
        return y

def csv_mpl_get_pressure_data(filename):
    with open(filename) as f:
        # y = np.loadtxt(f, delimiter=',', dtype=float,converters={ 2: (lambda d : int(d)) }, comments='#', usecols=list(range(1,12)))
        y = np.loadtxt(f, delimiter=',', dtype=int, comments='#', usecols=list(range(2,3)))
        #np.set_printoptions(threshold=np.nan, precision=2, linewidth=200)
        f.close()
        return y

def csv_mpl_get_t_data(filename):
    with open(filename) as f:
        # y = np.loadtxt(f, delimiter=',', dtype=float,converters={ 2: (lambda d : int(d)) }, comments='#', usecols=list(range(1,12)))
        y = np.loadtxt(f, delimiter=',', dtype=int, comments='#', usecols=list(range(3,4)))
        #np.set_printoptions(threshold=np.nan, precision=2, linewidth=200)
        f.close()
        return y

# open CSV file
def csv_mpl_getheaders(filename):
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
        headers  = csv_mpl_getheaders(filename)
        y        = csv_mpl_getdata(filename)

    #    print (y[0])
    #    print (y[2])
    #    print (float(y[2,0]),'\t', int(y[2,1]),'\t', int(y[2,2]), '\t', int(y[2,3]))
 
        print(headers)
        print(y)

    except KeyboardInterrupt:
        print ("\nQuitting")

