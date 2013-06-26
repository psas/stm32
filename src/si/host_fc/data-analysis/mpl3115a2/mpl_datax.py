#! /usr/bin/python3

# mpl_datax.py
# 
#

# get division operator '/' vs. '//'
# (e.g., 1/2 == 0.5; 1//2 == 0)
from __future__ import division

__author__  = 'K Wilson'
__version__ = "0.0.1"


import time
import csv_readmpl as csvrd
import mpl_convert as mpl

# cmd line parsing
#from operator import itemgetter, attrgetter
from optparse import OptionParser

# math
#import math
#import numpy as np

# plotting
#import pylab as p

#from matplotlib import cm

#import matplotlib.mlab as mlab
#import matplotlib.pyplot as plt

# system
import os
#import random
#import re
import datetime
import sys
#import time

now          = datetime.datetime.today()
#now         = time.strftime("%Y-%m-%d %H:%M %z",time.gmtime())
now          = now.strftime("%a %Y-%m-%d %H:%M %Z")
source_info  = os.path.basename(sys.argv[0]) + " \"" + " ".join(sys.argv) + " \"" + " on: " + now

class BadFileRead(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class CommandFailed(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class MissingOption(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)


if __name__ == "__main__":
    try:
        default_infile     = "mpl3115a2_log.txt"
        default_outputfile = "mpl3115a2_converted.txt"
        default_dC   = 1

    #  parse command line
        usage = "usage: %prog --infile string --outputfile string [-h|--help]"
        parser = OptionParser(usage=usage)
        parser.add_option(\
                "-i", "--infile", \
                dest="infile", \
                help="Input file name.", \
                default=default_infile)
        parser.add_option(\
                "-o", "--outputfile", \
                dest="outputfile", \
                help="Output file name.", \
                default=default_outputfile)

        (options, args) = parser.parse_args()

        progname    = os.path.basename(sys.argv[0])
        progname    = os.path.join(sys.path[0], progname)

        run_command = " ".join(sys.argv)
        now         = datetime.datetime.today()
        now         = now.strftime("%Y-%m-%d--%H-%M")

        infile      = options.infile
        outputfile     = options.outputfile

        

#         if infile==default_infile or infile == "":
#             print ("No infile file name.")
#             print (usage)
#             raise MissingOption("No input file name supplied.")           

        print ("command: ")
        print (run_command + "\n")

        runsettings = progname\
                + " --infile " + infile\
                + " --outfile " + outputfile\
                + "\n"
        print ("settings: ")
        print (runsettings)

########################
# end administrative
########################
        outfile    = open(outputfile, 'w')
        headers    = csvrd.csv_mpl_getheaders(infile)
        data_all   = csvrd.csv_mpl_getdata(infile)
        col0_time  = data_all[:,0]
        col1_pres  = mpl.csvrd.csv_mpl_get_pressure_data(infile)
        col2_dC    = csvrd.csv_mpl_get_t_data(infile)

        pr_data    = list(map(mpl.mpl_raw_pres_to_pascals, col1_pres))
        t_data     = list(map(mpl.mpl_raw_temp_to_dC,      col2_dC))
        
        now        = time.strftime("%c", time.gmtime())
        outfile.write('# %s\n' % now)
        
        for i in headers:
            outfile.write(i)
            outfile.write('\t\t')
        outfile.write('\n')
        for i in range(0,len(col0_time)):
            print("%.6f" % col0_time[i], '\t', pr_data[i],'\t', t_data[i], '\t', file=outfile)
        
        outfile.close()

    except BadFileRead as e:
        print ('BadFileRead exception occurred, value:', e.value)
    except CommandFailed as e:
        print ('CommandFailed exception occurred, value:', e.value)
    except MissingOption as e:
        print ('Missing Option:', e.value)
    except KeyboardInterrupt:
        print ("\nQuitting")



