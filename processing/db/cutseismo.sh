#! /bin/bash
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-11-27
#
# Wrapper for wavetool (SEISAN) for cutting out segment of all seismochannels
# and putting them in GAKS.
#
# Usage: cutseismo.sh hhmmss.s interval
#
# Time follows wavetool manual.
# Important: a . must be present to get ABS time.

# figure out date
eventdir=$(basename $(pwd))

yearmonth=${eventdir#*.S}
day=${eventdir%%-*}

time=${yearmonth}${day}$1

echo $#

if [[ $# -ne "2" ]]; then
  echo "cutseismo.sh takes exactly two arguments."
  exit 1
fi

echo "Extracting from: $yearmonth$day$1.."
echo "Interval: $2"

cd GAKS/
dirf ../*GAKS_*
wavetool -wav_files filenr.lis -start $time -duration $2 -format MSEED

year=$(echo $yearmonth | cut -c1-4)
month=$(echo $yearmonth | cut -c5-6)
hour=$(echo $1 | cut -c1-4)
newevent=$year-$month-$day-$hour-00.GB_GAKS_G3____.mseed
mv wavetool.out $newevent

cd ..

echo "Moved output file to: $newevent."

