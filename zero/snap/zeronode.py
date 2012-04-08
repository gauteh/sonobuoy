from synapse.evalBase import *
from synapse.switchboard import *

from synapse import *

buoyaddress = ''

@setHook (HOOK_STARTUP)
def startup():
  global buoyaddress
  # 2 Mbps radio rate
  setRadioRate (3)

  # Full power
  txPwr (17)

  # UART1 is connected to USB serial interface (computer)

  # Set up UART1
  initUart (1, 1)
  flowControl (1, False)

  # Relay data from Wireless to UART1
  uniConnect (DS_UART1, DS_TRANSPARENT)

  # Get address info and buoy instructions from UART1 (connected to computer)
  uniConnect (DS_STDIO, DS_UART1)

  # Send buoy instructions from DS_STDIO to DS_TRANSPARENT
  uniConnect (DS_TRANSPARENT, DS_STDIO)

  if buoyaddress != '':
    connect_buoy ()

def connect_buoy ():
  global buoyaddress
  ucastSerial (buoyaddress)


# state, 0 = waiting for start of command
#        1 = waiting for start of z command
#        2 = waiting for z command
state = 0
cmd   = ''
l     = 0
maxl  = 80

@setHook (HOOK_STDIN)
def stdinhnd (buf):
  global cmd, state
  # Listen for address instructions, otherwise relay instructions to buoy
  for c in buf:
    if state == 0:
      if c == '$':
        cmd = c
        l = 1
        state = 1

      # Inside buoy instruction, relay
      else:
        print c

    elif state == 1:
      if c == 'Z':
        state = 2
        cmd = cmd + c
        l   = l + 1

      # Buoy instruction, relay
      else:
        print cmd
        print c
        cmd = ''
        state = 0

    elif state == 2:
      cmd = cmd + c
      l   = l + 1

      # end of instruction, parse
      if c == '*':
        parse (cmd)

      cmd = ''
      state = 0

    # Something wrong with state, reset
    else:
      cmd   = ''
      state = 0
      l     = 0

    # Lost count, reset
    if l > maxl:
      state = 0
      cmd   = ''
      l     = 0


def parse (cmd):
  l = len (cmd)
  i = 0

  t = 0 # token no

  # Commands:
  # $ZA,address*

  while i < l:
    # Find next token
    token = ''
    while cmd[i] != ',' and cmd[i] != '*' and i < l:
      token = token + cmd[i]
      i = i + 1

    # skip delimiter
    i = i + 1

    if t == 0:
      if token != '$ZA':
        return

    elif t == 1:
      # Token contains address of buoy
      buoyaddress = token
      connect_buoy ()

    else:
      return

    t = t + 1



