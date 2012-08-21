#! /usr/bin/python2
#
# fakedtt.py: Generates a fake DTT and ITT with a sine signal
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-08-21
#

import os
import math
import time


id         =   1 # id of dtt
samplerate = 250.0 # signal
batches    =  40.0
samples    =  40.0 * 1024.0

amplitude  =  math.pow(2, 24)
freq       =  0.1  # of generated signal (sine, etc..)


dttf = open (str(id) + '.DTT', 'w')
ittf = open (str(id) + '.ITT', 'w')

# Write ITT header
ittf.write (str(3) + '\n') # local version
ittf.write (str(9) + '\n') # remote version
ittf.write (str(id) + '\n')
ittf.write (str(int(samples)) + '\n')
ittf.write (str(int(batches)) + '\n')
ittf.write ("True\n")
ittf.write ("True\n")

tlen = samples / samplerate
td   = 1 / samplerate
t    = 0.0

func = math.sin

mt = time.time ()
s = 0
ref = 0

maxint = 2^32

while s < samples:
  # Write ref
  if s % 1024 == 0:
    dttf.write ('R,1024,' + str(ref) + "," + str(int(mt * 1e6)) + ",15,0S,0E,0\n")
    ittf.write (str(ref) + "," + str(int(mt * 1e6)) + ",15,0S,0E,0," + str(s + ref) + ",0\n")
    ref += 1
    mt  += 1024.0 / 250.0

  v = int(amplitude * func (freq * t * 2 * math.pi))
  if v < 0:
    v += int(math.pow(2, 32))

  dttf.write (str(v) + '\n')
  t += td
  s += 1

dttf.close ()
ittf.close ()

print "fakedtt: Wrote " + str(int(samples)) + " samples to " + str(id) + ".DTT, using: " + str(func)

