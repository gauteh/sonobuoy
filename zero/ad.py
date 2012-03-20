"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

"""

import time
import threading
import math

from util import *

class AD:
  buoy = None
  logger = None

  ad_qposition  = 0
  ad_queue_time = 0 # Time to fill up queue
  ad_value      = ''
  ad_config     = ''

  # Receving binary data
  ad_k_remaining    = 0
  ad_k_samples      = 0
  ad_reference      = 0
  ad_reference_status = 0
  ad_sample_csum    = '' # String rep of hex value
  ad_samples        = '' # Array of bytes (3 * byte / value)
  ad_time           = '' # Array of bytes (4 * byte / time stamp)


  # AD storage, swap before storing
  storelock = None
  samplesa = []
  samplesb = []
  store    = 0 # 0 = a, 1 = b
  nsamples = 0
  freq     = 0
  last     = 0


  def __init__ (self, b):
    self.buoy = b
    self.logger = b.logger
    self.storelock = threading.Lock ()

  ''' Print some AD stats '''
  def ad_status (self):
    # Gets called when an AD status message has been received and interpreted
    self.logger.debug ("[AD] Sample rate: " + str((self.ad_k_samples * 1000 / float(self.ad_queue_time if self.ad_queue_time > 0 else 1))) + " [Hz], value: " + str(self.ad_value) + ", Queue postion: " + str(self.ad_qposition) + ", Config: " + self.ad_config)

  def swapstore (self):
    self.store = 1 if (self.store == 0) else 0

  ''' Handle received binary samples '''
  def ad_handle_samples (self):
    self.logger.info ("[AD] Got " + str(self.ad_k_samples) + " samples starting at: " + str(self.ad_reference))

    self.nsamples += self.ad_k_samples

    l = len(self.ad_samples)
    if (l != (self.ad_k_samples * 4)):
      self.logger.error ("[AD] Wrong length of binary data.")
      return

    # Check checksum
    csum = 0

    s = []
    t = []

    i = 0
    while (i < self.ad_k_samples):
      n  = long(ord(self.ad_samples[i * 4 + 3])) << 8 * 3
      n += long(ord(self.ad_samples[i * 4 + 2])) << 8 * 2
      n += long(ord(self.ad_samples[i * 4 + 1])) << 8
      n += long(ord(self.ad_samples[i * 4 + 0]))

      csum = csum ^ ord(self.ad_samples[i * 4 + 3])
      csum = csum ^ ord(self.ad_samples[i * 4 + 2])
      csum = csum ^ ord(self.ad_samples[i * 4 + 1])
      csum = csum ^ ord(self.ad_samples[i * 4])

      i += 1
      s.append(n)

      #print "[AD] Sample[", i, "] : ", hex(n)

    i = 0
    while (i < self.ad_k_samples):
      n  = ord(self.ad_time[i * 4 + 3]) << 8 * 3
      n += ord(self.ad_time[i * 4 + 2]) << 8 * 2
      n += ord(self.ad_time[i * 4 + 1]) << 8
      n += ord(self.ad_time[i * 4 + 0])

      csum = csum ^ ord(self.ad_time[i * 4 + 3])
      csum = csum ^ ord(self.ad_time[i * 4 + 2])
      csum = csum ^ ord(self.ad_time[i * 4 + 1])
      csum = csum ^ ord(self.ad_time[i * 4])

      i += 1
      t.append (int(self.ad_reference * math.pow(10,6)) + n)

    if (hex2 (csum) != self.ad_sample_csum):
      self.logger.error ("[AD] Checksum mismatch in received binary samples (length: " + str(l) + ").")

    else:
      # Successfully received samples and time stamps
      self.storelock.acquire ()
      i = 0
      while i < self.ad_k_samples:
        if self.store == 0:
          self.samplesa.append ((t[i], s[i]))
        else:
          self.samplesb.append ((t[i], s[i]))

        i += 1

      self.storelock.release ()

      if self.buoy.LOG_ON_RECEIVE:
        self.buoy.log ()

      #print "[AD] Successfully received ", self.ad_k_samples, " samples.. (time of first: " + str(self.ad_time_of_first) + ")"
      #print "[AD] Frequency: " + str(self.freq) + "[Hz]"



