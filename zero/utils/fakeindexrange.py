#! /usr/bin/python2
#
# fakeindexrange.py: Outputs a range of the format in indexes which should
#                    be able to fool Zero into thinking the indexes exist.
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-08-18
#

import sys

for i in range(int(sys.argv[1]), int(sys.argv[2])):
  print str(i) + ",True"

