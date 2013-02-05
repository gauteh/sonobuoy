#!/bin/bash
# GMT map example
PSFILE="lofoten-basin-1.ps"
PROJ=-JB6/65/69.0/74.0/14i
REG=-R351/65/25/74r
ibcaogrd=~/ymse/maps/IBCAO-3rd-Edition/IBCAO_Ver3_RR_2012-03-16.grd
BATHY=$ibcaogrd
gmtset DOTS_PR_INCH 600
gmtset PAPER_MEDIA A1
gmtset HEADER_FONT_SIZE 12p
gmtset LABEL_FONT_SIZE 12p
makecpt -Cocean -T-8000/0/250 > farge.cpt
grdgradient -A0 -Nt0.5 -Gimage.grd $BATHY 
grdview -Iimage.grd -Qi100 $PROJ $REG $BATHY -Cfarge.cpt -V -K > $PSFILE
grdcontour $BATHY $REG $PROJ -L-4000/-200 -C200 -A600 -A2000 -K -O -V >> $PSFILE
pscoast $REG $PROJ -Df -W1 -Ba2g2/a1g1WSne -Lf6/66.5/66.5/200      -N1 -U -G0 -O >> $PSFILE
convert -rotate 90 $PSFILE ./lofoten-basin-1.png
