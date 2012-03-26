# Author: Gaute Hope <eg@gaute.vetsj.com> / 2011-09-26
#
# buoy.py: Represents one Buoy with AD and GPS

import threading
import logging
import time
import os

from ad import *
from gps import *

class Buoy:
  zero = None

  gps  = None
  ad   = None

  id      = -1    # Unique ID
  name    = ''    # Unique friendly name
  BASEDIR = 'log'
  logdir  = ''    # Will be BASEDIR/name
  logfile = ''
  logfilef = None
  filelock = None

  keeprun = True
  active  = False
  runthread = None
  logger  = None

  LOG_TIME_DELAY = 15
  LOG_ON_RECEIVE = True # write data when batch of samples have been received

  def __init__ (self, z, id, n):
    self.zero   = z
    self.id     = id
    self.name   = n
    self.logdir = os.path.join (self.BASEDIR, self.name)

    self.filelock = threading.Lock ()

    if not os.path.exists (self.logdir):
      os.makedirs (self.logdir)

    self.logfile  = os.path.join (self.logdir, self.name + '.dtt')
    self.logger   = self.zero.logger

    self.logger.info ('[' + self.name + '] Initializing Buoy..')

    self.gps  = Gps (self)
    self.ad   = AD (self)

    self.rollfile ()

    # Starting log thread
    if not self.LOG_ON_RECEIVE:
      self.runthread = threading.Thread (target = self.run, name = 'Buoy' + self.name)
      self.runthread.start ()
      self.logger.info ("[" + self.name + "] Writing data file every " + str(self.LOG_TIME_DELAY) + " seconds.")

  def rollfile (self):
    self.filelock.acquire ()
    self.logger.info ("[" + self.name +"] Rolling data files..")
    if self.logfilef != None:
      self.logfilef.close ()

    # Find next file
    l = os.path.join (self.logdir, self.name + '.dtt')
    lnext = os.path.join (self.logdir, self.name + '.1.dtt')

    if os.path.exists (l):
      if os.path.exists (lnext):
        # move files backwards (recursively)
        self.__rollfilebackwards__ (1)

      os.rename (l, lnext)

    self.logfile = l

    # Open file
    self.logfilef = open (self.logfile, 'a')
    self.filelock.release()

  def __rollfilebackwards__ (self, i):
    l = os.path.join (self.logdir, self.name + '.' + str(i) + '.dtt')
    lnext = os.path.join (self.logdir, self.name + '.' + str(i+1) + '.dtt')

    # Recursively move files backwards, previous first.
    if (os.path.exists (lnext)):
      self.__rollfilebackwards__ (i+1)

    os.rename (l, lnext)


  def log (self):
    self.filelock.acquire ()
    self.logger.debug ('[' + self.name + '] Writing data file..')

    # Acquire lock and swap stores
    self.ad.storelock.acquire ()
    self.ad.swapstore ()
    self.ad.storelock.release ()

    # Data format:
    # Reference is written as:
    # R,[length],[reference],[reference_status] (soon to contain GPS information)
    # Samples are put on at the line in twos complement, there should be
    # the same number as in length, but confirm when reading.

    # Write data from inactive store
    l = len(self.ad.samplesb) if (self.ad.store == 0) else len(self.ad.samplesa)
    rl = len(self.ad.referencesb) if (self.ad.store == 0) else len(self.ad.referencesa)
    i = 0
    r = 0

    while i < l:
      if self.ad.store == 0:
        if (r < rl and i == self.ad.referencesb[r][1]):
          self.logfilef.write (self.ad.referencesb[r][0] + '\n')
          r += 1

        self.logfilef.write (str(self.ad.samplesb[i]) + '\n')

      else:
        if (r < rl and i == self.ad.referencesa[r][1]):
          self.logfilef.write (self.ad.referencesa[r][0] + '\n')
          r += 1

        self.logfilef.write (str(self.ad.samplesa[i]) + '\n')

      i += 1

    # Clear inactive store
    if self.ad.store == 0:
      self.ad.samplesb = []
      self.ad.referencesb = []
    else:
      self.ad.samplesa = []
      self.ad.referencesa = []

    self.logfilef.flush ()
    self.filelock.release ()

  def stop (self):
    self.logger.info ("[" + self.name + "] Stopping..")
    self.filelock.acquire ()
    self.keeprun = False

    if not self.LOG_ON_RECEIVE:
      self.runthread.join ()

    self.logfilef.close ()
    self.filelock.release ()

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

    self.logger.info ("[" + self.name + "] Stopped.")

