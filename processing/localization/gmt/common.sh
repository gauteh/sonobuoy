#! /bin/bash
#
# common.sh: Common settings and functions for all gmt scripts
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-02-05
#

# Output size
width=800
height=900

# GMT defaults
gmtset PAGE_ORIENTATION=landscape
gmtset PAPER_MEDIA=Custom_${width}x${height}

gmtset ELLIPSOID=WGS-84
gmtset MAP_SCALE_FACTOR=1

# IBCAO grid
ibcaogrd=~/ymse/maps/IBCAO-3rd-Edition/IBCAO_Ver3_RR_2012-03-16.grd
data=~/dev/gautebuoy/processing/localization/gmt


# Calculate image height, use after defining x and y boundaries
function get_ih() {
  # ih = iw * yd/xd
  echo "scale=10; ( ${ymax} - ${ymin} ) * ${iw} / ( ${xmax} - ${xmin} )" | bc
}

# Set up region and projections from cartesian and geographic bounds,
# assuming x and y boundaries have been defined for both bounds.
function setup_reg_proj() {
  # Cartesian
  REGc="-R${xmin}/${ymin}/${xmax}/${ymax}r"
  PROJc="-JX${iw}/${ih}"

  # Geographic: Polar Stereographic, Standard latitude = 75N
  REGg="-R${xmind}/${ymind}/${xmaxd}/${ymaxd}r"
  PROJg="-JS0/90/75/${iw}"
}

# Set up map, assuming all variables have been set by
# calling script
function create_basemap() {
  # Get image height
  ih="$(get_ih)"

  # Create shaded relief
  echo "Create shaded relief.."
  grdimage ${ibcaogrd} -I${data}/gradient.grd ${REGc} ${PROJc} -C${data}/ibcao.cpt -P -K -V  >> $out

  echo "Create map boundary and grid lines.."
  pscoast ${REGg} ${PROJg} ${GRID} -Df -W -O -K -P >> $out
}

function plot_stations() {
  echo "Plotting stations.."

  # Triangles: stations.cpt define color of station
  psxy ${PROJg} ${REGg} -O stations.d -St${1}p -C${data}/stations.cpt -K -P >> $out

  # Label stations
  #pstext ${PROJg} ${REGg} -O stations.t -Gwhite -K -P >> $out

}

function plot_quakes() {
  echo "Plotting quakes.."

  psxy ${PROJg} ${REGg} -O quakes.d -Sa${1}p -C${data}/quakes.cpt -K -P >> $out

  # plot error ellipsis
  psxy ${PROJg} ${REGg} -O quakes.e.d -W- -SE1p -C${data}/quakes.cpt -K -P >> $out
}

function add_colorbar() {
  echo "Adding colorbar.."
  psscale -D600p/250p/500p/30p -O -C${data}/ibcao.cpt -I -P -E -B1000:Depth:/:m: -K >> $out
}

function add_legend() {
  echo "Adding legend.."

  # figure out number of jobs
  noj=$(wc -l quakes.d | tr -d ' quakes.d')
  cmperj=1.5

  gmtset ANNOT_FONT_SIZE_PRIMARY=9
  h=$(echo "scale=10; 3 + $cmperj * $noj" | bc) # calculate height from number of jobs
  #y=$(echo "scale=10; ($ih - $h)" | bc) # align to the top
  y=0                                    # align to the bottom
  w=6
  x=$(echo "scale=10; ($iw - $w)" | bc)  # align to the right

  pslegend -R-${iw}/-${ih}/${iw}/${ih}r -JX${iw}/${ih} -Gazure1 -C0.15c/0.15c -Dx$x/$y/$w/$h/BL -F -P -K -O < legend.txt >> $out
}

