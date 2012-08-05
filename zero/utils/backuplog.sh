#! /bin/bash
#
# Gaute Hope <eg@gaute.vetsj.com> 2012-08-01
#
# Back up log

d=$(date +"%Y-%m-%d")

root=$(dirname $0)/..

if [[ -f "${root}/zero.py" ]]; then
  echo "Found dir."
else
  echo "Cannot find Zero root directory (looking for zero.py)."
  exit 1
fi

src="${root}/log/"
dest="${root}/log/backup/$d"

echo "Backing up to: $dest.."

mkdir -p $dest

cp -ru "${src}"{One,Two,Three,Four,Five,*.log*} "${dest}"

echo "Done."

