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

from ui import *

class Zero:
  port = '/dev/ttyUSB0'
  baud = 115200
  ser  = None

  uimanager = None
  uiservicethread = None

  logger = None

  protocol = None # Serial protocol to ZeroNode

  buoys    = []   # List of Buoys
  currenti = 0 # Current buoy index

  # Reading thread
  go       = True

  def get_current (self):
    try:
      return self.buoys[self.currenti]
    except ValueError as e:
      return None

  def set_current (self, b):
    if self.current:
      self.current.active = False

    self.currenti = self.buoys.index(b)

    self.current.active = True
    self.logger.info ("Setting current Buoy to: " + b.name)

  current  = property(get_current, set_current) # Current Buoy

  def __init__ (self):
    self.logger = multiprocessing.log_to_stderr ()
    self.logger.setLevel (logging.INFO)

    self.logger.info( "Starting Zero..")

    # Currently receiving buoy object, hardcode to 'One'.
    # TODO: Do multicast to map available buoys, also do every now and then.
    #       Or have ZeroNode do that..
    #
    # Each node should register now and then as well..

    self.buoys.append (Buoy(self, 'One'))
    #self.buoys.append (Buoy(self, 'Two'))
    #self.buoys.append (Buoy(self, 'Three'))
    self.set_current (self.buoys[0])

    # Protocol handler; receives data from ZeroNode
    self.protocol = Protocol (self)

    # Start UI manager
    self.uimanagerthread = Thread (target = self.run_ui_service, name = 'ZeroUIService')
    self.uimanagerthread.daemon = True
    self.uimanagerthread.start ()

    # Start thread reading stdin
    t = Thread (target = self.stdin, name = 'StdinHandler')
    t.daemon = True
    t.start ()

    self.main ()

  def run_ui_service (self):
    self.uimanager = ZeroUIManager ()
    self.uimanager.setup_server (self)
    self.uimanagerserver = self.uimanager.get_server ()
    self.uimanagerserver.serve_forever ()


  def openserial (self):
    while (self.ser == None or not self.ser.isOpen ()):
      self.logger.info ("Opening serial port " + str(self.port) + "..")
      try:
        self.ser = serial.Serial (self.port, self.baud)
        self.logger.info ("Serial port open.")
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

  def get_buoys (self):
    for i in self.buoys:
      yield i

  def main (self):
    try:
      self.logger.info ("Entering main loop..")
      self.openserial ()

      while self.go:
        try:
          if not self.ser == None:
            r = self.ser.read (1024)
            if self.current is not None:
              self.protocol.handle (r)

          time.sleep (0.01)
        except serial.SerialException as e:
          self.logger.error ("Exception with serial link, reconnecting..: " + str(e))
          self.closeserial ()
          self.openserial ()


    except Exception as e:
      self.logger.error ("General exception in main loop: " + str(e))

    finally:
      self.logger.info ("Main loop finished..")
      self.closeserial ()
      self.stop ()

  def stdin (self):
    # Wait for input on stdin, then exit..

    raw_input ()
    self.go = False

  def stop (self):
    self.logger.info ("Stopping Zero..")
    self.go = False

    # Stop all Buoys..
    if self.current:
      self.current.deactivate ()
      self.current = None

    for i in self.buoys:
      i.stop ()


    sys.exit (0)

if __name__ == '__main__':
  z = Zero ()



