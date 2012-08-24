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
hold on;

% IBCAO
ibcaotfw = '~/ymse/maps/IBCAO-3rd-Edition/IBCAO_V3_500m_RR.tfw';
ibcaotif = '~/ymse/maps/IBCAO-3rd-Edition/IBCAO_V3_500m_RR.tif';
R = worldfileread (ibcaotfw);
[ortho, cmap] = imread(ibcaotif);
geoshow (ortho(:, :, 1:3), R);


% Todo: This probably considers minutes to be decimals of degree
minutes = mod(lat, 100);
lat     = (lat - minutes) / 100;
minutes = minutes / 60;
lat     = lat + minutes;

minutes = mod(lon, 100);
lon = (lon - minutes) / 100;
minutes = minutes / 60;
lon = lon + minutes;

plotm (lat, lon, 'rx-');


end