"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

ad.py: AD stuff

"""

from arduino import *

# AD channel 0
ad_samplerate = 0
ad_value      = ''

# Receving binary data
ad_k_remaining    = 0
ad_k_samples      = 0
ad_time_of_first  = 0
ad_sample_csum    = '' # String rep of hex value
ad_samples        = '' # Array of bytes (4 * byte / value)

''' Print some AD stats '''
def ad_status ():
  global ad_samplerate, ad_value
  print "[AD] Sample rate: ", ad_samplerate, " [Hz], value: ", ad_value


''' Handle received binary samples '''
def ad_handle_samples ():
  global ad_samples, ad_k_samples, ad_time_of_first, ad_sample_csum
  print "[AD] Handling samples.."

  l = len(ad_samples)
  if (l != (ad_k_samples * 4)):
    print "[AD] Wrong length of binary data."
    return

  # Check checksum
  csum = 0

  i = 0
  while (i < ad_k_samples):
    n =  hex(ord(ad_samples[i * 4 + 3]))
    n += hex(ord(ad_samples[i * 4 + 2]))
    n += hex(ord(ad_samples[i * 4 + 1]))
    n += hex(ord(ad_samples[i * 4]))

    csum = csum ^ ord(ad_samples[i * 4 + 3])
    csum = csum ^ ord(ad_samples[i * 4 + 2])
    csum = csum ^ ord(ad_samples[i * 4 + 1])
    csum = csum ^ ord(ad_samples[i * 4])

    i += 1
    #print "[AD] Sample[", i, "] : ", n

  if (hex (csum) != ad_sample_csum):
    print "[AD] Checksum mismatch: Received binary samples.", hex(csum), ",", ad_sample_csum, ",", l
  else:
    print "[AD] Successfully received ", ad_k_samples, " samples.."


