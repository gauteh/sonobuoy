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
  out=ibcao_big.ps
fi
rm -f $out

echo "Output: $out"


# boundaries: Cartesian
xmin=-1568009.00342
ymin=-1568009.00342
xmax=1568009.00342
ymax=1568009.00342

# boundaries: Geographic
xmind=-45
ymind=70
xmaxd=135
ymaxd=70

# Output image width
iw=20

# Setup region and projection
setup_reg_proj

# Create base map
create_basemap

# Plot stations
plot_stations

# Add colorbar
add_colorbar

