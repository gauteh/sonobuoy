function [lat, lon] = getlatlon (l)
% Return latitude and longitude from reference
R = sscanf (l, '%c,%u,%u,%lu,%u,%u,%u,%u');
lat = R(6);
lon = R(7);
crc = R(8);

north = lat & hex2num('8000')
east  = lon & hex2num('8000')

bitset (lat, 16, 0);
bitset (lon, 16, 0);

latd = lat / 1e4

end