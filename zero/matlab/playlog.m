function [ny, SFs, y, Fs] = playlog (t, v, nf)
% PLAYLOG   Playback log file
%
% Author: Gaute Hope <eg@gaute.vetsj.com> / 2011-09-28
%
% [ny, SFs, y, Fs] = playlog (t, v)
%
% t  = time stamps
% v  = data values
% nf = speed up playback nf times
% 
% Returns:
% ny = values normalized and centered around 2^23 interpolated
%      to standard frequency 8192.
%
% SFs = standard frequency 8192.
% y   = Normalized and centered data at original frequency
% Fs  = Original average frequency
%

if ~exist('nf', 'var')
  nf = 1;
end

%% Normalize the values vector
% 31 bit
y = v - mean(v);
y = y / max(abs(y));
%%

%% Find avarage frequency, t is in microseconds
Fs = 10^6 / mean(diff(t));

%% Interpolate to convert to desired frequency
SFs = 8192;

nt = linspace(t(1), t(end), length(t)*round(SFs/(nf*Fs)));
ny = interp1(t, y, nt);

%% Playback
fprintf ('Playing: %6.2f seconds (at %dx speed)..\n', length(t)/(nf*Fs), nf)

sound (ny, SFs);

end