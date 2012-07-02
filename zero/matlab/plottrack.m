function [lat, lon] = plottrack (refs)
% plots track

[lat, lon] = gettrack (refs);

latlim = [55 90];
lonlim = [-180 180];

ax = worldmap (latlim, lonlim);

land = shaperead ('landareas', 'UseGeoCoords', true);
c = load ('coast');
geoshow (ax, land);
plotm (c.lat, c.long);

plotm (lat/100, lon/100, 'rx-');


end