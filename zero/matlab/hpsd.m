
%% Calculate PSD for segments
h = spectrum.welch;

%% Load segments
ff = dir ('*.mseed');
%ff = ff(3:10);

nseg = size(ff,1);
nfreq = 129;

split = 3; % split hourly segments

fs = zeros(nfreq, nseg*split);

% load
for k=1:nseg
  fprintf ('reading: [%d/%d] %s..\n', k, nseg, ff(k).name);
  m = rdmseed(ff(k).name);
  d = c2p(double(cat(1,m.d)));
  jl = length(d)/split;
  for jj=1:split
    start = ((jj-1)*jl + 1);
    if (jj == split)
      td = d(start:end);
    else
      td = d(start:start + jl);
    end
    
    kpsd = psd (h, td, 'Fs', 250);
    fs(:,(k-1)*split + jj) = kpsd.Data;
    
    freq = kpsd.Frequencies;
  end
end

fprintf ('binning and plotting..\n');
yres = 500; ymin = 1; ymax = 14;
y = logspace (ymin, ymax, yres); % bin at these values
%fsn = zeros(yres, nfreq);
[fsn, xout] = hist(fs', y);

ydb = 10*log10(y);

fsn = fsn ./ (nseg*split);
clf('reset');
surface (freq, ydb, fsn, 'EdgeColor', 'none');
shading interp;

box on;
set(gca,'TickDir','out');


ylabel ('Power [mPa] [dB]');
xlabel ('Frequency [Hz]');
title (sprintf('PDF of #%d PSDs, hourly segments split into a total of #%d segments.', nseg, split*nseg));

hold on;

hh = [];

minpow = 10*log10(min(fs'));
maxpow = 10*log10(max(fs'));

z = max(fsn(:)) .* ones(length(minpow),1);

hh(1) = line(freq, minpow, z, 'Color', 'black', 'LineWidth', 2);

hh(2) = line(freq, maxpow, z, 'Color', 'black', 'LineWidth', 2);

avgpow = mean(fs,2);
avgpowdb = 10*log10(avgpow);
hh(3) = line (freq, avgpowdb, z, 'Color', 'blue', 'LineWidth', 2);

med = 10*log10(median (fs, 2));
hh(4) = line (freq, med, z, 'Color', 'blue', 'LineStyle', '--', 'LineWidth', 2);

legend (hh, 'Minimum', 'Maximum', 'Avarage PSD', 'Median PSD');

cmap = load('pdfpsd.cp');
colormap(cmap);

axis tight;
ya = ylim;
ylim ([ya(1) 120]);
