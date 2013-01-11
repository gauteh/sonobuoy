#! /bin/bash
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-01-11
#
# Create a virtual db for 'eev', run in a safe 04../db dir

for i in ../*/*.mseed; do
  d=$(basename ${i})
  echo "Linking (soft): ${i} to ${d}.."
  ln -s ${i}
done

echo "Create file list.."
dirf ../*/*.S*

mv filenr.lis vdb.lis

echo "Ready file: vdb.lis"
