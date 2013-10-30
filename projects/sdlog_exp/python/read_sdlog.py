#!/usr/bin/python3

# read_data.py
# read a binary file written by the psas_sdlog interface to the sd card
#

# get division operator '/' vs. '//'
# (e.g., 1/2 == 0.5; 1//2 == 0)
# Q: Does python 3 still need this?
# A: NO.
# from __future__ import division

import sys
if sys.version_info < (3, 0):
    sys.stdout.write("Sorry, this application requires Python 3.x.\n")
    sys.exit(1)

import struct

__author__  = 'K Wilson'
__version__ = "0.0.1"

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

def read_sdlogfile(infile):
    f     = open(infile, "rb")
    block = block = f.read(msgsize)
    while (len(block) == msgsize):
        # Do stuff with byte.
        (id, )       = struct.unpack('4c', block[0:4]

        (index,    ) = struct.unpack('I', block[0:4])
        (tv_date,  ) = struct.unpack('I', block[4:8])
        (tv_time,  ) = struct.unpack('I', block[8:12])
        hour_fmt     = struct.unpack('4?', block[12:16])[0]
            #        hour_fmt     = block[12]
            # Don't understand pad bytes yet...
            #        (pad, )      = struct.unpack('x', block[13])
        (tv_msec,  ) = struct.unpack('I', block[16:20])
        print( "index   = " + str(index))
        print( "tv_date = " + str(tv_date))
        print( "tv_time = " + str(tv_time))
        print( "hour_fmt= " + str(hour_fmt))
        print( "tv_msec = " + str(tv_msec))
        print("--\n")
#        print( struct.unpack('B', byte)[0] )
        block = f.read(20)
    f.close()


if __name__ == "__main__":
    try:
        default_infile  = "LOGSMALL.bin"
        default_msgsize = 166

        #  parse command line
        usage = "usage: %prog --infile string  [-h|--help]"
        parser = OptionParser(usage=usage)
        parser.add_option(\
                "-i", "--infile", \
                dest="infile", \
                help="Input file name.", \
                default=default_infile)
        
        parser.add_option(\
                "-s", "--msgsize", \
                dest="msgsize", \
                help="Generic Message size in bytes.", \
                default=default_msgsize)


        (options, args) = parser.parse_args()

        progname    = os.path.basename(sys.argv[0])
        progname    = os.path.join(sys.path[0], progname)

        run_command = " ".join(sys.argv)
        now         = datetime.datetime.today()
        now         = now.strftime("%Y-%m-%d--%H-%M")

        infile      = options.infile
        msgsize     = options.msgsize

        if infile == "":
            print ("No infile file name.")
            print (usage)
            raise MissingOption("No input file name supplied.")           

        if infile==default_infile:
            print ("Using default input file: " + infile)

        if msgsize == "":
            print ("No message size supplied.")
            print (usage)
            raise MissingOption("No message size supplied.")           

        if msgsize==default_msgsize:
            print ("Using default message size: " + msgsize)
 
        print ("command: ")
        print (run_command + "\n")

        runsettings = progname\
                + " --infile " + infile\
                + "\n"
        print ("settings: ")
        print (runsettings)

        # end administrative 

        read_sdlogfile(infile)


    except BadFileRead as e:
        print ('BadFileRead exception occurred, value:', e.value)
    except CommandFailed as e:
        print ('CommandFailed exception occurred, value:', e.value)
    except MissingOption as e:
        print ('Missing Option:', e.value)
    except KeyboardInterrupt:
        print ("\nQuitting")

    finally:
        print("\n------------\nDone\n")





