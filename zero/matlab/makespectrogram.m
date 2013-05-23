function makespectrogram (varargin)
n = size(varargin,2);

%% Find min and max t for reference
gmint = NaN;
gmaxt = NaN;

for k=1:n
  m = varargin{k};

  t = cat(1, m.t);
  gmint = min([gmint t(1)]);
  gmaxt = max([gmaxt t(end)]);
end


%% Compute spectrogram
for k=1:n
  pm = n;
  pn = 2;

  m = varargin{k};
  fprintf ('== stream: %s\n', m(1).ChannelFullName);

  if (strcmp(strtrim(m(1).StationIdentifierCode), 'GAKS'))
    seismo = true;
  else
    seismo = false;
  end

  t = cat(1, m.t);
  d = cat(1, m.d);
  d = double(d);

  if ~seismo
    d = c2p(d); % convert counts to pressure [mPa]
  else
    d = abs(transfertrillium (d));
  end

  Fs = 250; % Hz
  F  = 0:1:125;
  win = 250; % samples (one sec)
  overlap = 125; % samples overlap

  % filter
  fcut = 8;
  if ~isnan(fcut)
    poles = 8;
    [b, a] = butter (poles, fcut / (Fs/2), 'high');
    fprintf ('=> filtering..\n');
    d = real(filter (b, a, d));
  end

  fprintf ('=> creating and plotting spectrogram..\n');
  subplot (pm, pn, (k-1)*2 + 1);
  [y,f,T,p] = spectrogram(d, win,overlap,F,Fs,'yaxis');
  %surf(T,f,10*log10(abs(p)),'EdgeColor','none');

  % Realign T to ref time frame
  mint = t(1); maxt = t(end);
  Tr = T + (mint - gmint)*24*60*60;

  surf(Tr,f,20.*log10(abs(p)),'EdgeColor','none');
  axis xy; axis tight; view (0,90);

  colormap (jet);
  %xlabel('Time [s]');
  ylabel('Frequency (Hz)');
  xlim ([0 ((gmaxt-gmint)*24*60*60)]);
  if ~isnan(fcut)
    title (sprintf('Station %s (filtered above %g Hz) (dB)', m(1).ChannelFullName, fcut));
  else
    title (sprintf('Station %s (unfiltered) (dB)', m(1).ChannelFullName));
  end
  a = xlim;
  if (k == n)
    xlabel ('Time [s]');
  end

  fprintf ('=> plotting trace..\n');
  subplot (pm, pn, (k-1)*2 + 2);
  tt = linspace(Tr(1), Tr(end), length(d));
  %tt = tt./Fs;
  plot (tt, d);
  xlim (a);
  if (k == n)
    xlabel ('Time [s]');
  end

  if ~seismo
    ylabel ('Amplitude [uPa]');
  else
    ylabel ('Amplitude [counts]');
  end
end

% require suplabel from fileexchange
if ~isnan(fcut)
  suplabel (sprintf('Spectrograms between %s and %s (filtered above %g Hz)', datestr(gmint), datestr(gmaxt), fcut), 't');
else
  suplabel (sprintf('Spectrograms between %s and %s (unfiltered)', datestr(gmint), datestr(gmaxt)), 't');
end

fprintf ('== done.\n');

end
