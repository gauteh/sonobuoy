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
    if (lat_(1) == 0)
      lat_ = NaN;
    else
      lat_ = -lat_(1);
    end
  else
    lat_ = lat_(1);
  end
  
  if (lon_(2) == 'W')
    if (lon_(1) == 0)
      lon_ = NaN;
    else
      lon_ = -lon_(1);
    end
  else
    lon_ = lon_(1);
  end
  
  lat = [lat; lat_];
  lon = [lon; lon_];
  
end

end