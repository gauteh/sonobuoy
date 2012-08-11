#! /usr/bin/python2
#
# Buoywidget: Widget for each buoy
#
# Author: Gaute Hope <eg@gaute.vetsj.com>
# Date:   2012-08-10

import os
import os.path
import sys

# Add path of Zero to modules path
zdir = os.path.join (os.path.dirname(sys.argv[0]), '../../')
sys.path.append (zdir[:-1])

from gi.repository import Gtk

class BuoyWidget (Gtk.Container):
  buoytrace = None
  frame     = None
  buoy      = None

  datadir   = os.path.join (os.path.dirname(sys.argv[0]), '../../log/')

  def __init__ (self, bt, buoy):
    Gtk.Container.__init__ (self)

    self.buoytrace = bt
    self.buoy = buoy

    self.datadir = os.path.join (self.datadir, self.buoy['name'])

    ui = Gtk.Builder ()
    ui.add_objects_from_file (self.buoytrace.UI_FILE, ['frame_buoy', ])

    self.frame = ui.get_object ('frame_buoy')
    self.sw_monitor = ui.get_object ('sw_monitor')

    self.label = self.frame.get_label_widget ()
    self.label.set_markup ('<b>' + self.buoy['name'] + '</b>')

    self.sw_monitor.connect ('notify::active', self.activated)

  def activated (self, *args):
    if self.sw_monitor.get_active ():
      self.buoytrace.switchoffbuoys (self)



