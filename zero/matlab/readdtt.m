function [t, d, refs] = readdtt(f, complement_and_bitshift)
% Reads DTT format file

% complement_and_bitshift defaults to true and takes the second complement
% of d and shifts it down to 31 bits.

if (~exist('complement_and_bitshift', 'var'))
  complement_and_bitshift = true;
end

freq = 250;

t = [];
d = [];
refs = [];

ref = 0;
samples = 0;
pl = '';

T = 0;

fh = fopen (f);
while ~feof(fh)
  % Search for next reference
  l = fgetl(fh);
  while (length(l) <= 1 || l(1) ~= 'R')
    if (length(l) > 1)
      fprintf ('Discarded line while searching for next reference: %s\n', l);
    end

    l = fgetl (fh);
    if (feof(fh)), break; end
  end

  if (feof(fh)), break; end

  % Got referenceline in l
  R = sscanf (l, '%c,%u,%u,%lu,%u,%f%c,%f%c,%u');
  refs = [refs; R'];
  crc = R(10);
  L = R(2); % Length of samples
  
  if (T>R(4))
    fprintf ('[%d] [Warning] Time going backwards: %lu -> %lu\n', ref, T, R(4));
    disp (pl);
    disp (l);
  end
  pl = l;
  T = R(4); % Reference (microseconds)
  S = R(5); % Status

  ref = ref + 1;
  samples = samples + L;

  %fprintf ('Reading %d samples for reference %d at %d (status: %d)\n', ...
  %         L, ref, T, S);
  [batch, l] = fscanf (fh, '%u', L);

  if (l ~= L)
    disp ('[E] Number of samples does not match length of batch. Batch probably not completely downloaded.');
    disp ('[E] Stopping here.');
    return;
  end
    
  if (complement_and_bitshift)
    batch = bitshift (twos_comp (batch), -1);
  end

  d = cat(1, d, batch);

  MICROSECONDS_PER_SAMPLE = 1e6 / freq;
  t = cat(1,t, (T + (0:L-1)'*MICROSECONDS_PER_SAMPLE));
end

fprintf ('=> Read %d references with %d samples\n', ref, samples);

end
