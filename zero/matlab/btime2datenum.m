function t = btime2datenum (tu)
% Converts buoy time to MATLAB time

  offset = datenum('1970-01-01');
  msecsperday = 60 * 60 * 24 * 1e6 ;
  t = (tu / msecsperday) + offset;

end