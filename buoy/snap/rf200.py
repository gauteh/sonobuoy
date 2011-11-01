"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

buoy.py: Buoy Control

"""

from synapse.evalBase import *
from synapse.switchboard import *

from arduino import *

#PORTAL_ADDR = "\x00\x00\x01"
PORTAL_ADDR = "\x03\xFF\x37"
s = ''

@setHook (HOOK_STARTUP)
def startup ():
  global s
  initProtoHw ()

  i = 0
  while i < 87:
    i += 1
    s += chr(ord('0') + i)

  # 2Mbps
  setRadioRate(3)

def testecho ():
  print "Hello there."

