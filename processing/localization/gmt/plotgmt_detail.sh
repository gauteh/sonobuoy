#! /bin/bash
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-11-29
#
# Plot event localization on IBCAO bathymetry using GMT
#

# load common
selfd=$(dirname $0)
. "${selfd}/common.sh"

out=$1
if [ $# -ne 1 ]; then
  out=ibcao_det.ps
fi

rm -f $out

echo "Output: $out"

# boundaries: Cartesian
xmin=0
ymin=-637312.873449
xmax=127935.668703
ymax=-477462.415701

# boundaries: Geographic
xmind=0
ymind="84:12"
xmaxd=15
ymaxd="85:30"

# Output image width
iw=20

# Setup region and projection
setup_reg_proj

# Create base map
GRID="-Ba1g1/a0.1g0.1WeSn" # grid
create_basemap

# Plot stations
plot_stations 12

# Plot quake
plot_quakes 10

# Add colorbar
add_colorbar

# Add legend
add_legend

