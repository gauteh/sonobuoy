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

event=$(basename ${PWD})
if [ ! -e ${event} ]; then
  echo "mhs: not run from event dir."
  exit 1
fi

hyposearchdir=~/dev/uib/hyposearch_3d_layered

# load common
selfd=$(dirname $0)/gmt
. "${selfd}/common.sh"

out=$1
if [ $# -ne 1 ]; then
  out=job_hs01
fi

mkdir -p $out

jobtemplate=${out##job_}
echo "mhs: template: ${jobtemplate}"

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

echo "mhs: create job file from template..: ${jobtemplate}"
cp ${hyposearchdir}/hs_job_template_${jobtemplate}.m ${out}/hs_job.m

# update event
sed -e "s|@T_EVENT@|${event}|"  -i ${out}/hs_job.m
sed -e "s|@T_JOB@|${out}|"      -i ${out}/hs_job.m

echo "mhs: done"

