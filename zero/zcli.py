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
from texttable import *

class zCLI:
  m         = None
  z         = None

  def __init__ (self):
    self.m = ZeroUIManager ()
    self.m.setup_client ()

    try:
      self.m.connect ()
    except:
      print "Error: Could not connect to Zero manager. Is the service running?"

    self.z = self.m.get_zcliservice ()
    self.go ()

  def summary (self):
    print "Summary of known buoys:"
    #print "=" * 80 

    q_length = self.z.ad_queue_length ()

    t = Texttable ()
    t.header (['A', 'Name', 'Last value', 'Config', 'Queue pos.', 'Sample rate', 'Total samples'])
    t.set_cols_align  (["c", "l", "l", "l", "c", "c", "c"])
    t.set_cols_valign (["m", "m", "m", "m", "m", "m", "m"])
    t.set_cols_width  ([1, 10, 8, 8, 8, 12, 10,])

    for s in self.z.buoy_statuses ():
      f = "{0:.2f} Hz".format(0 if s[5] == 0 else q_length * 1000 / s[5])
      t.add_row ([("X" if s[0] else ""),] + s[1:5] + [f,] + [s[6]])

    print t.draw ()

    print "Total: ", self.z.bouy_count ()

  def show (self, b):
    q_length = self.z.ad_queue_length ()
    t = Texttable ()

    s = self.z.buoy_status_by_name (b)
    if s is None:
      print "Error: No such buoy."
      sys.exit (1)

    t.header (['Buoy:', b])
    t.set_cols_align (["r", "l"])
    t.set_cols_width ([20, 50])

    t.add_rows ([ ["Active:", s[0]],
                  ["Current value:", s[2]],
                  ["Control register:", s[3]],
                  ["Queue position:", s[4]],
                  ["Sample rate:", "{0:.2f} Hz".format(0 if s[5] == 0 else q_length * 1000 / s[5])],
                  ["Total samples:", s[6]]
                  ], False)


    print t.draw ()

  def monitor (self, b):
    while True:
      #print chr(27) + "[2J" # Clear screen
      os.system ('clear')
      print "Monitoring buoy:"
      self.show (b)
      time.sleep (1)

  def stop (self):
    print "Stopping Zero Manager.."
    self.z.stop ()

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

    elif sys.argv[1] == 'help' or sys.argv[1] == '--help':
      self.help ()

    else:
      print "Error: Unknown command."
      self.help ()
      return

if __name__ == '__main__':
  z = zCLI ()

