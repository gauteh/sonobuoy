
%% Calculate PSD for segments
%h = spectrum.welch ('Gaussian', 250, 50) ;
h = spectrum.welch ('Rectangular', 512, 50);
%h = spectrum.mtm;


%% Load segments
ff = dir ('*.mseed');
%ff = ff(4:20);

nseg = size(ff,1);
%nfreq = 129;
nfreq = 257;

split = 30; % split hourly segments

fs = zeros(nfreq, nseg*split);

% load
for k=1:nseg
  fprintf ('reading and psd-ing: [%d/%d] %s..\n', k, nseg, ff(k).name);
  m = rdmseed(ff(k).name);
  d = c2p(double(cat(1,m.d)));
  jl = floor(length(d)/split);
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
fs = fs ./ 1e6; % convert to pascal

%% Bin and plot
fprintf ('binning and plotting..\n');
yres = 400; ymin = -4; ymax = 5;
y = logspace (ymin, ymax, yres); % bin at these values
%fsn = zeros(yres, nfreq);

[fsn, xout] = hist(fs', y);

ydb = 10*log10(y);

fsn = fsn ./ (nseg*split);

%fsn(fsn>0.03) = NaN;
clf('reset');
ax = surface (freq, ydb, fsn, 'EdgeColor', 'none');
set(gca, 'XScale', 'log');
shading interp;

box on;
set(gca,'TickDir','out');


ylabel ('Power [Pa] [dB]');
xlabel ('Frequency [Hz]');
title (sprintf('%s - PDF of #%d PSDs, hourly segments split into a total of #%d segments.', m(1).ChannelFullName, nseg, split*nseg));

hold on;

hh = [];

minpow = 10*log10(min(fs'));
maxpow = 10*log10(max(fs'));

z = max(fsn(:)) .* ones(length(minpow),1);
% 
% hh(1) = line(freq, minpow, z, 'Color', 'blue', 'LineWidth', 2);
% 
% hh(2) = line(freq, maxpow, z, 'Color', 'red', 'LineWidth', 2);
% 
% % avgpow = mean(fs,2);
% % avgpowdb = 10*log10(avgpow);
% % hh(3) = line (freq, avgpowdb, z, 'Color', 'blue', 'LineWidth', 2);
% 
% % med = 10*log10(median (fs, 2));
% % hh(4) = line (freq, med, z, 'Color', 'blue', 'LineStyle', '--', 'LineWidth', 2);
% 
% % trace surface along maximum
% [~, mmi] = max(fsn,[], 1);
% yy = ydb(mmi);
% hh(3) = line (freq, yy, z, 'Color', 'black', 'LineStyle', '--', 'LineWidth', 2);
% 
% legend (hh, 'Minimum', 'Maximum', 'Mode');

cmap = load('pdfpsd.cp');
colormap(cmap);

axis tight;
ya = ylim;
%ylim ([ya(1) 40]);
colorbar;
