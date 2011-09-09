"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

buoy.py: Buoy Control

"""

from synapse.evalBase import *
from synapse.switchboard import *

from arduino import *

@setHook (HOOK_STARTUP)
def startup ():
  initProtoHw ()

  a_setup ()

def testecho ():
  print "Hello there."

@setHook (HOOK_STDIN)
def stdinHandler (buf):
  a_stdinhandler (buf)


