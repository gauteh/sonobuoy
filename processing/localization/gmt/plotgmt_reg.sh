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
  out=ibcao_reg.ps
fi
rm -f $out

echo "Output: $out"

# boundaries: Cartesian
xmin=-300876.42094
ymin=-826651.17258
xmax=282400.457416
ymax=-336551.759676

# boundaries: Geographic
xmind=-20
ymind=82
xmaxd=40
ymaxd=86

# Output image width
iw=20


# Setup region and projection
setup_reg_proj

# Create base map
GRID="-Ba5g5/a2g2WeSn" # grid
create_basemap

# Plot stations
plot_stations 6

# Plot quake
plot_quakes 6

# Add colorbar
add_colorbar

# Add legend
add_legend

# Close ps
close_ps

