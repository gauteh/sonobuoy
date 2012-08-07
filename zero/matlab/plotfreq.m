function [F, Ft] = plotfreq (d, removemean, fig)
% Do fourier transform of data d (signed input), normalize and plot
% Returns:
% F  = Frequency scale
% Ft = Fourier space values
%
% Arguments:
% d data samples, as returned by readdtt
% removemean, set to true to remove DC component

if (~exist('removemean', 'var')), removemean = false; end
if (~exist('fig', 'var')), fig = 3; end

if removemean, d = d - mean(d); end % Remove DC component

n = length (d);
N = 2^nextpow2 (n);

Ft = fft (d, N)/n;

freq = 250; % Sample rate

X = 2*abs (fftshift (Ft));
%X = X / max(X);          % Normalize

F = freq * linspace (-.5, .5, N);

figure (fig);
clf ('reset');
plot (F, X);
title ('Frequency components');
xlabel ('Frequency [Hz]');
ylabel ('Amplitude');

end
