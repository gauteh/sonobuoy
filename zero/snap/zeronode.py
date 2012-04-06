from synapse.evalBase import *
from synapse.switchboard import *

from synapse import *

@setHook (HOOK_STARTUP)
def startup():
  # 2 Mbps radio rate
  setRadioRate (3)
  
  # Full power
  txPwr (17)

  # Set up UART1
  initUart (1, 1)
  flowControl (1, False)
  crossConnect (DS_UART1, DS_TRANSPARENT)

  # Receive instructions from UART1 connected to computer
  crossConnect (DS_STDIO, DS_UART1)

@setHook (HOOK_STDIN)
def stdinhnd (buf):
  pass

