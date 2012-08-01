#! /bin/bash
#
# Gaute Hope <eg@gaute.vetsj.com> 2012-08-01
#
# Back up log

d=$(date +"%Y-%m-%d")

src="/home/gaute/dev/sonobuoy/zero/log/"
dest="/home/gaute/dev/sonobuoy/zero/log/backup/$d"

echo "Backing up to: $dest.."

mkdir -p $dest

cp -ru "${src}"{One,Two,Three,Four,Five,*.log*} "${dest}"

echo "Done."

