#! /bin/bash

root=$(dirname $0)
export LD_LIBRARY_PATH=${root}/libmseed

${root}/dtttomseed $@

