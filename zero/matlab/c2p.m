function p = c2p (d)
% Convert count to pressure level, outputs micro pascal
% (uPa). Expects input in bitshifted, complemented.

gain_db = -167; % Hydrophone, dB re V / uPa

gain    = 10^(gain_db/20);

p = c2v (d) ./ gain;

end
