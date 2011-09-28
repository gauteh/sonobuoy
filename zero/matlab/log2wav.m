function log2wav (fin, fout, nf)
% LOG2WAV     Converts logfile fin to wavfile fout
%
% Author: Gaute Hope <eg@gaute.vetsj.com> / 2011-09-28
%
% fin  = input log file
% fout = output wav file
% nf   = speed (default 1)

if ~exist('nf', 'var')
  nf = 1;
end

%% Read data
d = csvread(fin);
t = d(:,1);
v = d(:,2);

%% Normalize the values vector
% 24 bit
y = v - 2^23;
y = y / max(abs(y));
%%

%% Find avarage frequency, t is in microseconds
Fs = 10^6 / mean(diff(t));

%% Interpolate to convert to desired frequency
SFs = 8192;

nt = linspace(t(1), t(end), length(t)*round(SFs/(nf*Fs)));
ny = interp1(t, y, nt);

%% Write
fprintf ('Writing %6.2f seconds (at %dx speed).. to %s\n', length(t)/(nf*Fs), nf, fout)
wavwrite (ny, SFs, 24, fout);

end