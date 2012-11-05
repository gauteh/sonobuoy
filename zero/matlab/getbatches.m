function [batches] = getbatches (t, d)
% [batches] = getbatches (t, d)
%
% Prepares a time and data series for conversion to miniSEED by generating
% a batches parameter matrix.

samplerate  = 250.0;
batchlength = 1024;

n_batches = floor(length(d) / batchlength);

batches = zeros (n_batches, 3);

% start time
starts = [0:n_batches-1] * batchlength + 1;
batches(:,1) = int64(t(starts'));

% batch length
batches(:,2) = int32(batchlength);

% quality
batches(:,3) = 0; % default 0: undecided

end