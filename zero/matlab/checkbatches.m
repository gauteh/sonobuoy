function fixed = checkbatches (refs, t, d, fix)
% Check for errors in batches and optionally try to fix
%
% Attempt to detect errors of type:
% - Clock skews
% - Checksums
% - SD lag
% - Bad status and position

fprintf ('==> Checking %d samples..\n', length(t));

if ~exist ('fix', 'var'), fix = false; end

% Constants
samples_per_batch = 1024;

% sane time limits: 2012-08-31 - 2012-09-10
MIN_TIME = datenum2btime (datenum('2012-08-31'));
MAX_TIME = datenum2btime (datenum('2012-09-10'));

maxtdiff = 1e6; % us
positive_fix_threshold = 1e6;     % us
negative_fix_threshold = 0.5e6;   % us

%% Sanity checks
[nr, ~] = size (refs);

%% Simple time checks
deltat = 1 / 250.0;

tdiff = diff(t);
figure(1); clf('reset');
hist(tdiff, 100);
title ('Time differences');

figure(2); clf('reset');
plot(t); hold on;
title ('Time');

% Plot refs as stars
x = (0:(nr - 1)) * samples_per_batch;
plot(x, refs(:,4), 'r*')

%% Find zero time deltas (indicating duplicates)
nz = find (tdiff == 0);
if (~isempty(nz)) 
  fprintf ('==> Found zero time delta at: %d\n', nz);
  
  plot (nz, t(nz), 'gx');
end

%% Find negative time deltas
nn = find (tdiff < 0);
if (~isempty(nn)) 
  for i=nn'
    fprintf ('==> Found negative time delta at: %d (delta: %f)\n', i, tdiff(i));
  end
  
  plot (nn, t(nn), 'gx');
end

%% Find large positive time deltas
np = find (tdiff > maxtdiff);
if (~isempty(np))
  for i=np'
    fprintf ('==> Found large postive time delta at: %d (delta: %f)\n', i, tdiff(i));
  end
  
  plot (np, t(np), 'kx');
end

%% Fix



pause


end
