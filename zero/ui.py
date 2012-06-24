# Author: Gaute Hope <eg@gaute.vetsj.com> / 2011-09-28
#
# API to zero service.

from multiprocessing.managers import BaseManager, BaseProxy
from zero import *
from buoy import *
from ad   import *

from datetime import datetime

''' Provides queryable methods for zCLI '''
class ZeroCliService:
  zero = None

  def __init__(self):
    pass

  def getstatus (self):
    self.zero.current.getstatus ()

  def resetbuoy (self):
    self.zero.protocol.resetbuoy ()

  def getlatestbatch (self):
    self.zero.current.getlatestbatch ()

  def getids (self, start):
    self.zero.current.getids (start)

  def startacquire (self):
    self.zero.startacquire ()

  def stopacquire (self):
    self.zero.stopacquire ()

  def bouy_count (self):
    return len(self.zero.buoys)

  def buoy_statuses (self):
    s = []
    for b in self.zero.buoys:
      s.append (self.buoy_status (b))

    return s

  def buoy_status_by_name (self, bname):
    for b in self.zero.buoys:
      if b.name == bname:
        return self.buoy_status (b)

    return None

  def buoy_status (self, b):
    #return [b.active, b.name, b.ad.ad_value, b.ad.ad_config, b.ad.ad_qposition, b.ad.ad_queue_time, b.ad.nsamples, b.gps.latitude, b.gps.north, b.gps.longitude, b.gps.east, b.gps.valid, b.gps.gps_time, b.gps.unix_time, b.gps.gps_date, b.gps.has_time, b.gps.has_sync, b.gps.has_sync_reference, b.ad.ad_batch_length]
    return {'active'       : b.active,
            'name'         : b.name,
            'id'           : b.id,
            'ad_value'     : b.ad.ad_value,
            'ad_config'    : b.ad.ad_config,
            'ad_qposition' : b.ad.ad_qposition,
            'ad_queue_time' : b.ad.ad_queue_time,
            'ad_nsamples'   : b.ad.nsamples,
            'latitude'      : b.gps.latitude,
            'north'         : b.gps.north,
            'longitude'     : b.gps.longitude,
            'east'          : b.gps.east,
            'valid'         : b.gps.valid,
            'time'          : b.gps.gps_time,
            'unix_time'     : b.gps.unix_time,
            'datetime'      : datetime.fromtimestamp (b.gps.unix_time),
            'date'          : b.gps.gps_date,
            'has_time'      : b.gps.has_time,
            'has_sync'      : b.gps.has_sync,
            'has_sync_reference' : b.gps.has_sync_reference,
            'batch_length'  : b.ad.ad_batch_length,
            }


class ZeroUIManager (BaseManager):
  AUTHKEY = u'sdfaf2faeoidfasdfiasdufoasdiyfa'

  zcliservice = None

  def __init__ (self):
    BaseManager.__init__ (self, address = ('127.0.0.1', 50002), authkey = self.AUTHKEY)

  def setup_server (self, z):
    self.zcliservice = ZeroCliService ()
    self.zcliservice.zero = z

    self.register ('get_zcliservice', callable = lambda: self.zcliservice)
    self.register ('stop', z.stop_manual)
    self.register ('znportalmode', z.protocol.znportalmode)
    self.register ('zngetstatus', z.protocol.zngetstatus)
    self.register ('znconnect', z.protocol.znconnect)
    self.register ('znsetaddress', z.protocol.znsetaddress)
    self.register ('znoutputuart', z.protocol.znoutputuart)
    self.register ('znoutputwireless', z.protocol.znoutputwireless)

  def setup_client (self):
    self.register ('get_zcliservice')
    self.register ('stop')
    self.register ('znportalmode')
    self.register ('zngetstatus')
    self.register ('znconnect')
    self.register ('znsetaddress')
    self.register ('znoutputuart')
    self.register ('znoutputwireless')

