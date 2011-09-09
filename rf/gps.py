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
checkvalid = False  # Checksum valid
datavalid  = False  # Data valid
valid      = False  # GPS fields are all valid (checkvalid & datavalid)
                    # _Must_ be checked before using GPS data

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
  global token, lastnmea, tokeni, nmea
  i = 0
  l = len(buf)

  c = ''

  while (i < l):
    c = buf[i]
    i += 1

    if c == ',' or c == '*':
      do_token ()
      token = ''

    # New line, reset
    elif c == '$':
      # Token contains check sum, test
      check_nmea_sum ()

      token    = ''
      lastnmea = ''
      tokeni   = 0

    else:
      token += c

    # Error:
    if len(token) > 80:
      gps_fields_reset ()

''' Return hexdecimal string representation of integer, result in 2 digits '''
def hex (i):
  h = '0123456789ABCDEF'
  return h[i >> 4] + h[i & 0x0f]

''' Checks NMEA if it matches checksum '''
def check_nmea_sum ():
  global nmea, token, checkvalid, checkednmea, valid, datavalid

  if token[0:2] == hex(gen_check_sum(nmea)):
    checkvalid = True
    valid = (checkvalid and datavalid)
  else:
    checkvalid = False
    gps_fields_reset ()

''' Generate check sum for string, XOR of all characters '''
def gen_check_sum (src):
  i = 0
  l = len(src)
  sum = 0
  while i < l:
    sum = sum ^ ord(src[i])
    i += 1

  return sum

def gps_fields_reset ():
  global nmea, lastnmea, tokeni, token, valid, datavalid
  nmea      = ''
  lastnmea  = ''
  token     = ''
  tokeni    = 0
  valid     = False
  datavalid = False

def do_token ():
  global lastnmea, token, nmea
  global time, satellites
  global datavalid, latitude, longitude
  global latituder, longituder

  global tokeni

  if tokeni == 0:
    lastnmea = token
    nmea = token

  else:
    nmea += ','
    nmea += token

    if lastnmea == 'GPRMC':
      if tokeni == 1:
        time = token

      elif tokeni == 2:
        if token == 'A': datavalid = True
        else: datavalid = False

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

    elif lastnmea == 'GPGGA':
      if tokeni == 1:
        time = token

    elif lastnmea == 'GPGLL':
      if tokeni == 5:
        time = token

    elif lastnmea == 'GPGSA':
      if tokeni == 3:
        satellites = int(token)

  tokeni += 1

def gps_status ():
  global time, satellites, lastnmea
  print "GPS time: ", time, " (Sats: ", satellites, ")"
  print "Latitude:  ", latitude, latituder
  print "Longitude: ", longitude, longituder
  print "Valid: ", valid


