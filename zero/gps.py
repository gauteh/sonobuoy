
class Gps ():
  buoy = None


  lasttype = ''         # Last type received
  telegramsreceived = 0
  latitude = ''
  longitude = ''
  north = False
  east  = False
  gps_time = ''
  valid = False

  def __init__ (self, b):
    self.buoy = b

  def gps_status (self):
    print "GPS time: ", self.gps_time, " (T rec: ", self.telegramsreceived, ")"
    print "Latitude:  ", self.latitude, ('N' if self.north else 'S')
    print "Longitude: ", self.longitude, ('E' if self.east else 'W') 
    print "Valid: ", self.valid
