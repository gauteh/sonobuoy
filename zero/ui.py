from multiprocessing.managers import BaseManager, BaseProxy
from zero import *
from buoy import *
from ad import *

''' Provides queryable methods for zCLI '''
class ZeroCliService:
  zero = None

  def __init__(self, z):
    self.zero = z

  def bouy_count (self):
    return len(self.zero.buoys)

  def ad_queue_length (self):
    return AD7710.AD_QUEUE_LENGTH

  def stop (self):
    time.sleep (1)
    self.zero.stop ()

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
    return [b.active, b.name, b.ad.ad_value, b.ad.ad_config, b.ad.ad_qposition, b.ad.ad_queue_time, b.ad.nsamples, b.gps.latitude, b.gps.north, b.gps.longitude, b.gps.east, b.gps.valid, b.gps.gps_time, b.gps.unix_time, b.gps.gps_date, b.gps.has_time, b.gps.has_sync, b.gps.has_sync_reference]


class ZeroUIManager (BaseManager):
  AUTHKEY = u'sdfaf2faeoidfasdfiasdufoasdiyfa'

  def __init__ (self):
    BaseManager.__init__ (self, address = ('127.0.0.1', 50001), authkey = self.AUTHKEY)

  def setup_server (self, z):
    self.zcliservice = ZeroCliService (z)
    self.register ('get_zcliservice', callable = lambda: self.zcliservice)

  def setup_client (self):
    self.register ('get_zcliservice')

