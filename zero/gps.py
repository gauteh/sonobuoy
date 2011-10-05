
class Gps ():
  buoy = None


  lasttype = ''         # Last type received
  telegramsreceived = 0
  latitude = ''
  longitude = ''
  north = False
  east  = False
  unix_time = ''
  gps_time = ''
  gps_date = ''
  valid = False
  has_time = False
  has_sync = False

  def __init__ (self, b):
    self.buoy = b

  def gps_status (self):
    print "[GPS] [", ("valid" if self.valid else "invalid"), "] Time: ", self.gps_time, "[", self.unix_time, "]", " Position: ", self.latitude, ('N' if self.north else 'S'),  ", ", self.longitude, ('E' if self.east else 'W') 
