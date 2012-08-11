#! /usr/bin/python2
#
# Buoytrace: Show status and plot trace in realtime of selected buoys
#
# Author: Gaute Hope <eg@gaute.vetsj.com>
# Date:   2012-08-10
#
# Requires:
# - GTK 3
# - python2
# - python2-gobject

import os
import os.path
import sys

# Add path of Zero to modules path
zdir = os.path.join (os.path.dirname(sys.argv[0]), '../../')
sys.path.append (zdir[:-1])

from gi.repository import Gtk

from buoys      import buoys
from buoywidget import *

class BuoyTrace:
  UI_FILE = 'buoytrace.glade'

  buoywidgets = []

  def __init__ (self):
    Gtk.init (sys.argv)

    # Load UI from glade file and set up signals
    self.ui = Gtk.Builder ()
    self.ui.add_from_file (self.UI_FILE)

    self.window_main = self.ui.get_object ('window_main')
    self.window_main.show_all ()

    self.window_main.connect ("delete-event", self.quit)
    self.window_main.connect ("destroy", self.quit)

    self.btn_quit = self.ui.get_object ('btn_quit')
    self.btn_quit.connect ("clicked", self.quit)

    self.box_buoys = self.ui.get_object ('box_buoys')

    # Load buoys
    for b in buoys:
      bw = BuoyWidget (self, b)
      self.buoywidgets.append (bw)
      self.box_buoys.pack_start (bw.frame, True, True, 3)

  def switchoffbuoys (self, ex):
    for bw in self.buoywidgets:
      if bw is not ex:
        bw.sw_monitor.set_active (False)

  def run (self):
    Gtk.main ()

  def quit (self, event = None, args = None):
    Gtk.main_quit ()

if __name__ == '__main__':
  b = BuoyTrace ()
  b.run ()


