function [F, Ft] = plotfreq (data, removemean)
% Do fourier transform of data d (signed input), normalize and plot
% Returns:
% F  = Frequency scale
% Ft = Fourier space values
%
% Arguments:
% data (unprocessed from dtt file, in twos complement, 32bits)
% removemean, set to true to remove DC component

if (~exist('removemean', 'var')), removemean = false; end

d = bitshift(twos_comp (data(:,2)), -1);

if removemean, d = d - mean(d); end % Remove DC component

n = length (d);
N = 2^nextpow2 (n);

Ft = fft (d, N)/n;

freq = 250; % Sample rate

X = 2*abs (fftshift (Ft));
%X = X / max(X);          % Normalize

F = freq * [ -N/2 : (N/2 -1)] / N;

figure (2);
clf ('reset');
plot (F, X);
title ('Frequency domain');
xlabel ('Frequency [Hz]');
ylabel ('Amplitude');

end
