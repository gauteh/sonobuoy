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
  out=ibcao_big.ps
fi

echo "Output: $out"

# IBCAO grid
ibcaogrd=~/ymse/maps/IBCAO-3rd-Edition/IBCAO_Ver3_RR_2012-03-16.grd
data=~/dev/gautebuoy/processing/localization/gmt

width=800
height=700
gmtset PAGE_ORIENTATION=landscape
gmtset PAPER_MEDIA=Custom_${width}x${height}

# boundaries: read from cartesian.asc
xmin=-1568009.00342
ymin=-1568009.00342
xmax=1568009.00342
ymax=1568009.00342

xmind=-45
ymind=70
xmaxd=135
ymaxd=70

rm $out 

# Create shaded relief
echo "Create shaded relief.."
grdimage ${ibcaogrd} -I${data}/gradient.grd -R${xmin}/${ymin}/${xmax}/${ymax}r -JX20/20 -C${data}/ibcao.cpt -P -K -V > $out


# Add coast and map box
export HDF5_DISABLE_VERSION_CHECK=1
pscoast -R${xmind}/${ymind}/${xmaxd}/${ymaxd}r -JS0/90/20 -Ba20g20/a5g5wESn -Dh -W -O -K >> $out
psxy -J -R -O quakes.d -Sa1c -Gred >> $out

# add color scale
psscale -D600p/250p/500p/30p -O -C${data}/ibcao.cpt -I -E -B1000:Depth:/:m: -K >> $out

#misc="-O -K -Sa0.2 -W1p/0 -G0"
#psxy $region $projection $misc << END >> ibcao.ps
#80.0 80.0
#END

