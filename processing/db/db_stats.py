#! /usr/bin/python2
#
# Author: Gaute Hope <eg@gaute.vetsj.com>
# Date:   2012-11-22
#
# Print stats about events in DB (assuming all are in same month..)
#

import sys
import os

cwd = os.getcwd ()

files = os.listdir (cwd)

days = [] # list with days and event number pairs

for f in files:
  if '.S' in f:
    d  = f[:2]
    ym = f[-6:]

    dd = ym + '-' + d
    
    # find and append to existing key if possible
    i = 0
    found = False
    while i < len(days):
      if days[i][0] == dd:
        days[i][1] += 1
        found = True
        break
      i += 1
    if not found:
      days.append ([dd, 1])

print "Event database stats:"

# print daily stats
total = 0
for k,v in days:
  print "%s ..: %3d" % (k , v)
  #print k + " .: " + str(v)
  total += v

# print total
print "total ......: %3d" % total


