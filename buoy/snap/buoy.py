"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

buoy.py: Buoy Control

"""

from synapse.evalBase import *
from synapse.switchboard import *

PORTAL_ADDR = "\x00\x00\x01"
ZERO_ADDR = "\x03\xFF\x37"

@setHook (HOOK_STARTUP)
def startup ():
  initProtoHw ()

  # 2Mbps
  setRadioRate(3)
  
  # Full power
  txPwr(17)

  # Set up serial and cross connect Buoy to ZeroNode
  initUart (1, 1, 8, 'N', 1)
  flowControl (1, False)

  crossConnect (DS_TRANSPARENT, DS_UART1)
  ucastSerial (ZERO_ADDR)

def testecho ():
  print "Hello there."
  rpc (ZERO_ADDR, 'testgot', 'Hey there.')

