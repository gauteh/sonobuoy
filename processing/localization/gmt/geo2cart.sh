# Map boundaries
echo "Set up boundaries (convert to cartesian coordinates..)"
ibcaoregion="-R-45/53:49:1.4687/135/53:49:1.4687r -JS0/90/5805000"
mapproject geographic.asc ${ibcaoregion} -C -V > cartesian.asc
