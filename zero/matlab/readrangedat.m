function [t, d, refs, sdlag] = readrangedat (r, path)
% Read all dats in range r, optionally specify path (otherwise current
% directory is assumed)

if (~exist('path', 'var')), path = './'; end

wd = cd;
cd (path);

t = [];
d = [];
refs = [];
sdlag = [];
for i = r
  [nt, nd, nr, nsd] = readdat(i);
  t = [t; nt];
  d = [d; nd];
  refs = [refs; nr];
  sdlag = [sdlag; nsd];
end

cd (wd);

end