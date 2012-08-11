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
  _run    = True
  thread  = None
  buoytrace = None

  refreshplot = None

  plot = True
  ids  = []

  def __init__ (self, bt):
    self.thread = Thread (target = self.loop)
    self.buoytrace = bt
    self.refreshplot = Event ()

  def run (self):
    print "[P] Starting.."
    self._run = True
    self.thread.start ()

  def loop (self):
    print "[P] Started."
    while self._run:
      self.checkfiles ()

      self.refreshplot.wait (5)

  def checkfiles (self):
    # Check if there are new DTT's
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

  def close (self):
    print "[P] Stopping.."
    self._run = False
    self.refreshplot.set ()
    self.thread.join ()
    print "[P] Finished."

  def event (self):
    self.refreshplot.set ()

