#! /usr/bin/python2
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-11-26
#
# Prepare HYPOSAT job, run from (readily picked) event dir or with S file
# as first argument.

# Input needed for HYPOSAT:
# - Job file: .IN (phase picks, etc) : traveltime corrected for waterdepth
# - Parameter file
# - Station coordinates
# - Regional model
# - as well as some general velocity models and data files

import sys
import os
import os.path
import shutil
import datetime

if len(sys.argv) == 2:
  event = sys.argv[1]
else:
  event = os.path.basename(os.getcwd())

if not os.path.exists (event):
  print "Could not open event file: %s.." % event
  sys.exit (1)


jobd = 'job_01'
if not os.path.exists (jobd):
  os.makedirs (jobd)

print "Creating HYPOSAT job for: %s in %s.." % (event, jobd)



