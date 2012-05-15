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
  zero = None
  protocol = None

  gps  = None
  ad   = None

  entry   = None  # Configuration entry
  id      = -1    # Unique ID
  name    = ''    # Unique friendly name
  address = ''    # Node address
  address_p = ''  # Synapse format address
  enabled = False

  BASEDIR = 'log'
  logdir  = ''    # Will be BASEDIR/name

  active  = False
  logger  = None

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

    self.filelock = threading.Lock ()

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

  def activate (self):
    self.active = True

  def deactivate (self):
    self.active = False

  def getstatus (self):
    self.protocol.send ("GS")

  def getlatestbatch (self):
    self.getlastid ()

  def getlastid (self):
    self.protocol.send ("GLID")
  
  def getids (self, start):
    self.protocol.send ("GIDS," + str(start))

  error_strings = [ "E_CONFIRM",
                    "E_BADCOMMAND",
                    "E_UNKNOWNCOMMAND",
                    "E_SDUNAVAILABLE",
                    "E_NOSUCHID",
                    "E_NOSUCHREF",
                    "E_NOSUCHSAMPLE",
                    "E_NOSUCHDAT",
                  ]


