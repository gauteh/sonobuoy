#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2011-09-28
#
# Command line interface to ZeroManager
#
# Requires:
# - texttable

from ui import *
from zero import *
from buoy import *
from ad import *

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

  def summary (self):
    print "Summary of known buoys:"

    try:
      q_length = self.z.ad_queue_length ()
    except:
      print "Error: Lost connection to Zero"
      sys.exit (1)

    t = Texttable ()
    t.header (['A', 'Name', 'Last value', 'Config', 'Queue pos.', 'Sample rate', 'Total samples'])
    t.set_cols_align  (["c", "l", "l", "l", "c", "c", "c"])
    t.set_cols_valign (["m", "m", "m", "m", "m", "m", "m"])
    t.set_cols_width  ([1, 10, 8, 8, 8, 12, 10,])

    try:
      for s in self.z.buoy_statuses ():
        f = "{0:.2f} Hz".format(0 if s[5] == 0 else q_length * 1000 / s[5])
        t.add_row ([("X" if s[0] else ""),] + s[1:5] + [f,] + [s[6]])
    except:
      print "Error: Lost connection to Zero"
      sys.exit (1)

    print t.draw ()

    try:
      print "Total: ", self.z.bouy_count ()
    except:
      print "Error: Lost connection to Zero"
      sys.exit (1)

  def show (self, b):
    t = Texttable ()

    try:
      s = self.z.buoy_status_by_name (b)
    except Exception as e:
      print "Error: Lost connection to Zero"
      self.r_monitor = False
      #self.m.close ()
      sys.exit (1)
      throw (e)

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
      #self.z.stop ()
    except:
      print "Error: Lost connection to Zero."
      sys.exit (1)

  ''' Usage output '''
  def help (self):
    print ""
    print "Usage: ", sys.argv[0] + " command [arguments]"
    print ""
    print "Commands:"
    print "summary                  Print summary of connected nodes"
    print "show [buoy name]         Show detailed information about buoy"
    print "monitor [buoy name]      Regularily print information about buoy"
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

    elif sys.argv[1] == 'help' or sys.argv[1] == '--help':
      self.help ()
      return

    else:
      print "Error: Unknown command."
      self.help ()
      return

if __name__ == '__main__':
  z = zCLI ()

