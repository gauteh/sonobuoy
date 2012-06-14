function v = c2v (c)
% Calculates voltage value from count, expects bitshifted and complemented
% counts.

maxv = 2.5;  % range +/- 2.5 V
maxc = 2^30; % one bit is shifted out, one bit is used for twos complement

v = c ./ maxc .* maxv;

end

