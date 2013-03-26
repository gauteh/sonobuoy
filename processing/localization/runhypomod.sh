#! /bin/bash
#
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2013-03-12
#
# Set up environment and run HYPOMOD

base=$(dirname $0)
export HYPOSAT_DATA="${base}/data"

#/home/gaute/dokument/UiB/Master/Prosessering/Lokalisering/HYPOSAT/hyposat.4_4b/bin_l/hypomod $@

hypomod $@

