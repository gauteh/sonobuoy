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

rm ibcao.ps

# Create shaded relief
echo "Create shaded relief.."
grdimage ${ibcaogrd} -Igradient.grd -R${xmin}/${ymin}/${xmax}/${ymax}r -JX10/10 -Cibcao.cpt -P -K -V > ibcao.ps


# Add coast and map box
pscoast -R${xmind}/${ymind}/${xmaxd}/${ymaxd}r -JS0/90/10 -Ba20g20/a5g5WeSn -Dh -W -O -K >> ibcao.ps

# Create basemap
#psbasemap $region $projection $boundaries $misc > ibcao.ps

# Add bathymetry
#misc="-Cibcao.cpt -V -0"
#grdimage ${ibcaogrd} $region $projection $boundaries $misc >> ibcao.ps
#grdview -Iimage.grd $ibcaogrd -Qi100 $projection $region -Cibcao.cpt -V -K > ibcao.ps

#misc="-O -K -Sa0.2 -W1p/0 -G0"
#psxy $region $projection $misc << END >> ibcao.ps
#80.0 80.0
#END

