#! /bin/bash
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-11-28
#
# Set up environment and run HYPOSAT

base=$(dirname $0)
export HYPOSAT_DATA="${base}/data"

hyposat

