function makesegment (location, station, r, t, d, sdlag)
% Make a miniSEED of segment like makehourly does for an hour,
% takes a trace, does not check.
%
% Location: G1 for first drift, G3 for third drift
% Station:  GAK1 for buoy 1

% Creates:
% - MiniSEED file for this segment
% - Info file with: ID numbers of input files
% - Track file for hour

% Common
network = 'GB';
channel = 'BNR';

% Constants
samples_per_hour  = 250 * 60 * 60;
samples_per_file  = 40 * 1024;
samples_per_batch = 1024;
batches_per_file  = 40;


%% Write mseed file
batches = getbatches (t, d);
fname = MsWriteMseed (batches, d', network, station, location, channel, 250.0, 1.0, 250.0);

% Write IDs used to info file with sdlag info
ids = unique(r(:,12));
fname_base = fname(1:end-6); % strip out .mseed
fname_info = sprintf ('%s.ids', fname_base);
f_info = fopen (fname_info, 'w');
for iid=1:length(ids)
  % format: id,sdlag
  fwrite (f_info, sprintf('%d,%d\n', ids(iid), sdlag(iid)));
end
fclose (f_info);

% Write out references
fname_track = sprintf ('%s.refs', fname_base);
f_track = fopen (fname_track, 'w');
for rr=r'
  % format:
  % - file id
  % - no
  % - ref
  % - status
  % - lat
  % - N/S
  % - lon
  % - E/W
  % - checksum
  % - checksum pass

  fwrite (f_track, sprintf ('%d,%d,%lu,%d,%f,%c,%f,%c,%lu,%d\n', rr(12), rr(3), ...
    rr(4), rr(5), rr(6), rr(7), rr(8), rr(9), rr(10), rr(11)));
end
fclose (f_track);

end
