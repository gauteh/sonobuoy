% Calculate noise

%data = load('One.log');

% d should be as returned from readdtt with complement and bitshift

noiserange = max(d) - min(d) +1;

fsr = 2^32 - 1;

snr  = 20 * log10(fsr / noiserange);
bits = log2(fsr / noiserange);

fprintf ('SNR (FSR / signal range): %4.7f dB\n', snr);
fprintf ('Bits: %1.2f\n', bits);

