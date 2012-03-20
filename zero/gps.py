
class Gps ():
  buoy = None
  logger = None


  lasttype = ''         # Last type received
  telegramsreceived = 0
  latitude = 0
  longitude = 0
  north = False
  east  = False
  unix_time = 0
  gps_time = ''
  gps_date = ''
  valid = False
  has_time = False
  has_sync = False
  has_sync_reference = False

  def __init__ (self, b):
    self.buoy = b
    self.logger = b.logger

  def gps_status (self):
    self.logger.debug ("[GPS] [" + ("valid" if self.valid else "invalid") + "] Time: " + str(self.gps_time) + " [" + str(self.unix_time) + "] Position: " + str(self.latitude) + ('N' if self.north else 'S') +  ", " + str(self.longitude) + ('E' if self.east else 'W'))

