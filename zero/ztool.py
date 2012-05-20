#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2011-09-28
#
# Command line interface to ZeroManager
#
# Requires:
# - texttable

from ui   import *
from zero import *
from buoy import *
from ad   import *

import sys
import os
import time
from texttable import *

class zCLI:
  m         = None
  z         = None

  # For monitor function
  r_monitor = True

  def __init__ (self):
    try:
      self.m = ZeroUIManager ()
      self.m.setup_client ()
    except:
      print "Error: Could not connect to Zero manager. Is the service running?"
      sys.exit (1)

    try:
      self.m.connect ()
    except:
      print "Error: Could not connect to Zero manager. Is the service running?"
      sys.exit (1)

    try:
      self.z = self.m.get_zcliservice ()
    except:
      print "Error: Could not connect to Zero manager. Is the service running?"
      sys.exit (1)

    self.go ()

  def getstatus (self):
    print "Getting status..",
    self.z.getstatus ()
    print "done."

  def resetbuoy (self):
    print "Resetting current buoy..",
    self.z.resetbuoy ()
    print "done."

  def getlatestbatch (self):
    print "Getting latest batch..",
    self.z.getlatestbatch ()
    print "done."

  def getids (self, start):
    print "Getting ids..",
    self.z.getids (start)
    print "done."

  def zngetstatus (self):
    print "Getting Zero Node status..",
    self.m.zngetstatus ()
    print "done."

  def znportalmode (self):
    print "Setting zeronode in portal mode and stopping zero.."
    try:
      self.m.znportalmode ()
    except:
      os._exit (1)

  def znconnect (self):
    print "Requesting Zero node to connect to buoy node.."
    self.m.znconnect ()

  def znaddress (self):
    print "Setting address on Zero node to address of current buoy.."
    self.m.znsetaddress ()

  def znoutputuart (self):
    print "Requesting Zero node to output to uart.."
    self.m.znoutputuart ()

  def znoutputwireless (self):
    print "Requesting Zero node to output to wireless.."
    self.m.znoutputwireless ()

  def summary (self):
    print "Summary of known buoys:"

    t = Texttable ()
    t.header (['A', 'Name', 'Last value', 'Config', 'Queue pos.', 'Sample rate', 'Total samples'])
    t.set_cols_align  (["c", "l", "l", "l", "c", "c", "c"])
    t.set_cols_valign (["m", "m", "m", "m", "m", "m", "m"])
    t.set_cols_width  ([1, 6, 10, 10, 8, 12, 10,])

    try:
      for s in self.z.buoy_statuses ():
        q_length = s[18]
        f = "{0:.2f} Hz".format(0 if s[5] == 0 else q_length * 1000 / s[5])
        t.add_row ([("X" if s[0] else ""),] + s[1:5] + [f,] + [s[6]])
    except:
      print "Error: Lost connection to Zero"
      os._exit (1)

    print t.draw ()

    try:
      print "Total: ", self.z.bouy_count ()
    except:
      print "Error: Lost connection to Zero"
      os._exit (1)

  def show (self, b):
    t = Texttable ()

    try:
      s = self.z.buoy_status_by_name (b)
    except Exception as e:
      print "Error: Lost connection to Zero"
      self.r_monitor = False
      os._exit (1)

    if s is None:
      print "Error: No such buoy."
      sys.exit (1)

    q_length = s[18]

    t.header (['Buoy:', b])
    t.set_cols_align (["r", "l"])
    t.set_cols_width ([20, 50])

    t.add_rows ([ ["Active:", s[0]],
                  ["Current value:", s[2]],
                  ["Queue position:", s[4]],
                  ["Sample rate:", "{0:.2f} Hz".format(0 if s[5] == 0 else q_length * 1000 / s[5]) + " ( Control register: " + s[3] + " )"],
                  ["Total samples:", s[6]],
                  ["Position:", "{0:.4f}{1}, {2:.4f}{3}".format(s[7], ("N" if s[8] else "S"), s[9], ("E" if s[10] else "W")) + " ( Valid: " + str(s[11]) + " )"],
                  ["Synchronization:", "( Sync: " + str(s[16]) + " )" + '( Time: ' + str(s[15]) + " )" + '( Sync ref: ' + str(s[17]) + ' )'],
                  ["Time:", time.asctime(time.gmtime(float(s[13]))) + ' (' + str(s[12]) + ', ' + str(s[14]) + ')']
                  ], False)


    print t.draw ()

  def monitor (self, b):
    while self.r_monitor:
      #print chr(27) + "[2J" # Clear screen
      os.system ('clear')
      print "Monitoring buoy:"
      self.show (b)
      time.sleep (1)

  def stop (self):
    print "Stopping Zero Manager.."
    try:
      self.m.stop ()
    except:
      os._exit (1)

  def startacquire (self):
    print "Starting continuous transmission from buoys.."
    self.z.startacquire ()

  def stopacquire (self):
    print "Stopping continuous transmission from buoys.."
    self.z.stopacquire ()

  ''' Usage output '''
  def help (self):
    print ""
    print "Usage: ", sys.argv[0] + " command [arguments]"
    print ""
    print "=> Data commands:"
    print "summary                  Print summary of connected nodes"
    print "show [buoy name]         Show detailed information about buoy"
    print "monitor [buoy name]      Regularily print information about buoy"
    print ""
    print "=> Buoy commands:"
    print "getstatus                Request status from current buoy"
    print "getlatestbatch           Request latest batch from current buoy"
    print "getids [start]           Get ids"
    print "reset                    Reset CPU card on current buoy"
    print ""
    print "=> ZeroNode commands:"
    print "zngetstatus              Request status from zero node"
    print "znportalmode             Put zeronode in portal mode and exit zero"
    print "znaddress                Set address of zeronode to current buoy"
    print "znconnect                Connect to currently specified address"
    print "znoutputuart             Configure output of Zero to go to uart"
    print "znoutputwireless         Configure output of Zero to go to wireless"
    print ""
    print "=> Zero Manager commands:"
    print "stopacquire              Stop continuous data transmission from buoys"
    print "startacquire             Start contnuous data transmission from buoys"
    print "stop                     Stop Zero Manager"

  def go (self):
    # Parse options
    # First argument is command
    if len(sys.argv) == 1:
      print "Error: No command."
      self.help ()
      return

    if sys.argv[1] == 'summary':
      self.summary ()

    elif sys.argv[1] == 'getstatus':
      self.getstatus ()

    elif sys.argv[1] == 'getlatestbatch':
      self.getlatestbatch ()

    elif sys.argv[1] == 'getids':
      if len(sys.argv) != 3:
        print "Error: Must specify starting id."
        return
      self.getids (int(sys.argv[2]))

    elif sys.argv[1] == 'zngetstatus':
      self.zngetstatus ()

    elif sys.argv[1] == 'reset':
      self.resetbuoy ()

    elif sys.argv[1] == 'znportalmode':
      self.znportalmode ()

    elif sys.argv[1] == 'znaddress':
      self.znaddress ()

    elif sys.argv[1] == 'znconnect':
      self.znconnect ()

    elif sys.argv[1] == 'znoutputuart':
      self.znoutputuart ()

    elif sys.argv[1] == 'znoutputwireless':
      self.znoutputwireless ()

    elif sys.argv[1] == 'show':
      if len(sys.argv) != 3:
        print "Error: No Buoy identifier specified."
        self.help ()
        return
      self.show (sys.argv[2])

    elif sys.argv[1] == 'monitor':
      if len(sys.argv) != 3:
        print "Error: No Buoy identifier specified."
        self.help ()
        return
      self.monitor (sys.argv[2])

    elif sys.argv[1] == 'stop':
      self.stop ()
      return

    elif sys.argv[1] == 'startacquire':
      self.startacquire ()
      return

    elif sys.argv[1] == 'stopacquire':
      self.stopacquire ()
      return

    elif sys.argv[1] == 'help' or sys.argv[1] == '--help':
      self.help ()
      return

    else:
      print "Error: Unknown command."
      self.help ()
      return

if __name__ == '__main__':
  z = zCLI ()

