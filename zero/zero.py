#! /usr/bin/python2
#
# Interfaces with ZeroNode
#

import serial

from ad import *
from protocol import *
from buoy import *

class Zero:
  port = '/dev/ttyUSB0'
  baud = 115200
  ser  = None

  go   = True

  protocol = None
  ad       = None

  current  = None
  
  def __init__ (self):
    print "Starting Zero.."

    # Currently receiving buoy object
    self.current = Buoy (self)

    # Protocol handler; receives data from ZeroNode
    self.protocol = Protocol (self)

    self.openserial ()
    self.main ()

  def openserial (self):
    print "Opening serial port", self.port, ".."
    self.ser = serial.Serial (self.port, self.baud)


  def main (self):
    print "Starting main loop.."
    while self.go:
      r = self.ser.read (80)
      self.protocol.handle (r)

      if (self.current.ad.nsamples % 1000 == 0):
        self.current.log ()



if __name__ == '__main__':
  z = Zero ()




