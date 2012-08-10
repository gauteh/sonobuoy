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

from buoys import buoys

class BuoyTrace:
  UI_FILE = 'buoytrace.glade'

  def __init__ (self):
    Gtk.init (sys.argv)

    # Load UI from glade file and set up signals
    ui = Gtk.Builder ()
    ui.add_from_file (self.UI_FILE)

    window_main = ui.get_object ('window_main')
    window_main.show_all ()

    window_main.connect ("delete-event", self.quit)
    window_main.connect ("destroy", self.quit)


  def run (self):
    Gtk.main ()

  def quit (self, event = None, args = None):
    Gtk.main_quit ()

if __name__ == '__main__':
  b = BuoyTrace ()
  b.run ()


