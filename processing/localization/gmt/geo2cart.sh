#! /bin/bash

# Convert geographic coordinates to UPS cartesian coordinates
gmtset ELLIPSOID=WGS-84
gmtset MAP_SCALE_FACTOR=1
ibcaoregion="-R-45/53:49:1.4687/135/53:49:1.4687r -JS0/90/75/5805000"
mapproject $1 ${ibcaoregion} -C


