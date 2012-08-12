#! /usr/bin/python2
#
# Plot: Manages gnuplot
#
# Author: Gaute Hope <eg@gaute.vetsj.com>
# Date:   2012-08-11

import os
import sys
import time
from threading import Thread, Event

class Plot:
  _run      = True
  thread    = None
  buoytrace = None

  refreshplot = None

  plot = True
  ids  = []
  greatestid = 0

  def __init__ (self, bt):
    self.thread = Thread (target = self.loop)
    self.buoytrace = bt
    self.refreshplot = Event ()
    self.refreshplot.set () # Run through initially

  def run (self):
    print "[P] Starting.."
    self._run = True
    self.thread.start ()

  def loop (self):
    print "[P] Started."
    while self._run:
      if self.refreshplot.wait (5):
        self.refreshplot.clear ()

        if self.plot:
          self.checkfiles ()
          self.replot ()

      else:
        if self.plot:
          self.checkfiles ()
          if max(self.ids) > self.greatestid:
            print "[P] New ID: " + str(max(self.ids))
            self.replot ()

  def replot (self):
    self.greatestid = max(self.ids)
    print "[P] Replot.. (latest id: " + str(self.greatestid) + ")"

  def checkfiles (self):
    # Check if there are new DTT's
    print "[P] Checking files.."
    if self.buoytrace.currentbuoy is not None:
      self.buoytrace.lbl_status.set_label ('Checking files..')
      self.buoytrace.window_main.show_all ()
      files = os.listdir (self.buoytrace.currentbuoy.datadir)
      self.ids = []
      for f in files:
        if f[-3:] == 'DTT':
          try:
            self.ids.append (int(f[:-4]))
          except ValueError:
            print "[P] Could not add id: " + f

      self.ids.sort ()
      self.buoytrace.lbl_status.set_label ('Checking files.. done.')
    else:
      print "[P] No current buoy."

  def close (self):
    print "[P] Stopping.."
    self._run = False
    self.plot = False
    self.refreshplot.set ()
    self.thread.join ()
    print "[P] Finished."

  def event (self):
    self.refreshplot.set ()

