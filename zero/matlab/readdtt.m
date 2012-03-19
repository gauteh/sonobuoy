function [t, d] = readdtt(f)

data = load (f);
t = data(:,1);
d = bitshift(twos_comp(data(:,2)), -1);

end