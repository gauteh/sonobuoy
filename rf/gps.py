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

from util import *

# Latest GPS data
valid      = False # Data valid

longitude  = 0
longituder = '' # Reference E/W
latitude   = 0
latituder  = '' # Reference N/S

satellites = 0
time       = 0

# Token parsing
nmea      = '' 
lastnmea  = '' 
token     = ''
tokeni    = 0 # Token no

''' Set up GPS module and serial connection '''
def gps_setup ():
  initUart (0, 9600, 8, 'N', 1)
  stdinMode (1, False) # Char mode (string length exceeds STDIN limit of 40
                       # chars), no echo

  crossConnect (DS_UART0, DS_STDIO)

''' Handle NMEA data '''
def gps_stdinhandler (buf):
  global token, lastnmea, tokeni
  i = 0
  l = len(buf)

  c = ''

  while (i < l):
    c = buf[i]
    i += 1

    if c == '$' or c == ',':
      do_token ()
      token = ''

      # New line, reset
      if c == '$':
        lastnmea = ''
        tokeni   = 0

    else:
      token += c

    if len(token) > 80:
      lastnmea  = ''
      token     = ''
      tokeni    = 0

def do_token ():
  global lastnmea, token
  global time, satellites
  global valid, latitude, longitude
  global latituder, longituder

  global tokeni

  if tokeni == 0:
    lastnmea = token

  else:
    if lastnmea == 'GPRMC':
      if tokeni == 1:
        time = token
      elif tokeni == 2:
        if (token == 'A'): valid = True
        else: valid = False
      elif tokeni == 3:
        latitude = token
        latituder = ''
      elif tokeni == 4:
        latituder = token
      elif tokeni == 5:
        longitude = token
        longituder = ''
      elif tokeni == 6:
        longituder = token

    elif lastnmea == 'GPGSA':
      if tokeni == 3:
        satellites = int(token)

  tokeni += 1

def gps_status ():
  global time, satellites, lastnmea
  print "GPS time: ", time
  print "Latitude:  ", latitude, latituder
  print "Longitude: ", longitude, longituder
  print "Valid: ", valid
  print "Satellites: ", satellites
  print "Current NMEA: ", lastnmea


