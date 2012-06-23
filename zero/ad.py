# -*- coding: utf-8 -*-
# Author: Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29
#
# ad.py: Handle incoming data batches

import time
import threading
import math

from util import *

from data import *

class AD:
  buoy = None
  logger = None

  ad_batch_length = 1024 # Only for sample rate calculations.. may change
  ad_qposition  = 0
  ad_queue_time = 0 # Time to fill up queue
  ad_value      = ''
  ad_config     = ''

  # Receving binary data
  ad_batch_id       = 0
  ad_refno          = 0
  ad_start          = 0
  ad_k_remaining    = 0
  ad_k_samples      = 0
  AD_K_SAMPLES_MAX  = 10000 # protect from erronous infinite large batches
  ad_reference      = 0
  ad_reference_status = 0
  ad_reference_latitude = 0
  ad_reference_longitude = 0
  ad_sample_csum    = '' # String rep of hex value
  ad_samples        = '' # Array of bytes (3 * byte / value)

  nsamples = 0
  freq     = 0
  last     = 0


  def __init__ (self, b):
    self.buoy = b
    self.logger = b.logger

  ''' Print some AD stats '''
  def ad_status (self):
    # Gets called when an AD status message has been received and interpreted
    self.logger.debug ("[AD] Sample rate: " + str((self.ad_batch_length * 1000 / float(self.ad_queue_time if self.ad_queue_time > 0 else 1))) + " [Hz], value: " + str(self.ad_value) + ", Queue postion: " + str(self.ad_qposition) + ", Config: " + self.ad_config)

    # log to buoy log
    self.buoy.log ("[AD] Sample rate: " + str((self.ad_batch_length * 1000 / float(self.ad_queue_time if self.ad_queue_time > 0 else 1))) + " [Hz], value: " + str(self.ad_value) + ", Queue postion: " + str(self.ad_qposition) + ", Config: " + self.ad_config)

  ''' Handle received binary samples '''
  def ad_handle_samples (self):
    self.logger.debug ("[AD] Got " + str(self.ad_k_samples) + " samples starting at: " + str(self.ad_reference))

    self.nsamples += self.ad_k_samples

    l = len(self.ad_samples)
    if (l != (self.ad_k_samples * 4)):
      self.logger.error ("[AD] Wrong length of binary data.")
      return

    # Check checksum
    csum = 0

    s = []

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

      s.append(n)

      i += 1

      #print "[AD] Sample[", i, "] : ", hex(n)

    # checksum from buoy may only have one digit
    scsum = hex2 (csum)
    if scsum[0] == '0':
      scsum = scsum[1]

    if (scsum != self.ad_sample_csum):
      self.logger.error ("[AD] Checksum mismatch in received binary samples (length: " + str(l) + "): " + hex2(csum) + " != " + self.ad_sample_csum)

    else:
      # Successfully received samples and time stamps
      self.buoy.index.gotbatch (self.ad_batch_id, self.ad_refno, self.ad_start, self.ad_k_samples, self.ad_reference, self.ad_reference_status, self.ad_reference_latitude, self.ad_reference_longitude, s)

      #print "[AD] Successfully received ", self.ad_k_samples, " samples.. (time of first: " + str(self.ad_time_of_first) + ")"
      #print "[AD] Frequency: " + str(self.freq) + "[Hz]"



