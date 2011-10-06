"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-09-09

arduino.py: Interface and protocol to Arduino board

"""

from util import *

class Protocol:
  zero = None

  def __init__ (self, z):
    self.zero = z

  # Last type received
  lasttype = ''

  # Receiver state:
  # 0 = Waiting for '$'
  # 1 = Between '$' and '*'
  # 2 = On first CS digit
  # 3 = On second CS digit

  # 4 = Waiting for '$' to signal start of binary data
  # 5 = Receiving AD binary sample data
  # 6 = Receiving AD binary time stamp data

  a_receive_state = 0
  a_buf           = ''

  def handle (self, buf):
    i = 0
    l = len (buf)

    while (i < l):
      c = buf[i]
      i += 1

      if (self.a_receive_state == 0):
        if (c == '$'):
          self.a_buf = '$'
          self.a_receive_state = 1

      elif (self.a_receive_state == 1):
        if (c == '*'):
          self.a_receive_state = 2

        self.a_buf += c

      elif (self.a_receive_state == 2):
        self.a_buf += c
        self.a_receive_state = 3

      elif (self.a_receive_state == 3):
        self.a_buf += c
        self.a_receive_state = 0
        self.a_parse (self.a_buf)
        self.a_buf = ''

      elif (self.a_receive_state == 4):
        if (c == '$'):
          self.a_receive_state = 5

      elif (self.a_receive_state == 5):
        self.zero.current.ad.ad_samples += c
        self.zero.current.ad.ad_k_remaining -= 1
        if (self.zero.current.ad.ad_k_remaining < 1):
          self.zero.current.ad.ad_k_remaining = self.zero.current.ad.ad_k_samples * 4
          self.a_receive_state = 6

      elif (self.a_receive_state == 6):
        self.zero.current.ad.ad_time += c
        self.zero.current.ad.ad_k_remaining -= 1
        if (self.zero.current.ad.ad_k_remaining < 1):
          self.a_receive_state = 0

      else:
        # Something went terribly wrong..
        self.a_buf = ''
        self.a_receive_state = 0

      # Check if we're receiving sane amounts of data..
      if len(self.a_buf) > 80:
        self.a_buf = ''
        self.a_receive_state = 0


  def a_parse (self, buf):
    # Test checksum
    if (not test_checksum (buf)):
      print "[Protocol] Message discarded, checksum failed."
      print buf
      return

    # Parse message
    tokeni = 0
    token  = ''

    msgtype = ''
    subtype = ''

    finished = False

    i = 0
    l = len (buf)

    while (i < l):
      # Get token
      token = ''
      while ((i < l) and (buf[i] != ',' and buf[i] != '*')):
        token += buf[i]
        i += 1

      if ((i < l) and buf[i] == '*'):
        finished = True

      i += 1 # Skip delimiter

      if (tokeni == 0):
        msgtype = token[1:]
      else:
        if (msgtype == 'GPS'):
          if (tokeni == 1): subtype = token
          elif (tokeni > 1):
            if (subtype == 'S'):
              if (tokeni == 2):
                self.zero.current.gps.lasttype = token

              elif (tokeni == 3):
                self.zero.current.gps.telegramsreceived = int(token)

              elif (tokeni == 4):
                self.zero.current.gps.latitude = float(token) if len(token) else 0

              elif (tokeni == 5):
                self.zero.current.gps.north = (token[0] == 'N')

              elif (tokeni == 6):
                self.zero.current.gps.longitude = float(token) if len(token) > 0 else 0

              elif (tokeni == 7):
                self.zero.current.gps.east = (token[0] == 'E')

              elif (tokeni == 8):
                self.zero.current.gps.unix_time = token

              elif (tokeni == 9):
                self.zero.current.gps.gps_time = token

              elif (tokeni == 10):
                self.zero.current.gps.gps_date = token

              elif (tokeni == 11):
                self.zero.current.gps.valid = (token == 'Y')

              elif (tokeni == 12):
                self.zero.current.gps.has_time = (token == 'Y')
              
              elif (tokeni == 13):
                self.zero.current.gps.has_sync = (token == 'Y')

              elif (tokeni == 14):
                self.zero.current.gps.has_sync_reference = (token == 'Y')

                self.zero.current.gps.gps_status ()

        elif (msgtype == 'AD'):
          if (tokeni == 1): subtype = token
          elif (tokeni > 1):
            if (subtype == 'S'):
              if (tokeni == 2):
                self.zero.current.ad.ad_qposition = int(token)
              elif (tokeni == 3):
                self.zero.current.ad.ad_queue_time = int (token)
              elif (tokeni == 4):
                self.zero.current.ad.ad_value = token
              elif (tokeni == 5):
                self.zero.current.ad.ad_config = token
                self.zero.current.ad.ad_status ()
                return

            elif (subtype == 'D'):
              if (tokeni == 2):
                self.zero.current.ad.ad_k_samples = int (token)
                self.zero.current.ad.ad_k_remaining = self.zero.current.ad.ad_k_samples * 3
                self.zero.current.ad.ad_samples = ''
                self.zero.current.ad.ad_time = ''
                self.a_receive_state = 4

              elif (tokeni == 3):
                self.zero.current.ad.ad_time_of_first = int (token)

                #print "[AD] Initiating binary transfer.. samples: ", self.zero.current.ad.ad_k_samples
                return

            elif (subtype == 'DE'):
              if (tokeni == 2):
                #print "[AD] Binay data transfer complete."
                self.zero.current.ad.ad_sample_csum = token
                self.zero.current.ad.ad_handle_samples ()

              return

        elif (msgtype == 'DBG'):
          if (tokeni == 1):
            print "[Debug] ", token

      tokeni += 1

