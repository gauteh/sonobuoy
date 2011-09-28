"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

"""

import time
import threading

from util import *

class AD7710:
  buoy = None

  AD_QUEUE_LENGTH = 500.0

  ad_qposition  = 0
  ad_queue_time = 0 # Time to fill up queue
  ad_value      = ''
  ad_config     = ''

  # Receving binary data
  ad_k_remaining    = 0
  ad_k_samples      = 0
  ad_time_of_first  = 0
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
    self.storelock = threading.Lock ()

  ''' Print some AD stats '''
  def ad_status (self):
    # Gets called when an AD status message has been received and interpreted
    #print "[AD] Sample rate: ", (self.AD_QUEUE_LENGTH * 1000 / float(self.ad_queue_time if self.ad_queue_time > 0 else 1)), " [Hz], value: ", self.ad_value, ", Queue postion: ", self.ad_qposition, ", Config: " + self.ad_config
    pass

  def swapstore (self):
    self.store = 1 if (self.store == 0) else 0

  ''' Handle received binary samples '''
  def ad_handle_samples (self):
    #print "[AD] Handling samples.."

    self.nsamples += self.ad_k_samples

    l = len(self.ad_samples)
    if (l != (self.ad_k_samples * 3)):
      print "[AD] Wrong length of binary data."
      return

    # Check checksum
    csum = 0

    s = []
    t = []

    i = 0
    while (i < self.ad_k_samples):
      n  = ord(self.ad_samples[i * 3]) << 8 * 2
      n += ord(self.ad_samples[i * 3 + 1]) << 8
      n += ord(self.ad_samples[i * 3 + 2])

      csum = csum ^ ord(self.ad_samples[i * 3 + 2])
      csum = csum ^ ord(self.ad_samples[i * 3 + 1])
      csum = csum ^ ord(self.ad_samples[i * 3])

      i += 1
      s.append(n)

      #print "[AD] Sample[", i, "] : ", int(n,16)

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
      t.append (n)

    if (hex2 (csum) != self.ad_sample_csum):
      print "[AD] Checksum mismatch: Received binary samples.", hex2(csum), ",", self.ad_sample_csum, ",", l

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

      #print "[AD] Successfully received ", self.ad_k_samples, " samples.. (time of first: " + str(self.ad_time_of_first) + ")"
      #print "[AD] Frequency: " + str(self.freq) + "[Hz]"



