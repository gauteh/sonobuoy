"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

buoy.py: Buoy Control

"""

from synapse.evalBase import *
from synapse.switchboard import *

PORTAL_ADDR = "\x00\x00\x01"
ZERO_ADDR = "\x03\xFF\x37"

RESET_PIN = GPIO_18
RADIORATE_TIMEOUT = 60 # seconds
counter = 0
myrate  = 0 # currently set rate
gorate  = 0 # go to this rate next ms
countdown_gorate = 0

@setHook (HOOK_STARTUP)
def startup ():
  # Full power
  saveNvParam(70,'\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11')
  txPwr(17)
  
  setRadioRate (0)

  # Set up serial and cross connect Buoy to ZeroNode
  initUart (1, 1, 8, 'N', 1)
  flowControl (1, False)

  crossConnect (DS_TRANSPARENT, DS_UART1)
  ucastSerial (ZERO_ADDR)
  
  # Set RESET pin high
  setPinDir (RESET_PIN, True)
  writePin (RESET_PIN, True)
  setPinDir (RESET_PIN, False)

def testecho ():
  print "Hello there."
  rpc (ZERO_ADDR, 'testgot', 'Hey there.')

def reset_maple ():
  # Resetting maple by toggeling reset pin: GPIO18
  setPinDir (RESET_PIN, True)
  writePin (RESET_PIN, False)
  sleep (1, 2) # sleep 2 seconds
  writePin (RESET_PIN, True)
  setPinDir (RESET_PIN, False)

# timeout
@setHook(HOOK_1S)
def countdown_radiorate_reset (tick):
  global counter, RADIORATE_TIMEOUT, myrate
  if myrate != 0:
    if counter > 1:
      counter -= 1
    elif counter == 1:
      # reset
      setRadioRate (0)
      myrate = 0


def mysetradiorate (rate):
  global counter, RADIORATE_TIMEOUT, myrate, gorate, countdown_gorate

  if rate >= 0 and rate <= 3:
      
    counter = RADIORATE_TIMEOUT
    
    if myrate != rate:
      myrate = rate
      setRadioRate (rate)

      # rates (Synapse Reference Manual, p. 153):
      # 0 = 250 kbps (default)
      # 1 = 500 kbps
      # 2 =   1 mbps
      # 3 =   2 mbps
