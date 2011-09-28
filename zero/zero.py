#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2011-09-26
#
# Central logging and managing point 
#
# Requires:
#  - pyserial

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

  protocol = None # Serial protocol to ZeroNode

  buoys    = []   # List of Buoys
  current  = None # Current Buoy

  # Reading thread
  go       = True

  def __init__ (self):
    self.logger = multiprocessing.log_to_stderr ()
    self.logger.setLevel (logging.DEBUG)

    self.logger.info( "Starting Zero..")

    # Setting up signals..
    signal.signal (signal.SIGINT, self.sigterm)

    # Currently receiving buoy object, hardcode to 'One'.
    # TODO: Do multicast to map available buoys, also do every now and then.
    #       Or have ZeroNode do that..
    #
    # Each node should register now and then as well..

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
    while (self.ser == None or not self.ser.isOpen ()):
      self.logger.info ("Opening serial port " + str(self.port) + "..")
      try:
        self.ser = serial.Serial (self.port, self.baud)
      except:
        self.logger.info ("Failed to open serial port.. retrying in 5 seconds.")
        self.ser = None
        #self.stop ()
        time.sleep(5)

  def closeserial (self):
    try:
      if self.ser != None:
        self.ser.close ()
    except: pass

    self.ser = None

  def main (self):
    self.logger.info ("Entering main loop..")
    while self.go:
      if not self.ser == None:
        try:
          r = self.ser.read (80)
          self.protocol.handle (r)
        except serial.SerialException as e:
          self.logger.error ("Exception with serial link, reconnecting..: " + str(e))
          self.closeserial ()
          self.openserial ()

        except Exception as e:
          self.logger.error ("General exception in main loop: " + str(e))
          self.stop ()

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



