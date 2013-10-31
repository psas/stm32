#!/usr/bin/env python

""" read_sdlog.py read a binary file written by the psas_sdlog interface to the sd card """

from optparse import OptionParser

import struct
import crcmod
import os
import datetime
import sys

#if sys.version_info < (3, 0):
#    sys.stdout.write("Sorry, this application requires Python 3.x.\n")
#    sys.exit(1)
#
__author__  = 'K Wilson'
__version__ = "0.0.1"

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

"""
    struct Message_head {
            char                 ID[SDC_NUM_ID_CHARS];         // This must be first part of data. Reserved value: 0xa5a5
            uint32_t             index;
            psas_timespec        ts;
            uint16_t             data_length;
            } __attribute__((packed));
    typedef struct Message_head Message_head;

    struct GENERIC_message {
            Message_head         mh;                           // 16 bytes
            uint8_t              data[SDC_MAX_PAYLOAD_BYTES];  // 150 bytes
            } __attribute__((packed));
    typedef struct GENERIC_message GENERIC_message;

"""

def read_sdlogfile(infile, msgsize):
    f     = open(infile, "rb")
    block = f.read(msgsize)
    while (len(block) == msgsize):
        # Do stuff with byte.
        (ident,     ) = struct.unpack('4c', block[0:4])         # 4 chars
        (index,     ) = struct.unpack('I',  block[4:8])         # uint32
        (timespec,  ) = struct.unpack('I',  block[8:14])        # timespec, 6 bytes
        (data_len,  ) = struct.unpack('I',  block[14:17])
        print( "ident    = " + str(ident))
        print( "index    = " + str(index))
        print( "timespec = " + str(timespec))
        print( "deta_len = " + str(data_len))
        print("--\n")
        checksum = f.read(2)
        print( "checksum = " + str(checksum))
        block    = f.read(msgsize)

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
            print ("Using default message size: " + str(msgsize))

        print ("command: ")
        print (run_command + "\n")

        runsettings = progname\
                + " --infile " + infile\
                + "\n"
        print ("settings: ")
        print (runsettings)

        # end administrative

        read_sdlogfile(infile, msgsize)

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





