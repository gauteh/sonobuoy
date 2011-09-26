#! /usr/bin/python2
#
# Interfaces with ZeroNode and logs data from all nodes
#
# Requires pyserial

import serial
import signal
import time
from threading import Thread
import multiprocessing, logging
import sys

from ad import *
from protocol import *
from buoy import *

class Zero:
  port = '/dev/ttyUSB0'
  baud = 115200
  ser  = None

  logger = None

  protocol = None
  ad       = None

  current  = None

  # Reading thread
  go       = True
  stopping = False

  def __init__ (self):
    self.logger = multiprocessing.log_to_stderr ()
    self.logger.setLevel (logging.INFO)

    self.logger.info( "Starting Zero..")

    # Setting up signals..
    signal.signal (signal.SIGINT, self.sigterm)

    # Currently receiving buoy object
    self.current = Buoy (self)

    # Protocol handler; receives data from ZeroNode
    self.protocol = Protocol (self)

    self.openserial ()

    self.main ()

  def openserial (self):
    self.logger.info ("Opening serial port " + str(self.port) + "..")
    try:
      self.ser = serial.Serial (self.port, self.baud)
    except:
      self.logger.info ("Failed to open serial port..")
      self.ser = None
      #self.stop ()

  def closeserial (self):
    try:
      if self.ser != None:
        self.ser.close ()
    except: pass

  def main (self):
    self.logger.info ("Starting main loop..")
    while self.go:
      if not self.ser == None:
        r = self.ser.read (80)
        self.protocol.handle (r)

      time.sleep (0.000001)

    self.logger.info ("Main loop finished..")

  def sigterm (self, signum, frame):
    if not self.stopping:
      self.stopping = True
      self.logger.info ("Got SIGTERM..")
      self.stop ()

  def stop (self):
    self.logger.info ("Stopping Zero..")
    self.go = False
    self.closeserial ()

    # Stop all Buoys..
    if self.current:
      self.current.stop ()

    sys.exit (0)

if __name__ == '__main__':
  z = Zero ()



