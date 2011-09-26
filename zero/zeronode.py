from synapse.evalBase import *
from synapse.switchboard import *

from synapse import *

@setHook (HOOK_STARTUP)
def startup():
  # 2 Mbps
  setRadioRate (3)

n = 0
start = 0

def dobytes(bytes):
  global n
  i = 0
  n += len(bytes)
  if (n > 2000): printstatusb ()

def printstatusb():
  global n, now, start
  now = getMs ()

  s = n / (now - start)
  print "Speed: " + str(s) + " [KBps]"
  start = now
  n = 0

