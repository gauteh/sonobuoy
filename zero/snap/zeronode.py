from synapse.evalBase    import *
from synapse.switchboard import *

from synapse import *

mode        = 0 # 0 = stdout -> transparent, 1 = stdout -> uart1
buoyaddress = 0

rate_rpc    = 0
noderate    = 0

@setHook(HOOK_STARTUP)
def startup():
  global buoyaddress
  setRadioRate(0)
  
  # Full power
  saveNvParam(70,'\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11')
  txPwr (17)

  # Set up relays
  initUart (1, 1) # Enable UART1, for use in transparent, BPS 1 => 115,000.
  flowControl (1, False)
  uniConnect (DS_UART1, DS_TRANSPARENT)   # Relay data from Wireless to UART1 (zero)
  uniConnect (DS_STDIO, DS_UART1)         # Relay data from UART1 to zeronode (this script)
  uniConnect (DS_TRANSPARENT, DS_STDIO)   # Relay data from zeronode (this script) to UART1
  stdinMode (0, False)
  
  if buoyaddress != '':
    connect_buoy ()

def portal_mode ():
  initUart (1, 38400)
  crossConnect (DS_UART1, DS_PACKET_SERIAL)

def connect_buoy ():
  global buoyaddress
  ucastSerial (buoyaddress)

def reset_buoy ():
  global buoyaddress
  rpc (buoyaddress, 'reset_maple')

@setHook(HOOK_RPC_SENT)
def radiorate_complete (ref):
  global rate_rpc, noderate
  if rate_rpc == ref:
    setRadioRate (noderate)
    rate_rpc = 0
    

def setradiorate_buoy (rate):
  global buoyaddress, rate_rpc, noderate
  rpc (buoyaddress, 'mysetradiorate', int(rate))
  noderate = rate
  rate_rpc = getInfo (9) # bufRef of prev rpc call

# state, 0 = waiting for start of command, '$'
#        1 = recording command, waiting for *
#        2 = waiting for first CS digit
#        3 = waiting for seconds CS digit
state = 0
cmd   = ''
l     = 0
maxl  = 100

@setHook(HOOK_STDIN)
def stdinHandler(buf):
  global cmd, state, l, maxl
  # Listen for address instructions, otherwise relay instructions to buoy
  bl = len(buf)
  i = 0
  while i < bl:
    # Reset
    if l > maxl:
      cmd = ''
      l   = 0
      state = 0
    
    if state == 0:
      if buf[i] == '$':
        state = 1
        cmd = buf[i]
        l   = 1
        
    elif state == 1:
      cmd = cmd + buf[i]
      l   = l + 1
      if buf[i] == '*':
        state = 2
      
      # Reset
      if buf[i] == '$':
        state = 0
        cmd = ''
        l   = 0
    
    elif state == 2:
      cmd = cmd + buf[i]
      l   = l + 1
      state = 3

    elif state == 3:
      cmd = cmd + buf[i]
      l   = l + 1
      
      parse ()
      
      cmd = ''
      l   = 0
      state = 0
    
    i = i + 1


def parse ():
  global cmd, l, state, maxl
  global buoyaddress
  global mode
  i = 0

  t = 0 # token no

  # Commands:
  # $ZA,address*   : Change address of remote node (buoy)
  # $ZS*           : Get status of this node
  # $ZE*           : Get energy level readings
  # $ZP*           : Put into portal mode
  # $ZT*           : Connect stdout to transparent
  # $ZU*           : Connect stdout to uart1
  # $ZC*           : Connect to previously specified address
  # $ZR*           : Reset current buoy
  # $Z0*           : Set local radiorate to 0
  # $Z1*           : Set local radiorate to 1
  # $Z2*           : Set local radiorate to 2
  # $Z3*           : Set local radiorate to 3
  # $ZB0*          : Set current buoy radiorate to 0
  # $ZB1*          : Set current buoy radiorate to 1
  # $ZB2*          : Set current buoy radiorate to 2
  # $ZB3*          : Set current buoy radiorate to 3
  c = -1

  while i < l:
    # Find next token
    token = ''
    while cmd[i] != ',' and cmd[i] != '*' and i < l:
      token = token + cmd[i]
      i = i + 1

    # skip delimiter
    i = i + 1

    if t == 0:
      if token == '$ZA':
        c = 1
      elif token == '$ZT':
        uniConnect (DS_TRANSPARENT, DS_STDIO)
        mode = 0
        return
      elif token == '$ZU':
        uniConnect (DS_UART1, DS_STDIO)
        mode = 1
        return
      elif token == '$ZC':
        ucastSerial (buoyaddress)
        return
      elif token == '$ZE':
        get_energy ()
        return
      elif token == '$ZP':
        portal_mode ()
        return
      elif token == '$ZS':
        print "$Z,S," + buoyaddress + "*NN"
        return
      elif token == '$ZR':
        reset_buoy ()
        return
    
      elif token == '$Z0':
        setRadioRate(0)
        return
      elif token == '$Z1':
        setRadioRate(1)
        return
      elif token == '$Z2':
        setRadioRate(2)
        return
      elif token == '$Z3':
        setRadioRate(3)
        return
    
      elif token == '$ZB0':
        setradiorate_buoy (0)
        return
      elif token == '$ZB1':
        setradiorate_buoy (1)
        return
      elif token == '$ZB2':
        setradiorate_buoy (2)
        return
      elif token == '$ZB3':
        setradiorate_buoy (3)
        return
    
      else:
        # Not zeronode command, relay to buoy
        print cmd
        return
    
    else:
      if c == 1:
        if t == 1:
          # Token contains address of buoy
          ai = find (token, '.')
          if mode == 1:
            print "$DBGZ," + token + " " + str(ai) + "*NN"
          buoyaddress = ''
          aa = int (token[:ai])
          buoyaddress  = chr(aa)
          if mode == 1:
            print "$DBGZ," + str(aa) + "," + str(ai) + "*NN"
          
          token = token[ai+1:]
          ai = find (token, '.')
          aa = int (token[:ai])
          buoyaddress  += chr(aa)
          
          if mode == 1:
            print "$DBGZ," + str(aa) + "," + str(ai) + "*NN"
          
          aa = int (token[ai+1:])
          buoyaddress += chr(aa)
          if mode == 1:
            print "$DBGZ," + str(aa) + "," + str(ai) + "*NN"
            print "$DBGZ," + buoyaddress + "*NN"
                
          return
        else: return
      else: return

    t = t + 1

def find(s, c):
  i = 0
  _l = len(s)
  while i < l:
    if s[i] == c:
      return i
    i = i + 1

  return -1

def get_energy ():
  pass

def setrr (rate):
  setRadioRate(rate)