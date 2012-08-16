function twos = twos_comp(d)
% Compute the 2's complement number for 32 bit long

MAX32 = hex2dec('7fffffff');
d(d>MAX32) = d(d>MAX32) - 2 * (MAX32 + 1);

twos = d;

end
