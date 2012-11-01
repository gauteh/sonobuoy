function t = datenum2btime (tu)
% Converts MATLAB time to buoy time (hp)

  offset = datenum('1970-01-01');
  msecsperday = 60 * 60 * 24 * 1e6 ;
  t = (tu - offset) * msecsperday;

end