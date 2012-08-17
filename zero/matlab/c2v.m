function v = c2v (c)
% Calculates voltage value from count, expects bitshifted and complemented
% counts.

maxv = 2.5;  % range +/- 2.5 V
maxc = 2^31; % complemented (last bit should be cleared by readdtt, out of FS bit)

v = c ./ maxc .* maxv;

end

