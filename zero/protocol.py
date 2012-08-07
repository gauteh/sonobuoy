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

  adressedbuoy  = 0     # id of buoy addressed on zeronode
  zerooutput    = -1    # output mode of zeronode, 0 = ota, 1 = uart, -1 = undef.
  zeroradiorate = -1     # radiorate of zeronode

  tokens = [0, 0, 0, 0, 0, 0, 0, 0] # temp for storing tokens during parsing

  radiorate_confirmed = False # is set true on first received telegram after
                              # radio change

  def __init__ (self, z):
    self.zero     = z
    self.logger   = z.logger

# Commands (buoy and zeronode) and ouput {{{
  def znsetaddress (self):
    if self.zero.ser is not None:
      # Address buoy
      self.logger.info ("[ZeroNode] Setting to buoy: " + self.zero.current.name + "[" + str(self.zero.current.id) + "] address: " + str(self.zero.current.address_p))
      _msg = 'ZA,' + self.zero.current.address_p
      self.zero.send ('$' + _msg + '*' + gen_checksum (_msg))
      self.adressedbuoy = self.zero.current.id

  # Request status from zeronode
  def zngetstatus (self):
    if self.zero.ser is not None:
      self.zero.send ('$ZS*' + gen_checksum ('ZS'))

  def znconnect (self):
    if self.zero.ser is not None:
      self.logger.info ("[ZeroNode] Connecting to current buoy: " + self.zero.current.name + "[" + str(self.zero.current.id) + "]")
      self.zero.send ('$ZC*' + gen_checksum ('ZA'))

  def znportalmode (self):
    if self.zero.ser is not None:
      # Put into portal mode and exit
      self.logger.info ("[ZeroNode] Going into portal mode..")
      self.zero.send ("$ZP*" + gen_checksum ('ZP'))
      self.zero.stop ()

  def resetbuoy (self):
    if self.zero.ser is not None:
      self.logger.info ("[ZeroNode] Resetting current buoy.")
      if self.zero.current.id != self.adressedbuoy:
        self.znsetaddress ()
        self.znconnect ()
      self.zero.send ("$ZR*" + gen_checksum ('ZR'))

  def znoutputuart (self):
    if self.zero.ser is not None:
      self.logger.info ("[ZeroNode] Setting Zero RF to output locally.")
      self.zero.send ("$ZU*" + gen_checksum ('ZU'))
      self.zerooutput = 1

  def znoutputwireless (self):
    if self.zero.ser is not None:
      self.logger.info ("[ZeroNode] Setting Zero RF to output on air.")
      self.zero.send ("$ZT*" + gen_checksum ('ZT'))
      self.zerooutput = 0

  def znradiorate (self, rate):
    if self.zero.ser is not None:
      self.logger.info ("[ZeroNode] Setting Zero RF radio rate to: " + str(rate))
      if rate >= 0 and rate <= 3:
        msg = "$Z" + str(rate) + "*"
        self.zero.send (msg + gen_checksum(msg))
        self.zeroradiorate = rate
        self.radiorate_confirmed = False
      else:
        self.logger.error ("[ZeroNode] Zero RF radio rate: Rate out of range.")

  def znbuoyradiorate (self, rate):
    if self.zero.ser is not None:
      # this one also sets the ZN radiorate
      if self.zero.current is not None:
        self.logger.info ("[ZeroNode] Setting radio rate on current buoy to: " + str(rate))
        if rate >= 0 and rate <= 3:
          msg = "$ZB" + str(rate) + "*"
          self.zero.send (msg + gen_checksum(msg))
          self.zero.current.radiorate = rate
          self.zero.current.set_radiorate_t = time.time ()
          #time.sleep (2) # allow rpc to be sent
          # snap script on zero node waits for rpc to be sent, then
          # changes rate of itself
          self.zeroradiorate = rate
          self.radiorate_confirmed = False
        else:
          self.logger.error ("[ZeroNode] Buoy radio rate: Rate out of range.")

      else:
        self.logger.error ("[ZeroNode] Set radio rate on buoy: No current buoy.")

  def ensure_zn_address (self):
    if self.zero.current is not None and self.zero.ser is not None:
      if self.zero.current.id != self.adressedbuoy:
        self.znsetaddress ()
        self.znconnect ()

      if self.zerooutput != 0:
        self.znoutputwireless ()

      if self.zero.current.radiorate != self.zeroradiorate:
        self.znradiorate (self.zero.current.radiorate)

      # check whether timeout for radiorate is approaching
      RADIORATE_RESET_TIMER = 10 # secs margin before reseting timer on buoy
                                 # dont wanna risk a rate change in the middle
                                 # of a data tx.
      if (time.time () - self.zero.current.set_radiorate_t) >= (Buoy.RADIORATE_TIMEOUT - RADIORATE_RESET_TIMER) and self.zero.current.radiorate != 0:
        self.logger.info ("[ZeroNode] Ensure radiorate.")
        self.znbuoyradiorate (self.zero.current.radiorate)

    else:
      self.logger.error ("[ZeroNode] Ensure address: No current buoy set.")

  def send (self, msg):
    if self.zero.ser is not None:
      self.ensure_zn_address ()

      # Encapsulate and add checksum
      msg = '$' + msg + '*' + gen_checksum (msg)
      self.zero.send (msg)
# }}}

# Parser and handler {{{
  # Last type received
  lasttype = ''

  # Handler {{{
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
        self.zero.current.index.reset (keepradiorate = True)

      # Check if we're receiving sane amounts of data..
      if len(self.a_buf) > 80:
        self.a_buf = ''
        self.a_receive_state = 0
  # }}}

  def a_parse (self, buf): # {{{
    # Test checksum
    if (buf[-2:] == 'NN'):
      #self.logger.debug ("[Protocol] Checksum not provided on received message")
      pass
    elif (not test_checksum (buf)):
      self.logger.info ("[Protocol] Message discarded, checksum failed.")
      self.logger.info ("[Protocol] Discarded: " + buf)
      self.zero.current.index.reset (keepradiorate = True)
      return

    # Parse message
    tokeni = 0
    token  = ''

    msgtype = ''
    subtype = ''

    i = 0
    l = len (buf)

    # Telegram received, so radio rate must be correctly set
    self.radiorate_confirmed = False

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
            self.zero.current.index.reset (keepradiorate = True)
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
                  self.zero.current.index.reset (keepradiorate = True)
                  self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                  return

              elif (tokeni == 4):
                try:
                  self.zero.current.gps.latitude = float(token) if len(token) else 0
                except ValueError:
                  self.zero.current.index.reset (keepradiorate = True)
                  self.logger.exception ("[Protocol] Could not convert token to float. Discarding rest of message.")
                  return

              elif (tokeni == 5):
                self.zero.current.gps.north = (token[0] == 'N')

              elif (tokeni == 6):
                try:
                  self.zero.current.gps.longitude = float(token) if len(token) > 0 else 0
                except ValueError:
                  self.zero.current.index.reset (keepradiorate = True)
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

                if self.zero.current.remote_protocolversion == 1:
                  self.zero.current.gps.gps_status ()
                  self.zero.current.index.gotstatus ()


              elif (tokeni == 15) and self.zero.current.remote_protocolversion > 1:
                try:
                  self.zero.current.uptime = int (token)

                  self.zero.current.gps.gps_status ()
                  self.zero.current.index.gotstatus ()

                except ValueError:
                  self.zero.current.index.reset (keepradiorate = True)
                  self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")

              else:
                self.zero.current.index.reset (keepradiorate = True)
                self.logger.error ("[Protocol] Too many tokens for message: " + msgtype + ", subtype: " + subtype + ", token: " + token)
                return

            else:
              self.zero.current.index.reset (keepradiorate = True)
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
                  self.zero.current.index.reset (keepradiorate = True)
                  self.logger.error ("[Protocol] Too many tokens for message: " + msgtype + ", subtype: " + subtype)
                  return
              except ValueError:
                self.zero.current.index.reset (keepradiorate = True)
                self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                return

            elif (subtype == 'D'):
              if (tokeni == 2):
                try:
                  self.zero.current.ad.ad_batch_id = int (token)
                except ValueError:
                  self.zero.current.index.reset (keepradiorate = True)
                  self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                  return

              elif (tokeni == 3):
                try:
                  self.zero.current.ad.ad_refno = int (token)
                except ValueError:
                  self.zero.current.index.reset (keepradiorate = True)
                  self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                  return

              elif (tokeni == 4):
                try:
                  self.zero.current.ad.ad_start = int (token)
                except ValueError:
                  self.zero.current.index.reset (keepradiorate = True)
                  self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                  return

              elif (tokeni == 5):
                try:
                  self.zero.current.ad.ad_k_samples = int (token)
                except ValueError:
                  self.zero.current.index.reset (keepradiorate = True)
                  self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                  return

                if (self.zero.current.ad.ad_k_samples > self.zero.current.ad.AD_K_SAMPLES_MAX):
                  self.logger.error ("[Protocol] Too large batch, resetting protocol.")
                  self.zero.current.index.reset (keepradiorate = True)
                  self.a_receive_state = 0
                else:

                  self.zero.current.ad.ad_k_remaining = self.zero.current.ad.ad_k_samples * 4
                  self.zero.current.ad.ad_samples = ''
                  self.a_receive_state = 4

              elif (tokeni == 6):
                try:
                  self.zero.current.ad.ad_reference = int (token)
                except ValueError:
                  self.zero.current.index.reset (keepradiorate = True)
                  self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                  return

              elif (tokeni == 7):
                try:
                  self.zero.current.ad.ad_reference_status = int (token)
                except ValueError:
                  self.zero.current.index.reset (keepradiorate = True)
                  self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                  return

              elif (tokeni == 8):
                self.zero.current.ad.ad_reference_latitude  = token

              elif (tokeni == 9):
                self.zero.current.ad.ad_reference_longitude = token

                #print "[AD] Initiating binary transfer.. samples: ", self.zero.current.ad.ad_k_samples

              elif (tokeni == 10):
                try:
                  self.zero.current.ad.ad_reference_checksum = int (token)
                except ValueError:
                  self.zero.current.index.reset (keepradiorate = True)
                  self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                  return

                #print "[AD] Initiating binary transfer.. samples: ", self.zero.current.ad.ad_k_samples

              else:
                self.zero.current.index.reset (keepradiorate = True)
                self.logger.error ("[Protocol] Too many tokens for message: " + msgtype + ", subtype: " + subtype)
                return

            elif (subtype == 'DE'):
              if not self.waitforreceipt:
                self.zero.current.index.reset (keepradiorate = True)
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
              self.zero.current.index.reset (keepradiorate = True)
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
              self.zero.current.index.reset (keepradiorate = True)
              self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
              return

        elif (msgtype == 'IDS'):
          if (tokeni == 1):
            self.tokens[0] = token

          elif (tokeni == 2):
            try:
              self.zero.current.index.gotids (int(self.tokens[0]), int(token))
            except ValueError:
              self.zero.current.index.reset (keepradiorate = True)
              self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
              return

        elif (msgtype == 'IND'):
          if (tokeni == 1):
            self.tokens[0] = token

          elif (tokeni == 2):
            self.tokens[1] = token

          elif (tokeni == 3):
            try:
              self.zero.current.index.gotid (int(self.tokens[0]), int(self.tokens[1]), int(token))
            except ValueError:
              self.zero.current.index.reset (keepradiorate = True)
              self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
              return

        elif (msgtype == 'I'):
          if (tokeni == 1):
            self.tokens[0] = token
          elif (tokeni == 2):
            self.tokens[1] = token
          elif (tokeni == 3):
            self.tokens[2] = token
            try:
              self.zero.current.index.gotinfo (int(self.tokens[0]), self.tokens[1], int (self.tokens[2]))
            except ValueError:
              self.zero.current.index.reset (keepradiorate = True)
              self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
              return

        elif (msgtype == 'DBG'):
          if (tokeni == 1):
            self.logger.info ("[Buoy] [DBG] " + token)

        elif (msgtype == 'DBGZ'):
          if (tokeni == 1):
            self.logger.info ("[ZeroNode] [DBG] " + token)

        elif (msgtype == 'ERR'):
          if (tokeni == 1):
            self.tokens[0] = token

            if self.zero.current.remote_protocolversion == 1:
              try:
                t = int(token)
                self.logger.error ("[Buoy] Received error: [" + token + "] " + Buoy.error_strings[t])
                self.zero.current.log ("[Buoy] Received error: [" + token + "] " + Buoy.error_strings[t])

                # info command not supported, must be version 1
                if t == 2 and self.zero.current.index.pendingid == 6:
                  self.zero.current.index.gotinfo (self.zero.current.id, 'v1.0.0', 1)
                  return

                # Only reset protocol in case of error with command
                if t == 1 or t == 2 or t == 4 or t == 5 or t == 6 or t == 7 or t == 8:
                  self.zero.current.index.reset (keepradiorate = True)


              except ValueError:
                self.logger.error ("[Buoy] [ID: " + i + "] Received error: [" + token + "]")
                self.zero.current.log ("[Buoy] [ID: " + i + "] Received error: [" + token + "]")
                self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
                self.zero.current.index.reset (keepradiorate = True)
                return


          elif (tokeni == 2):
            try:
              i = int (self.tokens[0])
              t = int(token)
              self.logger.error ("[Buoy] [ID: " + str(i) + "] Received error: [" + token + "] " + Buoy.error_strings[t])
              ii = self.zero.indexofid (i)
              if ii is not None:
                self.zero.buoys[ii].log ("[Buoy] Received error: [" + token + "] " + Buoy.error_strings[t])
              # Only reset protocol in case of error with command
              if t == 1 or t == 2 or t == 4 or t == 5 or t == 6 or t == 7 or t == 8:
                self.zero.buoys[ii].index.reset (keepradiorate = True)
              else:
                self.logger.error ("[Protocol] Received error from unknown buoy.")
                self.zero.current.index.reset (keepradiorate = True)


            except ValueError:
              self.logger.error ("[Buoy] [ID: " + i + "] Received error: [" + token + "]")
              self.zero.current.log ("[Buoy] [ID: " + i + "] Received error: [" + token + "]")
              self.logger.exception ("[Protocol] Could not convert token to int. Discarding rest of message.")
              self.zero.current.index.reset (keepradiorate = True)
              return

        else:
          self.zero.current.index.reset (keepradiorate = True)
          self.logger.error ("[Protocol] Unknown message: " + str(buf))
          return

      tokeni += 1
  # }}}

# }}}

