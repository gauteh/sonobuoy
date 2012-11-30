#! /bin/bash
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-11-29
#
# Plot event localization on IBCAO bathymetry using GMT
#

# Set up GMT defaults
gmtset PLOT_DEGREE_FORMAT -ddd:mm:ssF
gmtset D_FORMAT=%3.0f
gmtset ANOT_FONT_SIZE 10p

# IBCAO grid
ibcaogrd=~/ymse/maps/IBCAO-3rd-Edition/IBCAO_Ver3_RR_2012-03-16.grd

# Create color palette
#grd2cpt ${ibcaogrd} -Chaxby > ibcao.cpt

# Set up region
region="-R-20/80/79/90"
boundaries="-BSWnea.5f.1m/.5f.1m"
projection="-Js90/90/75/1:1"

misc="-K -V -X1i -Y1.5i"

rm ibcao.ps

# Create basemap
psbasemap $region $projection $boundaries $misc > ibcao.ps

# Add bathymetry
misc="-Cibcao.cpt -V -0"
#grdimage ${ibcaogrd} $region $projection $boundaries $misc >> ibcao.ps
grdview -Iimage.grd $ibcaogrd -Qi100 $projection $region -Cibcao.cpt -V -K > ibcao.ps

#misc="-O -K -Sa0.2 -W1p/0 -G0"
#psxy $region $projection $misc << END >> ibcao.ps
#80.0 80.0
#END

