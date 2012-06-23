
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

    # writing gps log
    self.buoy.track (str(self.gps_time) + "," + ("1" if self.valid else "0") + "," + str(self.latitude).strip() + "," + ('N' if self.north else 'S') + ", " + str(self.longitude).strip() + "," + ('E' if self.east else 'W') + "," + ('1' if self.has_time else '0') + "," + ('1' if self.has_sync else '0') + "," + ('1' if self.has_sync_reference else '0'))

