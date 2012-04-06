"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

buoy.py: Buoy Control

"""

from synapse.evalBase import *
from synapse.switchboard import *

PORTAL_ADDR = "\x00\x00\x01"
ZERO_ADDR = "\x03\xFF\x37"

ARDUINO_UART  = 1
ARDUINO_BAUD  = 1 # 1 = 115200 

@setHook (HOOK_STARTUP)
def startup ():
  initProtoHw ()

  # 2Mbps
  setRadioRate(3)
  
  # Full power
  txPwr(17)

  # Set up serial and cross connect Arduino to ZeroNode
  initUart (ARDUINO_UART, ARDUINO_BAUD, 8, 'N', 1)
  flowControl (1, False)

  crossConnect (DS_TRANSPARENT, DS_UART1)
  ucastSerial (ZERO_ADDR)

def testecho ():
  print "Hello there."
  rpc (ZERO_ADDR, 'testgot', 'Hey there.')

