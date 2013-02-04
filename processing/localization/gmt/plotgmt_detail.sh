#! /bin/bash
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-11-29
#
# Plot event localization on IBCAO bathymetry using GMT
#

# Set up GMT defaults
#gmtset PLOT_DEGREE_FORMAT -ddd:mm:ssF
#gmtset D_FORMAT=%3.0f
#gmtset ANOT_FONT_SIZE 10p

out=$1
if [ $# -ne 1 ]; then
  out=ibcao_det.ps
fi

echo "Output: $out"

width=800
height=700
gmtset PAGE_ORIENTATION=landscape
gmtset PAPER_MEDIA=Custom_${width}x${height}

gmtset ELLIPSOID=WGS-84
gmtset MAP_SCALE_FACTOR=1

# IBCAO grid
ibcaogrd=~/ymse/maps/IBCAO-3rd-Edition/IBCAO_Ver3_RR_2012-03-16.grd
data=~/dev/gautebuoy/processing/localization/gmt


# boundaries: read from cartesian.asc
xmin=0
ymin=-637312.873449
xmax=127935.668703
ymax=-477462.415701

# calc height and width of image
# ih = iw * yd/xd
iw=20
ih=$(echo \( ${ymax} - ${ymin} \) / \( ${xmax} - ${xmin} \) \* ${iw} | bc)
echo ih=$ih

xmind=0
ymind="84:12"
xmaxd=15
ymaxd="85:30"

rm $out

export HDF5_DISABLE_VERSION_CHECK=1

# Create shaded relief
echo "Create shaded relief.."

grdimage ${ibcaogrd} -I${data}/gradient.grd -R${xmin}/${ymin}/${xmax}/${ymax}r -JX${iw}/25 -C${data}/ibcao.cpt  -K -V \
  >> $out

# plot stations using meters
cat cartesian.asc | psxy -M -J -R -O -W1 -Gblack -K >> $out
./geo2cart.sh stations.d | psxy -M -J -R -O -W1 -K >> $out



psbasemap -Ba1g1/a0.1g0.1WeSn -R${xmind}/${ymind}/${xmaxd}/${ymaxd}r -JS0/90/$iw -K -O  >> $out


# Add coast and map box
#pscoast -R${xmind}/${ymind}/${xmaxd}/${ymaxd}r -JS0/90/90/$iw -Ba1g1/a0.1g0.1WeSn -Df -W -O -K >> $out

# stations
#psxy -J -R -O stations.d -St2p -Gyellow -K  >> $out
#pstext -J -R -O stations.t -Gblack -K >> $out

# quakes
#psxy -J -R -O quakes.d -Sa3p -Gred -K >> $out

# add color scale
#psscale -D600p/250p/500p/30p -O -C${data}/ibcao.cpt -I -E -B1000:Depth:/:m: -K >> $out

#misc="-O -K -Sa0.2 -W1p/0 -G0"
#psxy $region $projection $misc << END >> ibcao.ps
#80.0 80.0
#END

