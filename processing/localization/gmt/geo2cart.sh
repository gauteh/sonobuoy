# Map boundaries
echo "Set up boundaries (convert to cartesian coordinates..)"
ibcaoregion="-R-45/53:49:1.4687/135/53:49:1.4687r -JS0/90/5805000"
mapproject geographic.asc ${ibcaoregion} -C -V > cartesian.asc


# Create color palette
if [ ! -f ibcao.cpt ]; then
  grd2cpt ${ibcaogrd} -Chaxby > ibcao.cpt
fi

# Create grid gradient
if [ ! -f gradient.grd ]; then
  echo "Create gradient.."
  grdgradient ${ibcaogrd} -Nt0.6 -A300 -Ggradient.grd -V
fi
