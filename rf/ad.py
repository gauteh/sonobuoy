"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

ad.py: Interface to the AD7710 AD

"""

# AD pin configuration
# SDATA   is at GPIO12 (MOSI)
#                      (MISO is disabled in three-wire mode)
# SCLK    is at GPIO13 (SCLK)
# ~DTRDY  is at GPIO0
# ~RFS    is at GPIO1
# ~TFS    is at GPIO2
# A0      is at GPIO9

from synapse.evalBase import *
nDRDY = GPIO_13
nRFS  = GPIO_0
nTFS  = GPIO_2
A0   = GPIO_9

# AD channel 0
polls     = 0  # Number of polls
ad_0      = '' # String with bytes from AD channel 0
ad_0a     = 0  # Byte 1
ad_0b     = 0  # Byte 3
ad_0c     = 0  # Byte 4
failed    = False
hasworked = False

''' Set up AD '''
def ad_setup ():
  global nDRDY, nRFS, nTFS, A0
  # SPI paramters
  # CPOL = 0
  # CPHA = 1
  # MSB first = 1
  # Three wire
  spiInit (False, True, True, False)

  setPinDir (nDRDY, False) # input
  setPinPullup (nDRDY, True)
  setPinDir (nRFS, True)   # output
  setPinDir (nTFS, True)   # output
  setPinDir (A0, True)     # output

  # Reset
  writePin (A0, True)       # Access data / calibration register
  writePin (nTFS, True)     # Ready for read
  writePin (nRFS, False)    # Ready for read


''' Do sample from AD '''
def ad_get_value ():
  global ad_0, polls, failed, hasworked
  global ad_0a, ad_0b, ad_0c
  global nDRDY, nRFS, nTFS, A0

  failed = False

  # A falling ~DRDY means data is ready
  tries = 1000
  while (readPin(nDRDY) and tries >= 0):
    tries -= 1
    # Sleep?

  if tries < 0:
    failed = True
  else:
    hasworked = True
    # Register is 24 bits / 8 = 3 bytes
    ad_0 = spiRead (3, 8)

    ad_0a = ord(ad_0[0])
    ad_0b = ord(ad_0[1])
    ad_0c = ord(ad_0[2])
    polls += 1

''' Print some AD stats '''
def ad_status ():
  global ad_0, polls, failed, hasworked
  global ad_0a, ad_0b, ad_0c
  print "AD polls: ", polls
  print "Data ready: ", not readPin (nDRDY)
  print "Last value (A0): ", ad_0
  print "Last value (A0): ", ad_0a, ',', ad_0b, ',', ad_0c
  print "Has worked: ", hasworked 



