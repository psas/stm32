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
# msg = bytearray()
# msg.extend(struct.pack("I",len(points))
# 
# 
#         ypeError: unsupported operand type(s) for <<: 'str' and 'int'
#         >>> x
#         '3'
#         >>> x = 3
#         >>> y = x <<8
#         >>> y
#         768
#         >>> pow(3,8)
#         6561
#         >>> bin(x)
#         '0b11'
#         >>> bin(y)
#         '0b1100000000'
#         >>> y ^= 0b11
#         >>> y
#         771
#         >>> bin(y)
#         '0b1100000011'
#         >>> 
#         ~  > 
# 
# 
# 
# 
# 

def list_to_psas_ts(sixbytes):
    psas_ts = long(0)
    j       = 0
    for i in reversed(range(0,6)):
        print(sixbytes[j])
        psas_ts ^= long(sixbytes[j]) << i * 8
        j       += 1
    print (long(psas_ts))
    return long(psas_ts & 0xffffffffffff)
        
def read_sdlogfile(infile, msgsize):
    f     = open(infile, "rb")
    block = f.read(msgsize)
    while (len(block) == msgsize):
        # Do stuff with byte.
        (c1,c2,c3,c4,     )          = struct.unpack('4c', block[0:4])         # 4 chars
        (index,           )         = struct.unpack('I',  block[4:8])         # uint32
        # (ts1,ts2,ts3,ts4,ts5,ts6,  ) = struct.unpack('6b',  block[8:14])        # timespec, 6 bytes
        foo = struct.unpack('6B',  block[8:14])        # timespec, 6 bytes
        psas_ts = list_to_psas_ts(foo)
        (data_len,  )                = struct.unpack('I',  block[14:18])

        ident                        = ''.join([c1,c2,c3,c4])

        print("ident    = " + ident)
        print("index    = " + str(index))
#        print("timespec = " + str(ts1))
        #print("timespec = " + str(psas_ts))
        print("timespec = " + str(foo))
        print("deta_len = " + str(data_len))
        print("--\n")
        block = f.read(2)
        (checksum, ) = struct.unpack('H', block[0:3]);
        print( "checksum = "  + str(checksum))
        # check for end of data

        block    = f.read(msgsize)

    f.close()


if __name__ == "__main__":
    try:
        default_infile  = "LOGSMALL.bin"
        default_msgsize = 166

        #  parse command line
        usage = "usage: %prog --msgsize int --infile string  [-h|--help]"
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
                + " --infile  " + infile\
                + " --msgsize " + str(msgsize)\
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





