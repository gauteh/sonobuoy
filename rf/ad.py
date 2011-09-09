"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-08-29

ad.py: AD stuff 

"""

# AD channel 0
ad_samplerate = 0
ad_value      = ''


''' Print some AD stats '''
def ad_status ():
  global ad_samplerate, ad_value
  print "[AD] Sample rate: ", ad_samplerate, " [Hz], value: ", ad_value



