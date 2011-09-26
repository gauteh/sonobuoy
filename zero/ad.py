"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

"""

from util import *

class AD7710:
  buoy = None

  ad_samplerate = 0
  ad_value      = ''

  # Receving binary data
  ad_k_remaining    = 0
  ad_k_samples      = 0
  ad_time_of_first  = 0
  ad_sample_csum    = '' # String rep of hex value
  ad_samples        = '' # Array of bytes (3 * byte / value)


  # AD storage, swap before storing
  valuesa = []
  valuesb = []
  store   = 0 # 0 = a, 1 = b
  nsamples = 0


  def __init__ (self, b):
    self.buoy = b

  ''' Print some AD stats '''
  def ad_status (self):
    print "[AD] Sample rate: ", self.ad_samplerate, " [Hz], value: ", self.ad_value

  def swapstore (self):
    self.store = 1 if (self.store == 0) else 0

  ''' Handle received binary samples '''
  def ad_handle_samples (self):
    #print "[AD] Handling samples.."

    self.nsamples += self.ad_k_samples

    l = len(self.ad_samples)
    if (l != (self.ad_k_samples * 3)):
      #print "[AD] Wrong length of binary data."
      return

    # Check checksum
    csum = 0

    i = 0
    while (i < self.ad_k_samples):
      n  = ord(self.ad_samples[i * 3]) << 8 * 2
      n += ord(self.ad_samples[i * 3 + 1]) << 8
      n += ord(self.ad_samples[i * 3 + 2])

      csum = csum ^ ord(self.ad_samples[i * 3 + 2])
      csum = csum ^ ord(self.ad_samples[i * 3 + 1])
      csum = csum ^ ord(self.ad_samples[i * 3])

      i += 1
      if self.store == 0:
        self.valuesa.append (n)
      else:
        self.valuesb.append (n)

      #print "[AD] Sample[", i, "] : ", int(n,16)

    if (hex2 (csum) != self.ad_sample_csum):
      print "[AD] Checksum mismatch: Received binary samples.", hex2(csum), ",", self.ad_sample_csum, ",", l
    else:
      print "[AD] Successfully received ", self.ad_k_samples, " samples.."


