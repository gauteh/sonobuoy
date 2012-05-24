"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

buoy.py: Buoy Control

"""

from synapse.evalBase import *
from synapse.switchboard import *

PORTAL_ADDR = "\x00\x00\x01"
ZERO_ADDR = "\x03\xFF\x37"

RESET_PIN = GPIO_18

@setHook (HOOK_STARTUP)
def startup ():
  initProtoHw ()

  # 2Mbps
  setRadioRate(3)
  
  # Full power
  saveNvParam(70,'\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11')
  txPwr(17)

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
