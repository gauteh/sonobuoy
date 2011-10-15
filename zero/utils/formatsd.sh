#! /bin/bash

[[ "$#" -lt "1" ]] && ( echo "Specify partition to format."; exit 1 )

echo "Formatting: $1 to FAT32.."

mkdosfs -F 32 $1

