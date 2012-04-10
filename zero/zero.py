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
from   threading import Thread
import multiprocessing, logging, logging.config
import sys

from ad       import *
from protocol import *
from buoy     import *
from ui       import *

from buoys    import buoys

class Zero:
  port = '/dev/ttyUSB0'
  baud = 115200
  ser  = None

  uimanager       = None
  uimanagerserver = None
  uiservicethread = None

  logger = None

  protocol = None # Serial protocol to ZeroNode

  buoys    = []   # List of Buoys
  currenti = 0    # Current buoy index

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
    self.logger.info ("[Zero] Setting current Buoy to: " + b.name)

  current  = property(get_current, set_current) # Current Buoy

  def __init__ (self):
    multiprocessing.process.current_process ().name = 'Main'

    logging.config.fileConfig ('zero.logging.conf')
    self.logger = logging.getLogger ('root')

    # Protocol handler; receives data from ZeroNode
    self.protocol = Protocol (self)

    self.logger.info ("==================================================")
    self.logger.info ("[Zero] Starting Zero..")
    self.logger.info ("[Zero] Logging to console and to file log/zero.log.")

    # Currently receiving buoy object, hardcode to 'One'.
    # TODO: Do multicast to map available buoys, also do every now and then.
    #       Or have ZeroNode do that..
    #
    # Each node should register now and then as well..

    for b in buoys:
      if b['enabled']:
        self.buoys.append (Buoy(self, b))

    self.set_current (self.buoys[0])

    # Start UI manager
    self.uimanagerthread = Thread (target = self.run_ui_service, name = 'ZeroUIService')
    self.uimanagerthread.daemon = True
    self.uimanagerthread.start ()

    # Start thread reading stdin
    #t = Thread (target = self.stdin, name = 'StdinHandler')
    #t.daemon = True
    #t.start ()

    self.main ()

  def run_ui_service (self):
    self.uimanager = ZeroUIManager ()
    self.uimanager.setup_server (self)
    self.uimanagerserver = self.uimanager.get_server ()
    self.uimanagerserver.serve_forever ()


  def openserial (self):
    while ((self.ser == None or not self.ser.isOpen ()) and self.go):
      self.logger.info ("[Zero] Opening serial port " + str(self.port) + "..")
      self.protocol.adressedbuoy = 0 # reset adressed buoy
      try:
        try:
          self.ser = serial.Serial (self.port, self.baud)
          self.logger.info ("[Zero] Serial port open.")
        except serial.SerialException as e:
          self.logger.error ("[Zero] Failed to open serial port.. retrying in 5 seconds.")
          self.ser = None
          time.sleep (5)

      except:
        self.stop ()

  def closeserial (self):
    try:
      if self.ser != None:
        self.ser.close ()
    except: pass

    self.ser = None

  def get_buoys (self):
    for i in self.buoys:
      yield i

  def send (self, msg):
    try:
      self.logger.debug ("[Zero] Sending: " + msg)
      self.ser.write (msg + "\n")
    except serial.SerialException as e:
      self.logger.exception ("[Zero] Exception with serial link, reconnecting..: " + str(e))
      self.closeserial ()
      self.openserial ()


  def main (self):
    try:
      self.logger.info ("[Zero] Entering main loop..")
      self.openserial ()

      while self.go:
        try:
          if not self.ser == None:
            r = self.ser.read (1)
            if self.current is not None:
              self.protocol.handle (r)

          time.sleep (0.0001)
        except serial.SerialException as e:
          self.logger.exception ("[Zero] Exception with serial link, reconnecting..: " + str(e))
          self.closeserial ()
          self.openserial ()

        except KeyboardInterrupt as e:
          self.logger.info ("[Zero] Shutting down by local request.")
          self.stop ()
        except:
          self.logger.exception ("[Zero] General exception in inner main loop")
          self.stop ()

    except KeyboardInterrupt as e:
      self.logger.info ("[Zero] Shutting down by local request.")
      self.stop ()

    except:
      self.logger.exception ("[Zero] General exception in main loop")
      self.stop ()

    finally:
      self.logger.info ("[Zero] Main loop finished..")

  def stdin (self):
    # Wait for input on stdin, then exit..

    raw_input ()
    self.stop ()

  def stop_manual (self):
    self.stop (True)

  def stop (self, manual = False):
    if manual:
      self.logger.info ("[Zero] Stopping Zero on request..")
    else:
      self.logger.info ("[Zero] Stopping Zero..")

    self.go = False

    # Stop all Buoys..
    if self.current:
      self.current.deactivate ()
      self.current = None

    for i in self.buoys:
      i.stop ()

    self.logger.info ("[Zero] Stopped.")

if __name__ == '__main__':
  z = Zero ()



