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

% Common defaults
network = 'GB';
channel = 'BNR';

% Constants
files_per_hour    = ceil (250 * 60 * 60 / (40 * 1024)); % files necessary to load for each hour
samples_per_hour  = 250 * 60 * 60;
samples_per_file  = 40 * 1024;
samples_per_batch = 1024;
batches_per_file  = 40;

%% Work through range
% superfluos samples from previous file
prevt = [];
prevd = [];
prevr = [];
prevsdlag = [];

k = 1;
while k < length(range)
  %% Load one hour of samples
  % Calculate files needed to get new full hour
  needsamples = samples_per_hour - length(prevt);
  needfiles   = ceil (needsamples / samples_per_file);
  
  kend = min([k+needfiles-1 length(range)]);
  fprintf ('==> Loading one hour of samples: %d to %d..\n', k, kend);
  nf   = kend - k;
  thisrange = range(k:kend);
  
  % pre allocate vars
  t = nan(samples_per_file * nf, 1);
  d = nan(samples_per_file * nf, 1);
  r = nan(batches_per_file * nf, 11);
  sdlag = nan(nf, 1);
  
  % load from previous file
  [b, ~] = size(prevr);
  if (~isempty(prevt))
    fprintf ('==> Loaded %d of superfluos samples from previous collection.\n', length(prevt));
    t(1:length(prevt)) = prevt;
    d(1:length(prevd)) = prevd;
    
    r(1:b, :) = prevr(:,:);
    sdlag(1:length(prevsdlag)) = prevsdlag;
  end
  
  sk = max([length(prevt) 1]); % index of samples arrays
  sr = max([b 1]); % index of reference arrays
  ss = max([length(prevsdlag) 1]); % index of file arrays
  
  % clear prev
  prevt = [];
  prevd = [];
  prevr = [];
  prevsdlag = [];
  
  % load files
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
  
  %% Put superfluos samples in next 'prev'
  if (length(t) > needsamples)
    prevt = t(needsamples+1:end);
    prevd = d(needsamples+1:end);
    
    b = ceil(length(prevt) / samples_per_batch);
    f = ceil(length(prevt) / samples_per_file); % should never be more than 1
    assert (f <= 1, 'Loaded more than one extra file.');
    
    prevr = r(end-b:end, :);
    prevsdlag = sdlag(end);
    
    % remove superfluos samples from this collection
    t = t(1:needsamples);
    d = d(1:needsamples);
    r = r(1:b);
    sdlag = sdlag(1:end - 1);
    
    fprintf ('==> %d samples scheduled for next file..\n', length(prevt));
  end
  
  %% Check
  checkbatches (r, t, d, true);
  
  %% Write mseed file
  batches = getbatches (t, d);
  
  MsWriteMseed (batches, d', network, station, location, channel, 250.0, 1.0, 250.0);
  
  k = kend +1;
end

end
