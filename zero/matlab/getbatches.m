function [batches] = getbatches (t, d)
% [batches = timetobatches (t, d)
%
% Prepares a time and data series for conversion to miniSEED by generating
% a batches parameter matrix.

samplerate  = 250.0;
batchlength = 1024;

n_batches = floor(length(d) / batchlength);

batches = zeros (n_batches, 4);

% start time
starts = [0:n_batches-1] * batchlength + 1;
batches(:,1) = int64(t(starts'));

% end time
batches(:,2) = int64(t(starts' + batchlength -1));

% batch length
batches(:,3) = int32(batchlength);

% quality
batches(:,4) = 0; % default 0: undecided

end