#! /usr/bin/python2
#
# Author:  Gaute Hope <eg@gaute.vetsj.com>
# Date:    2012-04-28
#
# Reads /dev/maple persistently, allows programming without exiting.

import serial
import time, os, sys
import logging, multiprocessing

class ReadMaple:
  logger = multiprocessing.log_to_stderr ()
  logger.setLevel (logging.DEBUG)
  run = True

  port = '/dev/maple'
  baud = 115200

  ser = None

  def openserial (self):
    self.logger.info ("Opening serial port " + str(self.port) + "..")
    while ((self.ser == None or not self.ser.isOpen ()) and self.run):
      try:
        try:
          self.ser = serial.Serial (self.port, self.baud)
          self.logger.info ("Serial port open.")
        except serial.SerialException as e:
          #self.logger.error ("Failed to open serial port.. retrying in 1 seconds.")
          self.ser = None
          time.sleep (1)

      except:
        self.stop ()

  def closeserial (self):
    try:
      if self.ser != None:
        self.ser.close ()
    except: pass

    self.ser = None

  def stop (self):
    self.run = False

  def run (self):
    multiprocessing.process.current_process ().name = 'Maple'

    TIMESTAMPINT = 1*60
    lasttime = 0
    # Log to file aswell
    if len(sys.argv) > 1:
      pass


    try:
      self.logger.info ("Entering main loop..")
      self.openserial ()

      while self.run:
        try:
          if not self.ser == None:
            if time.time () - lasttime > TIMESTAMPINT:
              lasttime = time.time ()
              sys.stdout.write ('======= ' + time.asctime () + ' =======\n')
            r = self.ser.readline ()
            sys.stdout.write (r)

        except serial.SerialException as e:
          #self.logger.info ("Exception with serial link, reconnecting..: " + str(e))
          self.closeserial ()
          self.openserial ()

        except KeyboardInterrupt as e:
          self.logger.info ("Shutting down (keyboard).")
          self.stop ()
        except:
          self.logger.exception ("General exception in inner main loop")
          self.stop ()

    except KeyboardInterrupt as e:
      self.logger.info ("Shutting down by local request.")
      self.stop ()

    except:
      self.logger.exception ("General exception in main loop")
      self.stop ()

    finally:
      self.logger.info ("Main loop finished..")


## Main loop
if __name__ == '__main__':
  r = ReadMaple ()
  r.run ()

