#! /bin/bash

[[ "$#" -lt "1" ]] && ( echo "Not enough arguments."; exit 1 )

echo "Formatting: $1.."

mkdosfs -F 16 $1

