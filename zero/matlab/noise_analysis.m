% Calculate noise
% Expects input from ADS1282 with inputs shorted (only containing noise)

data = load('One.log');

% Last bit is redundant sign bit

d = bitshift(twos_comp(data(:,2)),-1);

noiserange = max(d) - min(d);

fsr = 2^31 - 1;

snr = 20 * log10(fsr / noiserange);
bits = log2(fsr / noiserange);
fprintf ('SNR (FSR / range): %4.7f dB\n', snr);
fprintf ('Bits: %1.2f\n', bits);

