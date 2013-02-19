#! /bin/bash
#
# makehyposearch.sh: Prepare job for hyposearch
#
# author: Gaute Hope <eg@gaute.vetsj.com> / 2013-02-18
#
# Extract grid data from IBCAO of specified region
# Prepare station and phase readings
#
# Run from event directory
#

echo "mhs: make hyposearch job"

# load common
selfd=$(dirname $0)/gmt
. "${selfd}/common.sh"

out=$1
if [ $# -ne 1 ]; then
  out=job_hs01
fi

mkdir -p $out

echo "mhs: output: $out.."

# boundaries: same as detailed

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

# check if plotevent_ibcao.py has been run to extract coordinates
if [ ! -e "map/stations.d" ]; then
  echo "error: plotevent_ibcao.py must be run first to extract station and quake coordinates from previous jobs."
  exit 1
fi

# cut out part of IBCAO grid
echo "mhs: cutting out region of IBCAO.."
grdcut $ibcaogrd -G"${out}/ibcao_det.grd" -R${xmin}/${ymin}/${xmax}/${ymax}r > /dev/null 2>&1

# write out region coordinates (UPS)
echo -e "${xmin} ${ymin}\n${xmax} ${ymax}\n" > ${out}/ibcao_det.coor

# write out station coordinates
echo "mhs: writing out station coordinates.."
cp map/stations.d ${out}/
cat map/stations.d | ${selfd}/geo2cart.sh > ${out}/stations.coor

echo "mhs: writing out hyposat solutions for quakes.."
cp map/quakes.d ${out}/
cat map/quakes.d | ${selfd}/geo2cart.sh > ${out}/quakes.coor

echo "mhs: load phases.."
${selfd}/../readphases.py > ${out}/phases.tt

echo "mhs: done"

