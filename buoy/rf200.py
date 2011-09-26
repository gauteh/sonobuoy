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

#  a_setup ()

def testecho ():
  print "Hello there."

#@setHook (HOOK_STDIN)
def stdinHandler (buf):
  a_stdinhandler (buf)

waitrpc = False
doperf  = False

packs   = 100
remain  = 0

@setHook (HOOK_RPC_SENT)
def rpc_sent (bufid):
  global waitrpc
  waitrpc = False

@setHook (HOOK_1MS)
def onems (tick):
  global waitrpc
  global s
  global doperf, remain 
  if doperf:
    if not waitrpc:
      waitrpc = rpc (PORTAL_ADDR, 'dobytes', s)
      remain -= 1
      if remain < 1: doperf = False

def startperf ():
  global doperf, packs, remain
  doperf = True
  remain = packs 

def whileperf():
  global remain, waitrpc, s
  while remain > 1:
    while waitrpc: pass
    waitrpc = rpc (PORTAL_ADDR, 'dobytes', s)
    remain -= 1

def stopperf ():
  global doperf
  doperf = False

def setpacks (p):
  global packs
  packs = p

