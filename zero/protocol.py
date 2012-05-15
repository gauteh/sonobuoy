"""
Gaute Hope <eg@gaute.vetsj.com> (c) 2011-09-09

protocol.py: Interface and protocol to buoys

"""

from time import time

from util import *
from buoy import *

class Protocol:
  zero    = None
  logger  = None

  adressedbuoy = 0     # id of buoy addressed on zeronode

  tokens = [0, 0, 0, 0, 0, 0, 0, 0] # temp for storing tokens during parsing

  def __init__ (self, z):
    self.zero     = z
    self.logger   = z.logger

# Commands (buoy and zeronode) and ouput {{{
  def znsetaddress (self):
    # Address buoy
    self.logger.info ("[ZeroNode] Setting to address: " + str(self.zero.current.address_p))
    _msg = 'ZA,' + self.zero.current.address_p
    self.zero.send ('$' + _msg + '*' + gen_checksum (_msg))
    self.adressedbuoy = self.zero.current.id

  # Request status from zeronode
  def zngetstatus (self):
    self.zero.send ('$ZS*' + gen_checksum ('ZS'))

  def znconnect (self):
    self.logger.info ("[ZeroNode] Connecting to current buoy..")
    self.zero.send ('$ZC*' + gen_checksum ('ZA'))

  def znportalmode (self):
    # Put into portal mode and exit
    self.logger.info ("[ZeroNode] Going into portal mode..")
    self.zero.send ("$ZP*" + gen_checksum ('ZP'))

  def znoutputuart (self):
    self.logger.info ("[ZeroNode] Setting output on RF to UART (computer)")
    self.zero.send ("$ZU*" + gen_checksum ('ZU'))

  def znoutputwireless (self):
    self.logger.info ("[ZeroNode] Setting output on RF to wireless (remote buoy)")
    self.zero.send ("$ZT*" + gen_checksum ('ZT'))

  def send (self, msg):
    if self.zero.current.id != self.adressedbuoy:
      self.znsetaddress ()
      self.znconnect ()
      self.znoutputwireless ()

    # Encapsulate and add checksum
    msg = '$' + msg + '*' + gen_checksum (msg)
    self.zero.send (msg)
# }}}

# Parser and handler {{{
  # Last type received
  lasttype = ''

  # Receiver state:
  # 0 = Waiting for '$'
  # 1 = Between '$' and '*'
  # 2 = On first CS digit
  # 3 = On second CS digit
  # 4 = Waiting for '$' to signal start of binary data
  # 5 = Receiving AD binary sample data

  a_receive_state = 0
  a_buf           = ''
  waitforreceipt  = False # Have just got a AD data batch and is waiting for
                          # a DE receipt message
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
          self.a_receive_state = 0
          self.waitforreceipt = True

      else:
        # Something went terribly wrong..
        self.a_buf = ''
        self.a_receive_state = 0
        self.waitforreceipt = False
        self.zero.current.index.reset ()

      # Check if we're receiving sane amounts of data..
      if len(self.a_buf) > 80:
        self.a_buf = ''
        self.a_receive_state = 0


  def a_parse (self, buf): # {{{
    # Test checksum
    if (buf[-2:] == 'NN'):
      #self.logger.debug ("[Protocol] Checksum not provided on received message")
      pass
    elif (not test_checksum (buf)):
      self.logger.info ("[Protocol] Message discarded, checksum failed.")
      self.logger.info ("[Protocol] Discarded: " + buf)
      self.zero.current.index.reset ()
      return

    # Parse message
    tokeni = 0
    token  = ''

    msgtype = ''
    subtype = ''

    i = 0
    l = len (buf)

    while (i < l):

      # Skip checksum
      if buf[i-1] == '*':
        return

      # Get token
      token = ''
      while ((i < l) and (buf[i] != ',' and buf[i] != '*')):
        token += buf[i]
        i += 1

      i += 1 # Skip delimiter

      if (tokeni == 0):
        msgtype = token[1:]
      else:
        if self.waitforreceipt:
          if msgtype != 'AD' or (tokeni > 1 and subtype != 'DE'):
            self.zero.current.index.reset ()
            self.logger.error ("[Protocol] Did not receive receipt immediately after data batch. Discarding data batch.")
            self.waitforreceipt = False
            self.zero.current.ad.ad_k_samples = 0
            self.zero.current.ad.ad_reference = 0


        if (msgtype == 'GPS'):
          if (tokeni == 1): subtype = token
          elif (tokeni > 1):
            if (subtype == 'S'):
              if (tokeni == 2):
                self.zero.current.gps.lasttype = token

              elif (tokeni == 3):
                try:
                  self.zero.current.gps.telegramsreceived = int(token)
                except ValueError:
                  self.zero.current.index.reset ()
                  self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                  return

              elif (tokeni == 4):
                try:
                  self.zero.current.gps.latitude = float(token) if len(token) else 0
                except ValueError:
                  self.zero.current.index.reset ()
                  self.logger.exception ("[Protocol] Could not convert token to float. Discarding rest of message.")
                  return

              elif (tokeni == 5):
                self.zero.current.gps.north = (token[0] == 'N')

              elif (tokeni == 6):
                try:
                  self.zero.current.gps.longitude = float(token) if len(token) > 0 else 0
                except ValueError:
                  self.zero.current.index.reset ()
                  self.logger.exception ("[Protocol] Could not convert token to float. Discarding rest of message.")
                  return

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
                self.zero.current.index.gotstatus ()
              else:
                self.zero.current.index.reset ()
                self.logger.error ("[Protocol] Too many tokens for message: " + msgtype + ", subtype: " + subtype + ", token: " + token)
                return

            else:
              self.zero.current.index.reset ()
              self.logger.error ("[Protocol] Unknown subtype for message: " + str(buf))
              return

        elif (msgtype == 'AD'):
          if (tokeni == 1): subtype = token
          elif (tokeni > 1):
            if (subtype == 'S'):
              try:
                if (tokeni == 2):
                  self.zero.current.ad.ad_qposition = int(token)
                elif (tokeni == 3):
                  self.zero.current.ad.ad_queue_time = int (token)
                elif (tokeni == 4):
                  self.zero.current.ad.ad_value = token
                elif (tokeni == 5):
                  self.zero.current.ad.ad_config = token
                  self.zero.current.ad.ad_status ()
                  self.zero.current.index.gotstatus ()
                  return
                else:
                  self.zero.current.index.reset ()
                  self.logger.error ("[Protocol] Too many tokens for message: " + msgtype + ", subtype: " + subtype)
                  return
              except ValueError:
                self.zero.current.index.reset ()
                self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                return

            elif (subtype == 'D'):
              if (tokeni == 2):
                try:
                  self.zero.current.ad.ad_k_samples = int (token)
                except ValueError:
                  self.zero.current.index.reset ()
                  self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                  return

                if (self.zero.current.ad.ad_k_samples > self.zero.current.ad.AD_K_SAMPLES_MAX):
                  self.logger.error ("[Protocol] Too large batch, resetting protocol.")
                  self.zero.current.index.reset ()
                  self.a_receive_state = 0
                else:

                  self.zero.current.ad.ad_k_remaining = self.zero.current.ad.ad_k_samples * 4
                  self.zero.current.ad.ad_samples = ''
                  self.a_receive_state = 4

              elif (tokeni == 3):
                try:
                  self.zero.current.ad.ad_reference = int (token)
                except ValueError:
                  self.zero.current.index.reset ()
                  self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                  return

              elif (tokeni == 4):
                try:
                  self.zero.current.ad.ad_reference_status = int (token)
                except ValueError:
                  self.zero.current.index.reset ()
                  self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                  return

                #print "[AD] Initiating binary transfer.. samples: ", self.zero.current.ad.ad_k_samples
                return

              else:
                self.zero.current.index.reset ()
                self.logger.error ("[Protocol] Too many tokens for message: " + msgtype + ", subtype: " + subtype)
                return

            elif (subtype == 'DE'):
              if not self.waitforreceipt:
                self.zero.current.index.reset ()
                self.logger.error ("[Protocol] Got end of batch data without getting data first.")
                self.zero.current.ad.ad_k_samples = 0
                self.zero.current.ad.ad_reference = 0
                self.waitforreceipt = False
                return

              if (tokeni == 2):
                #print "[AD] Binay data transfer complete."
                self.zero.current.ad.ad_sample_csum = token
                self.zero.current.ad.ad_handle_samples ()
                self.waitforreceipt = False

              return

            else:
              self.zero.current.index.reset ()
              self.logger.error ("[Protocol] Unknown subtype for message: " + str(buf))
              return

        elif (msgtype == 'Z'):
          if (tokeni == 1):
            subtype = token
          else:
            if subtype == 'S':
              self.znaddress = token
              try:
                if len(token) == 3:
                  self.logger.info ("[ZeroNode] Current node address: " + hex2(ord(token[0])) + ":" + hex2(ord(token[1])) + ":" + hex2(ord(token[2])))
                else:
                  self.logger.info ("[ZeroNode] No current address.")
              except:
                self.logger.exception ("[ZeroNode] Current node address (un-parseable): " + str(token))
              return

        elif (msgtype == 'LID'):
          if (tokeni == 1):
            try:
              self.zero.current.index.gotlastid (int(token))
            except ValueError:
              self.zero.current.index.reset ()
              self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
              return

        elif (msgtype == 'IDS'):
          if (tokeni == 1):
            self.tokens[0] = token

          elif (tokeni == 2):
            try:
              self.zero.current.index.gotids (int(self.tokens[0]), int(token))
            except ValueError:
              self.zero.current.index.reset ()
              self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
              return

        elif (msgtype == 'DBG'):
          if (tokeni == 1):
            self.logger.info ("[Buoy] [DBG] " + token)

        elif (msgtype == 'DBGZ'):
          if (tokeni == 1):
            self.logger.info ("[ZeroNode] [DBG] " + token)

        elif (msgtype == 'ERR'):
          self.zero.current.index.reset ()
          if (tokeni == 1):
            try:
              self.logger.error ("[Buoy] Received error: [" + token + "] " + Buoy.error_strings[int(token)])
            except ValueError:
              self.logger.error ("[Buoy] Received error: [" + token + "]")
              self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
              return

        else:
          self.zero.current.index.reset ()
          self.logger.error ("[Protocol] Unknown message: " + str(buf))
          return

      tokeni += 1
  # }}}

# }}}

