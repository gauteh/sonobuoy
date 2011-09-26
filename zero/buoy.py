from ad import *
from gps import *

class Buoy:
  zero = None

  gps  = None
  ad   = None

  name = 'One'

  def __init__ (self, z):
    self.zero = z

    self.gps = Gps (self)
    self.ad = AD7710 (self)

  def log (self):
    self.ad.swapstore ()

    # Use inactive store
    if self.ad.store == 0:
      v = self.ad.valuesa
    else:
      v = self.ad.valuesb

    l = open (self.name + '.log', 'a')
    for i in v:
      l.write (str(i) + '\n')

    l.close ()

    

