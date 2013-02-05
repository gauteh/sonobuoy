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
  pscoast ${REGg} ${PROJg} -Ba1g1/a0.1g0.1WeSn -Df -W -O -K -P >> $out
}

function plot_stations() {
  echo "Plotting stations.."
  psxy ${PROJg} ${REGg} -O stations.d -St2p -Gyellow -K -P >> $out

}

function add_colorbar() {
  echo "Adding colorbar.."
  psscale -D600p/250p/500p/30p -O -C${data}/ibcao.cpt -I -P -E -B1000:Depth:/:m: -K >> $out
}

