
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
