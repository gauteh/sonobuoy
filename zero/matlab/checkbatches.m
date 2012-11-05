function checkbatches (refs, t, d, fix)
% Check for errors in batches and optionally try to fix
%
% Attempt to detect errors of type:
% - Clock skews
% - Checksums
% - SD lag
% - Bad status and position

if ~exist ('fix', 'var'), fix = false; end

% sane time limits: 2012-08-31 - 2012-09-10
MIN_TIME = datenum2btime (datenum('2012-08-31'));
MAX_TIME = datenum2btime (datenum('2012-09-10'));


end
