function [lat, lon] = gettrack (refs)
% Plot track from list of refs
[m, n] = size(refs);

lat = [];
lon = [];
for i=1:m
  R = refs(i,:);
  lat_ = R(6:7);
  lon_ = R(8:9);
  
  if (lat_(2) == 'S')
    lat_ = -lat_(1);
  else
    lat_ = lat_(1);
  end
  
  if (lon_(2) == 'W')
    lon_ = -lon_(1);
  else
    lon_ = lon_(1);
  end
  
  lat = [lat; lat_];
  lon = [lon; lon_];
  
end

end