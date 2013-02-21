#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-02-21
#
# Set up environment and run hyposearch

hyposearchdir="/home/gaute/dev/uib/hyposearch_3d_layered"

# run matlab
matlab -nodisplay -r "addpath('${hyposearchdir}'); run_hyposearch; exit"

# reset terminal
stty echo
stty icrnl
stty icanon


