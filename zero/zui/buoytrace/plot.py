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

  def __init__ (self, bt):
    self.thread = Thread (target = self.loop)
    self.buoytrace = bt
    self.refreshplot = Event ()

  def run (self):
    self._run = True
    self.thread.start ()

  def loop (self):
    while self._run:
      self.refreshplot.wait (5)

  def close (self):
    print "[P] Stopping.."
    self._run = False
    self.refreshplot.set ()
    self.thread.join ()
    print "[P] Finished." 

  def event (self):
    self.refreshplot.set ()

