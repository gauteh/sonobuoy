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

maxtdiff = 0.9e6; % us

%% Sanity checks
[nr, ~] = size (refs);

%% Simple time checks
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
    fprintf ('==> Found postive time delta at: %d (delta: %f)\n', i, tdiff(i));
  end
  
  plot (np, t(np), 'kx');
end

%% Fix
if (fix)
  %% Try to find jump pairs (only small jumps)
  maxjump = 2e6; % us
  jump_range_threshold = 10 * 1024; % samples to search for a opposite jump (+/- direction)
  estimate_range       = 11 * 1024; % samples before and after to use to estimate new time values
  pairs = nan(0,2);
  
  % discard big jumps
  nn = nn(abs(tdiff(nn))<maxjump);
  np = np(abs(tdiff(np))<maxjump);
  
  NN = repmat (nn, 1, length(np));
  NP = repmat (np, 1, length(nn))';
  
  D = abs(NN - NP);
  [DI,  i] = sort(D);
  
  % Find matches and sort by best matches
  [mx, nx] = size(D);
  for ii=1:nx
    [~, mi] = min(DI(:,ii));
    
    pp = [nn(i(mi,ii)) np(ii)];
    
    if abs(pp(2) - pp(1)) < jump_range_threshold
      pairs = [pairs; pp];
    end
  end
  
  
  for pp=pairs'
    fprintf ('==> [fixing] Jump pair matched: %d -> %d (duration: %d samples)\n', pp(1), pp(2), abs(pp(1) - pp(2)));
    
    % Discard and recalculate time values for range
    s = min(pp);
    e = max(pp);
    
    leading_times_s   = max([(s - estimate_range) 1]);
    following_times_e = min([(e + estimate_range) length(t)]);
    
    x = [leading_times_s:s e+1:following_times_e];
    y = t(x);
    
    dx = s:e;
    ny = interp1(x, y, dx, 'linear');
    
    t(dx) = ny;
  end
  
  %% Replot
  figure(3); clf('reset');
  plot(t); hold on;
  title ('Fixed time');

  % Plot refs as stars
  x = (0:(nr - 1)) * samples_per_batch;
  plot(x, refs(:,4), 'r*')
  
  tdiff = diff(t);
  
  %% Rescan for large or uncaught jumps (outliers)
  % Find large negative time deltas
  nn = find (tdiff < 0);
  if (~isempty(nn)) 
    for i=nn'
      fprintf ('==> Found unhandled negative time delta at: %d (delta: %f)\n', i, tdiff(i));
    end

    plot (nn, t(nn), 'gx');
  end

  % Find large positive time deltas
  np = find (tdiff > maxtdiff);
  if (~isempty(np))
    for i=np'
      fprintf ('==> Found unhandled postive time delta at: %d (delta: %f)\n', i, tdiff(i));
    end

    plot (np, t(np), 'kx');
  end
  
  
end



pause


end
