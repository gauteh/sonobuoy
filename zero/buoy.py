# -*- coding: utf-8 -*-
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2011-09-26
#
# buoy.py: Represents one Buoy with AD and GPS

import threading
import logging
import time
import os

from ad import *
from gps import *

from data  import *
from index import *

class Buoy:
  zero      = None
  protocol  = None

  gps  = None
  ad   = None

  entry   = None  # Configuration entry
  id      = -1    # Unique ID
  name    = ''    # Unique friendly name
  address = ''    # Node address
  address_p = ''  # Synapse format address

  radiorate = 0           # Radio rate currently set
  RADIORATE_TIMEOUT = 60  # seconds before buoy node reset to default (0) rate
                          # is set in snap/buoy.py.
  set_radiorate_t   = 0   # time radio rate was set

  enabled = False

  BASEDIR = 'log'
  logdir  = ''    # Will be BASEDIR/name

  # position track (file)
  track_u = ''
  trackf  = None

  # log (file)
  log_u   = ''
  logf    = None
  t_format = "%Y-%m-%d %H:%M:%S"

  active  = False
  logger  = None

  uptime  = 0
  remote_version = ''
  remote_protocolversion = 1

  def __init__ (self, z, b):
    self.entry  = b
    self.zero   = z
    self.protocol = z.protocol
    self.id     = b['id']
    self.name   = b['name']
    self.address = b['address']
    self.enabled = b['enabled']

    self.address_p = str(int(self.address[:2],16)) + "." + str(int(self.address[3:5],16)) + "." + str(int (self.address[6:8], 16))

    self.logdir = os.path.join (self.BASEDIR, self.name)

    # open track and log files
    self.track_u = os.path.join (self.logdir, 'position_track.txt')
    self.log_u   = os.path.join (self.logdir, self.name + '.log')

    self.trackf = open (self.track_u, 'a')
    self.logf   = open (self.log_u, 'a')

    self.log   ("========== Initialization ==========")
    self.track ("[" + time.strftime(self.t_format) + "] ========== Initialization ==========")

    if not os.path.exists (self.logdir):
      os.makedirs (self.logdir)

    self.logger = self.zero.logger
    self.logger.info ('[' + self.name + '] Initializing Buoy..')

    self.index = Index (self.logger, self)

    self.gps  = Gps (self)
    self.ad   = AD (self)

  def loop (self):
    self.index.loop ()

  def stop (self):
    self.logger.info ("[" + self.name + "] Stopping..")
    self.index.close ()

    if self.trackf is not None:
      self.trackf.close ()
      self.trackf = None

    if self.logf is not None:
      self.logf.close ()
      self.logf = None

  def log (self, s):
    if not self.logf is None:
      self.logf.write ("[" + time.strftime (self.t_format) + "] " + str(s) + "\n")
      self.logf.flush ()

  def track (self, s):
    # input is formatted gps log string
    if not self.trackf is None:
      self.trackf.write (s + '\n')
      self.trackf.flush ()

  def activate (self):
    self.active = True

  def deactivate (self):
    self.active = False

  def getstatus (self):
    self.index.getstatus ()

  def getlatestbatch (self):
    self.getlastid ()

  def getlastid (self):
    self.index.getlastid ()

  error_strings = [ "E_CONFIRM",
                    "E_BADCOMMAND",
                    "E_UNKNOWNCOMMAND",
                    "E_SDUNAVAILABLE",
                    "E_NOSUCHID",
                    "E_NOSUCHREF",
                    "E_NOSUCHSAMPLE",
                    "E_NOSUCHDAT",
                    "E_BADDAT",
                    "E_SDLAG",
                    "E_MAXIDREACHED",
                  ]


