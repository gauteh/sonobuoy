#! /bin/bash
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-01-13
#
# Helper script to get more dats from continus database, run from station
# folder. Input will be parsed like seq can do it.
#

dbroot=../../..

s=$(basename `pwd`)

# figure out buoy number
buoy=${s:${#s}-1:${#s}}

echo "Getting dats for station: ${s} (buoy ${buoy}).."

broot=${dbroot}/b${buoy}/dat


if [ $# == 2 ]; then
  range=$(seq $*)
else
  range=$1
fi

for i in ${range[@]}; do
  echo "Copying: $i.."

  cp ${broot}/$i.* .

done

