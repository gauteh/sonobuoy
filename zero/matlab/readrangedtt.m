function [t, d, refs] = readrangedtt (r)
% Read all dtts in range r

t = [];
d = [];
refs = [];
for i = r
  n = sprintf ('%d.DTT', i);
  fprintf('=> Reading: %s..\n', n);
  [nt, nd, nr] = readdtt(n);
  t = [t; nt];
  d = [d; nd];
  refs = [refs; nr];
end

end