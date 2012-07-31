function [lat, lon] = plottrack (refs, fig)
% plots track
if (~exist('fig', 'var')), fig = 3; end

if (fig > 0)
  figure (fig);
  clf('reset');
end

[lat, lon] = gettrack (refs);

latlim = [82 87];
lonlim = [-20 20];

ax = worldmap (latlim, lonlim);

land = shaperead ('landareas', 'UseGeoCoords', true);
geoshow (ax, land);

% Todo: This probably considers minutes to be decimals of degree
minutes = mod(lat, 100) - mod(lat, 1);
lat = (lat - minutes) / 100;
lat = lat + minutes / 60;

minutes = mod(lon, 100) - mod(lon, 1);
lon = (lon - minutes) / 100;
lon = lon + minutes / 60;

plotm (lat, lon, 'rx-');


end