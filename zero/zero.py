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

  buoys    = []
  current  = None

  # Reading thread
  go       = True

  def __init__ (self):
    self.logger = multiprocessing.log_to_stderr ()
    self.logger.setLevel (logging.INFO)

    self.logger.info( "Starting Zero..")

    # Setting up signals..
    signal.signal (signal.SIGINT, self.sigterm)

    # Currently receiving buoy object, hardcode to 'One'.
    # TODO: Do multicast to map available buoys, also do every now and then.
    #       Or have ZeroNode do that..

    self.buoys.append (Buoy(self, 'One'))
    self.setcurrent(self.buoys[0])

    # Protocol handler; receives data from ZeroNode
    self.protocol = Protocol (self)

    self.openserial ()

    self.main ()

  def setcurrent (self, b):
    if self.current: self.current.deactivate ()
    self.current = b
    self.current.activate ()
    self.logger.info ("Setting current Buoy to: " + b.name)

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
    self.logger.info ("Got SIGTERM..")
    self.stop ()

  def stop (self):
    self.logger.info ("Stopping Zero..")
    self.go = False
    self.closeserial ()

    # Stop all Buoys..
    if self.current:
      self.current.deactivate ()
      self.current = None

    for i in self.buoys:
      i.stop ()

    sys.exit (0)

if __name__ == '__main__':
  z = Zero ()



