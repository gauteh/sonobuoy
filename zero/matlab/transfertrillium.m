function [ dd ] = transfertrillium ( d )
% correct for instrument response and return displacement in nm

% Transfer function for seismometer
% from manual p. 51
zeros_ = [0, 0, -90, -160.7, -3108]; % rad / s
poles_ = [-0.03852 + 0.03658i;
          -0.03852 - 0.03658i;
          -178;
          -135 + 160i;
          -135 - 160i;
          -671 + 1154i;
          -671 - 1154i]';
        
k_ = 3.080e5;
f0_ = 1;
S_ = 1201; % V s/m

T = zpk (zeros_, poles_, k_);
% figure(1); clf('reset');
% bode (T, 0.1:0.01:(125.*2.*pi));
% title ('Transfer function');

% Steps:
% fft input data
% calculate freqresponse at frequencies using transfer function
% deconvolve with input data
% ifft output data

% remove mean
%d = d - mean(d);

Fs = 250;

nfft = 2.^nextpow2(length(d));
fd = fft(d, nfft);
% figure(2); clf('reset');

f = Fs/2 .* linspace (-1,1,nfft);

% plot(f, fftshift(abs(fd)));
% title ('Original signal (FFT)');
% set(gca, 'YScale', 'log');
% set(gca, 'XScale', 'log');


resp = freqresp(T, fftshift(f).*2*pi);
resp = resp(:);

% figure(3); clf('reset');
% plot (f, fftshift(abs(resp)));
% title ('Freq resp of instrument (at freqs of original signal)');
% set(gca, 'YScale', 'log');
% set(gca, 'XScale', 'log');

nd = fd ./ resp;
% figure (4); clf('reset');
% 
% title ('Corrected signal (FFT)');
% set(gca, 'YScale', 'log');
% set(gca, 'XScale', 'log');


dd = abs(ifft(nd));

% highpass at 0.01 Hz
 fcut = .1;
% poles = 6;
% [b, a] = butter (poles, fcut / (Fs/2), 'high');
% fprintf ('=> filtering..\n');
% dd = abs(filtfilt (b, a, dd));

%dd = detrend(dd, 'linear', 250);

nfft = 2.^nextpow2(length(d));
fdd = fft(dd, nfft);
% 
% hold on;
% 
% plot(f, fftshift(abs(fdd)), 'r');
% 
% plot (f, fftshift(abs(fd)), 'g');
% plot (f, fftshift(abs(nd)));
% legend (sprintf('Corrected signal (filtered %g Hz)', fcut), 'Original signal', 'Corrected signal');

dd = dd ./ S_; % convert to s/m

% figure (5); clf('reset');
% subplot (2,1,1);
% plot (dd);
% title ('Final signal');
% ylabel ('m/s');
% 
% subplot (2,1,2);
% plot (d);
% title ('Original signal');
% ylabel ('counts');

end


