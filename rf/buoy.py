"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

buoy.py: Buoy Control

"""

from synapse.evalBase import *
from synapse.switchboard import *

drdy   = False
ready  = ''
buffer = ''

ARDUINO_UART  = 1
ARDUINO_BAUD  = 38400

@setHook (HOOK_STARTUP)
def startup ():
  initProtoHw ()

  initUart (ARDUINO_UART, ARDUINO_BAUD, 8, 'N', 1)
  stdinMode (1, False)

  crossConnect (DS_UART1, DS_STDIO)

def testecho ():
  print "Hello there."

@setHook (HOOK_1S)
def inputrelay(tick):
  global drdy, ready
  if (drdy):
    print "[Arduino] ", ready
    drdy = False
    ready = ''

@setHook (HOOK_STDIN)
def stdinHandler (buf):
  global buffer, ready, drdy
  i = 0
  l = len(buf)
  while i < l:
    if buf[i] != '$':
      buffer += buf[i]
    else:
      ready  += buffer
      drdy = True
      buffer = ''

    i += 1


