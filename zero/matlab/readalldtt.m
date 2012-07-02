function [t, d, refs] = readalldtt (f)
% Read all dtts specified by dir mask f, i.e. '*.DTT'

a = dir(f);
l = length(a);

% Sorting by id number
n = [];
for i=1:l
  n = [n sscanf(a(i).name, '%d.DTT')];
end

[~, i] = sort(n);
a = a(i);

t = [];
d = [];
refs = [];
for i = 1:l
  fprintf('=> Reading: %s..\n', a(i).name);
  [nt, nd, nr] = readdtt(a(i).name);
  t = [t; nt];
  d = [d; nd];
  refs = [refs; nr];
end

end
