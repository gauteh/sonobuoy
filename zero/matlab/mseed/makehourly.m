function makehourly (location, station, range)
% Make hourly miniSEED files from a range DAT files
%
% Location: G1 for first drift, G3 for third drift
% Station:  GAK1 for buoy 1
%
% Objectives:
% - Detect timing errors and stop
% - Save track and status along with each hourly miniSEED file
%

% Common
network = 'GB';
channel = 'BNR';

% Constants
files_per_hour    = ceil (250 * 60 * 60 / (40 * 1024)); % files necessary to load for each hour
samples_per_hour  = 250 * 60 * 60;
samples_per_file  = 40 * 1024;
samples_per_batch = 1024;
batches_per_file  = 40;
files_buffer      = 10; % extra files to load to make sure we get to the next hour

%% Work through range
% superfluos samples from previous file
prevt = [];
prevd = [];
prevr = [];
prevsdlag = [];

k = 1;
while (k < length(range) || ~isempty(prevt))
  %% Load one hour of samples
  % Calculate files needed to get new full hour
  needsamples = samples_per_hour - length(prevt) + files_buffer * samples_per_file;
  needfiles   = ceil (needsamples / samples_per_file);
  
  kend = min([k+needfiles-1 length(range)]);
  if (k > length(range))
    fprintf ('==> End of range reached, writing out samples of non-full hour..\n');
    nf = 0;
  else
    fprintf ('==> Loading one hour (+ buffer) of samples, index: %d to %d..\n', k, kend);
    nf   = kend - k;
    thisrange = range(k:kend);   
  end

  [b, ~] = size(prevr);
  
  % pre allocate vars
  t = nan(samples_per_file * nf + length(prevt), 1);
  d = nan(samples_per_file * nf + length(prevd), 1);
  r = nan(batches_per_file * nf + b, 11);
  sdlag = nan(nf + length(prevsdlag), 1);
  
  % load from previous file
  if (~isempty(prevt))
    fprintf ('==> Loaded %d of remaining samples from previous collection.\n', length(prevt));
    t(1:length(prevt)) = prevt;
    d(1:length(prevd)) = prevd;
    
    r(1:b, :) = prevr(:,:);
    sdlag(1:length(prevsdlag)) = prevsdlag;
  end
  
  sk = max([length(prevt) 1]);      % index of samples arrays
  sr = max([b 1]);                  % index of reference arrays
  ss = max([length(prevsdlag) 1]);  % index of file arrays
  
  % save sizes for checkbatches
  nprevr = b;
  nprevt = length(prevt);
  
  [nr, ~] = size(r);
  assert (b<=nr, 'Less total number of references than from remaining previous references.');
  
  % clear prev
  prevt = [];
  prevd = [];
  prevr = [];
  prevsdlag = [];
  
  % load files
  if (k < length(range))
    for i=thisrange
      [tt, dd, rr, ssdlag] = readdat (i);
      t(sk:sk+samples_per_file-1) = tt;
      d(sk:sk+samples_per_file-1) = dd;
      r(sr:sr+batches_per_file-1,:) = rr;
      sdlag(ss:ss+1) = ssdlag;

      sk = sk + samples_per_file;
      sr = sr + batches_per_file;
      ss = ss + 1;
    end
  end
  
  %% Check
  [fixed, t, d] = checkbatches (r, t, d, true, nprevr, nprevt);
  
  %% Find rollover to next hour
  [~, ~, ~, hours, ~, ~] = datevec(btime2datenum (t));
  starth = hours(1);
  endi = find(hours > starth, 1 );
  
  if (isempty(endi))
    endi = length(t);
    fprintf ('==> No hour rollover in range, collecting remaining samples in this file\n');
  else
    fprintf ('==> Found hour rollover at sample %d: %d -> %d.\n', endi, starth, hours(endi));
  end
  
  %% Put superfluos samples in next 'prev'
  if (length(t) > endi)
    prevt = t(endi+1:end);
    prevd = d(endi+1:end);
    
    b = ceil(length(prevt) / samples_per_batch);
    f = ceil(length(prevt) / samples_per_file); % should never be more than 1
    
    partial_ref = (b * samples_per_batch) - length(prevt);
    
    prevr = r(end-(b-1):end, :);
    prevsdlag = sdlag(end-f:end);
    
    testref = prevr(1,4) + partial_ref / 250 * 1e6;
    %assert (testref == prevt(1), 'Previous: Reference and time series does not match.');
    [mr, ~] = size(prevr);
    assert (b == mr, 'Previous: Bad number of references.');
    
    % remove superfluos samples from this collection
    t = t(1:endi);
    d = d(1:endi);
    r = r(1:end-b,:);
    sdlag = sdlag(1:end-f);
    
    fprintf ('==> %d samples scheduled for next file..\n', length(prevt));
  end
  
  %% Write mseed file
  batches = getbatches (t, d);
  
  MsWriteMseed (batches, d', network, station, location, channel, 250.0, 1.0, 250.0);
  
  k = kend +1;
end

end
