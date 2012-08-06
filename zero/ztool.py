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
    if len(sys.argv) == 1 or sys.argv[1] == "help":
      self.go ()
      sys.exit (0)

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

  def znradiorate (self, rate):
    print "Setting ZeroNode radiorate..",
    if rate >= 0 and rate <= 3:
      self.z.setznradiorate (rate)
      print "done."
    else:
      print "error: range."

  def radiorate (self, rate):
    print "Setting current buoy radiorate..",
    if rate >= 0 and rate <= 3:
      self.z.setbuoyradiorate (rate)
      print "done."
    else:
      print "error: range."

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

    for s in self.z.buoy_statuses ():
      q_length = s['batch_length']
      f = "{0:.2f} Hz".format(0 if s['ad_queue_time'] == 0 else q_length * 1000 / s['ad_queue_time'])
      t.add_row ([("X" if s['active'] else ""),] + [s['name'], s['ad_value'], s['ad_config'], s['ad_qposition']] + [f,] + [s['ad_nsamples']])
      #print e
      #print "Error: Lost connection to Zero"
      #raise e
      #os._exit (1)

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

    q_length = s['batch_length']

    t.header (['Buoy:', b])
    t.set_cols_align (["r", "l"])
    t.set_cols_width ([20, 50])

    t.add_rows ([ ["Active:", s['active']],
                  ["Current value:", s['ad_value']],
                  ["Queue position:", s['ad_qposition']],
                  ["Sample rate:", "{0:.2f} Hz".format(0 if s['ad_queue_time'] == 0 else q_length * 1000 / s['ad_queue_time']) + " ( Control register: " + s['ad_config'] + " )"],
                  ["Total samples:", s['ad_nsamples']],
                  ["Position:", "{0:.4f}{1}, {2:.4f}{3}".format(s['latitude'], ("N" if s['north'] else "S"), s['longitude'], ("E" if s['east'] else "W")) + " ( Valid: " + str(s['valid']) + " )"],
                  ["Synchronization:", "( Sync: " + str(s['has_sync']) + " )" + '( Time: ' + str(s['has_time']) + " )" + '( Sync ref: ' + str(s['has_sync_reference']) + ' )'],
                  ["Time:", time.asctime(time.gmtime(float(s['unix_time']))) + ' (' + str(s['time']) + ', ' + str(s['date']) + ')'],
                  ["Version:", s['version'] + ' (protocol: ' + str(s['protocolversion']) + ')'],
                  ["Uptime: ", str(s['uptime'] / 1000) + ' s (latest id: ' + str(s['lastid']) + ')'],
                  ], False)


    print t.draw ()

  def monitor (self, b):
    while self.r_monitor:
      #print chr(27) + "[2J" # Clear screen
      os.system ('clear')
      print "Monitoring buoy:"
      self.show (b)
      time.sleep (1)

  def chunks (self, b):
    print "Complete chunks for buoy: " + b
    print "================================="

    print "Not implemented."
    return


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
    print "chunks [buoy name]       Show completeness of data for buoy"
    print ""
    print "=> Buoy commands:"
    print "getstatus                Request status from current buoy"
    print "getlatestbatch           Request latest batch from current buoy"
    print "getids [start]           Get ids"
    print "radiorate [rate]         Set radio rate of current buoy (0-3)"
    print "reset                    Reset CPU card on current buoy"
    print ""
    print "=> ZeroNode commands:"
    print "zngetstatus              Request status from zero node"
    print "znportalmode             Put zeronode in portal mode and exit zero"
    print "znaddress                Set address of zeronode to current buoy"
    print "znconnect                Connect to currently specified address"
    print "znoutputuart             Configure output of Zero to go to uart"
    print "znoutputwireless         Configure output of Zero to go to wireless"
    print "znradiorate [rate]       Set radio rate of Zero (0-3)"
    print ""
    print "=> Zero Manager commands:"
    print "stopacquire              Stop continuous data transmission from buoys"
    print "startacquire             Start contnuous data transmission from buoys"
    print "stop                     Stop Zero Manager"
    print ""
    print "=> Other:"
    print "help                     Show this help message"

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

    elif sys.argv[1] == 'znradiorate':
      if len(sys.argv) != 3:
        print "Error: Must specify radio rate"
        return
      self.znradiorate (int(sys.argv[2]))

    elif sys.argv[1] == 'radiorate':
      if len(sys.argv) != 3:
        print "Error: Must specify radio rate"
        return
      self.radiorate (int(sys.argv[2]))

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

    elif sys.argv[1] == 'chunks':
      if len(sys.argv) != 3:
        print "Error: No Buoy identifier specified."
        self.help ()
        return

      self.chunks (sys.argv[2])


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

