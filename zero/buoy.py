# Author: Gaute Hope <eg@gaute.vetsj.com> / 2011-09-26
#
# buoy.py: Represents one Buoy with AD and GPS

import threading
import logging
import time

from ad import *
from gps import *

class Buoy:
  zero = None

  gps  = None
  ad   = None

  node = ''
  logfile = node + '.log'
  logfilef = None

  keeprun = True
  active  = False
  runthread = None
  logger  = None

  LOG_TIME_DELAY = 10

  def __init__ (self, z, n):
    self.zero = z
    self.node = n
    self.logger = self.zero.logger
    self.logger.info ('Starting Buoy ' + self.node + '..')

    self.gps = Gps (self)
    self.ad = AD7710 (self)

    # Open file
    self.logfilef = open (self.logfile, 'a')

    self.name = 'Buoy' + self.node

    # Starting log thread 
    self.runthread = threading.Thread (target = self.run, name = 'Buoy' + self.node )
    self.runthread.start ()

  def log (self):
    self.logger.info ('[' + self.name + '] Writing data file.. (every ' + str(self.LOG_TIME_DELAY) + ' seconds)')
    self.ad.swapstore ()

    # Use inactive store
    v = (self.ad.valuesa if (self.ad.store == 0) else self.ad.valuesb)

    for i in v:
      self.logfilef.write (str(i) + '\n')

    # Clear list
    if self.ad.store == 0:
      self.ad.valuesb = []
    else:
      self.ad.valuesa = []

    self.logfilef.flush ()

  def stop (self):
    self.keeprun = False
    self.log ()
    self.logfilef.close ()


  def activate (self):
    self.active = True

  def deactivate (self):
    self.active = False

  def run (self):
    i = self.LOG_TIME_DELAY # Log on first iteration

    while self.keeprun:
      if self.active:
        if (i >= self.LOG_TIME_DELAY):
          self.log ()
          i = 0

        i += 0.1

      time.sleep (0.1)

