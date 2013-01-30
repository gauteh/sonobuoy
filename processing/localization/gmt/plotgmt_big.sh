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

# IBCAO grid
ibcaogrd=~/ymse/maps/IBCAO-3rd-Edition/IBCAO_Ver3_RR_2012-03-16.grd

width=800
height=700
gmtset PAGE_ORIENTATION=landscape
gmtset PAPER_MEDIA=Custom_${width}x${height}

# Create color palette
if [ ! -f ibcao.cpt ]; then
  grd2cpt ${ibcaogrd} -Chaxby > ibcao.cpt
fi

# Create grid gradient
if [ ! -f gradient.grd ]; then
  echo "Create gradient.."
  grdgradient ${ibcaogrd} -Nt0.6 -A300 -Ggradient.grd -V
fi

# boundaries: read from cartesian.asc
xmin=-1568009.00342
ymin=-1568009.00342
xmax=1568009.00342
ymax=1568009.00342

xmind=-45
ymind=70
xmaxd=135
ymaxd=70

rm ibcao_big.ps

# Create shaded relief
echo "Create shaded relief.."
grdimage ${ibcaogrd} -Igradient.grd -R${xmin}/${ymin}/${xmax}/${ymax}r -JX20/20 -Cibcao.cpt -P -K -V > ibcao_big.ps


# Add coast and map box
export HDF5_DISABLE_VERSION_CHECK=1
pscoast -R${xmind}/${ymind}/${xmaxd}/${ymaxd}r -JS0/90/20 -Ba20g20/a5g5WeSn -Dh -W -O -K >> ibcao_big.ps

# add color scale
psscale -D600p/250p/500p/30p -O -Cibcao.cpt -I -E -B1000:Depth:/:m: -K >> ibcao_big.ps

#misc="-O -K -Sa0.2 -W1p/0 -G0"
#psxy $region $projection $misc << END >> ibcao.ps
#80.0 80.0
#END

