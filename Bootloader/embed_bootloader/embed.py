#!/usr/bin/python

#this script reads in a .bin file and outputs to STDOUT a C header file that can
#be included in a user application
#
#usage:  embed.py bootloader.bin > embed_bootloader.h
#
#after generating embed_bootloader.h, compile the bootloader update application

import sys
import re
import binascii

lines = 0
count = 0
temp_byte = bytearray()

print "static const int bootloader[8192] = {"

#output binary
f = open(sys.argv[1], "rb")
try:
    byte = f.read(4)
    while byte != "":
      temp_byte = bytearray(byte)
      temp_byte.reverse()
      #print "%s%s %s" % ("0x", binascii.hexlify(bytearray(byte)), " ,")
      print "%s%s %s" % ("0x", binascii.hexlify(temp_byte), " ,")
      if lines == 1: #check that it is a bootloader binary
        temp_byte = bytearray(byte)
        temp_byte.reverse()
        reset_addr = int(binascii.hexlify(temp_byte), 16)
        if reset_addr > 0x08010000 or reset_addr < 0x08000000:
          print "invalid reset address - this is a user application binary"
          sys.exit()
      byte = f.read(4)
      lines = lines + 1
finally:
  f.close()


print "0 };"
print "static const int lines = " + str(lines+1) + ";"

