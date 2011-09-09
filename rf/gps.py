"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

gps.py: Interface to the LINX RXP-GPS-SR GPS module

NMEA serial data is mapped to STDIN and parsed from there. There
should always be valid GPS data stored in the global variables
below for use when needed.

"""

from synapse import *
from synapse.evalBase import *
from synapse.switchboard import *

# GPS pin configuration
# GPS TX is at GPIO3 (UART0 RX)

# Latest GPS data
valid      = False

longitude  = '' 
north      = False # Longitude reference, north = true, south = false
latitude   = '' 
east       = False # Latitude reference, east = true, west = false 

gps_time   = '' 

# Telegram status
lasttype   = ''
telegramsreceived = ''


def gps_status ():
  global gps_time, telegramsreceived, lasttype, valid, longitude
  global north, latitude, east 
  print "GPS time: ", gps_time, " (T rec: ", telegramsreceived, ")"
  print "Latitude:  ", latitude, ('N' if north else 'S')
  print "Longitude: ", longitude, ('E' if east else 'W') 
  print "Valid: ", valid


