function [F, Ft] = frequency_analysis (d)
% Do fourier transform of data d (signed input), normalize and plot
% Returns:
% F  = Frequency scale
% Ft = Fourier space values

n = length (d);
N = 2^nextpow2 (n);

Ft = fft (d, N);

freq = 250; % Sample rate

X = abs (fftshift (Ft));
X = X / max(X);          % Normalize

F = freq * [ -N/2 : (N/2 -1)] / N;

figure (2);
clf ('reset');
plot (F, X);
title ('Frequency domain');
xlabel ('Frequency [Hz]');
ylabel ('Amplitude (normalized)');
ylim ([-.1 1.1]);

end
