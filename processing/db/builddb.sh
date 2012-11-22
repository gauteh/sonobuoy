#! /bin/bash
#
# 2012-11-21 / Gaute Hope <eg@gaute.vetsj.com>
#
# Build miniSEED database with info and track files
#
# better use this as a list of commands rather than running the script..

# * b2, b3, b4 contains a folder dat each with raw DAT files

# Create dirs
mkdir 01_cont 02_events_raw

# Create mseed files
cd b2/dat
mkdir out
matlab -nodisplay -r "makehourly ('G3', 'GAK2', 3:3026); exit;"
cd ../../

cd b3/dat
mkdir out
matlab -nodisplay -r "makehourly ('G3', 'GAK3', 3:3191); exit;"
cd ../../

cd b4/dat
mkdir out
matlab -nodisplay -r "makehourly ('G3', 'GAK4', 3:3627); exit;"
cd ../../

# move output files to station folders (continuous data)
mkdir -p b2/cont/wav
mkdir -p b3/cont/wav
mkdir -p b4/cont/wav
mv b2/dat/out/* b2/cont/wav/
mv b3/dat/out/* b3/cont/wav/
mv b4/dat/out/* b4/cont/wav/

# create db S-files for each station (needs manual input)
# choose local, dont move or copy, local db ',,'
#for i in 'b2' 'b3' 'b4'; do
  #cd $i/cont
  #dirf wav/*.mseed
  #autoreg
  #cd ../../
#done

# create combined continuous db
# choose same as above and * to create new event
mkdir 01_cont
cd 01_cont
for i in 'b2' 'b3' 'b4'; do
  dirf ../$i/cont/wav/*.mseed
  autoreg
done

# combining events to one event file (continuous one hour events)
# database: ',,', starttime 201209, end of month, 150s probably a good start
associ
associ # and once more to get the last station

## Use EEV on dir db
# in dir you want to access it from, i.e. 02_events_raw:
# dirf ../01_cont
# mv filenr.lis cont.db.lis
# eev cont.db.lis

