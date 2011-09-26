import threading
import logging
import time

from ad import *
from gps import *

class Buoy:
  zero = None

  gps  = None
  ad   = None

  node = 'One'
  logfile = node + '.log'
  logfilef = None

  keeprun = True
  runthread = None
  logger  = None

  LOG_TIME_DELAY = 2

  def __init__ (self, z):
    self.zero = z
    self.logger = self.zero.logger
    self.logger.info ('Starting Buoy ' + self.node + '..')

    self.gps = Gps (self)
    self.ad = AD7710 (self)

    # Open file
    self.logfilef = open (self.logfile, 'a')

    self.name = 'Buoy' + self.node

    # Starting logging process
    self.runthread = threading.Thread (target = self.run, name = 'Buoy' + self.node )
    self.runthread.start ()

  def log (self):
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


  def run (self):
    i = 0 
    while self.keeprun:
      if (i >= self.LOG_TIME_DELAY):
        self.logger.info ('[' + self.name + '] Writing data file..')
        self.log ()
        i = 0

      i += 0.1 
      time.sleep (0.1)

