function [t, d, refs] = readrangedat (r, path)
% Read all dats in range r, optionally specify path (otherwise current
% directory is assumed)

if (~exist('path', 'var')), path = './'; end

wd = cd;
cd (path);

t = [];
d = [];
refs = [];
for i = r
  [nt, nd, nr] = readdat(i);
  t = [t; nt];
  d = [d; nd];
  refs = [refs; nr];
end

cd (wd);

end