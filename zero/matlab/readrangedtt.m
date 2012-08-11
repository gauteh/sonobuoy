function [t, d, refs] = readrangedtt (r, path)
% Read all dtts in range r, optionally specify path (otherwise current
% directory is assumed)

if (~exist('path', 'var')), path = './'; end

t = [];
d = [];
refs = [];
for i = r
  n = sprintf ('%s/%d.DTT', path, i);
  fprintf('=> Reading: %s..\n', n);
  [nt, nd, nr] = readdtt(n);
  t = [t; nt];
  d = [d; nd];
  refs = [refs; nr];
end

end