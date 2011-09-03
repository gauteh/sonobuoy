"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

buoy.py: Main Buoy program

"""

from synapse.evalBase import *
from synapse.switchboard import *

from gps import *
from ad  import *

@setHook (HOOK_STARTUP)
def startup ():
  initProtoHw()
  gps_setup ()
  ad_setup ()

def testecho ():
  print "Hello"

statuscount = 0
@setHook (HOOK_1S)
def status (tick):
  ''' Prints a status message to STDOUT every 5 seconds '''
  global statuscount
  statuscount += 1
  if (statuscount == 5):
    gps_status ()
    statuscount = 0

@setHook (HOOK_STDIN)
def stdinHandler (buf):
  gps_stdinhandler (buf)


