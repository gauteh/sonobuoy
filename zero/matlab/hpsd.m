
%% Calculate PSD for segments
%h = spectrum.welch ('Gaussian', 250, 50) ;
h = spectrum.welch ('Rectangular', 512, 50);
%h = spectrum.mtm;


%% Load segments
%ff = dir ('*mseed');
%ff = ff(15:20);
%ff = ff(1:end-1); % seismo: '2012-09*BHZ'

nseg = size(ff,1);
%nfreq = 129;
nfreq = 257;

split = 30; % split hourly segments

fs = zeros(nfreq, nseg*split);

% load
figure(1); clf('reset');
for k=1:nseg
  fprintf ('reading and psd-ing: [%d/%d] %s..\n', k, nseg, ff(k).name);
  m = rdmseed(ff(k).name);
  
  %d = c2p(double(cat(1,m.d)));
  d = abs(transfertrillium(d));
  
  jl = floor(length(d)/split);
  for jj=1:split
    start = ((jj-1)*jl + 1);
    if (jj == split)
      td = d(start:end);
    else
      td = d(start:start + jl);
    end
    
    kpsd = psd (h, td, 'Fs', 250);
    plot(kpsd); drawnow;
    
    fs(:,(k-1)*split + jj) = kpsd.Data;
    
    freq = kpsd.Frequencies;
  end
end

% fs = fs ./ 1e6 ./ 1e6; % convert to pascal


%% Bin and plot
fprintf ('binning and plotting..\n');
yres = 400; ymin = -14; ymax = 3;
y = logspace (ymin, ymax, yres); % bin at these values
%fsn = zeros(yres, nfreq);

[fsn, xout] = hist(fs', y);

ydb = 10*log10(y);

fsn = fsn ./ (nseg*split);

% calculate widths
% width = [];
% width(1) = 2 * (y(2) - y(1));
% for i = 2:(length(y)-1)
%   width(i) = ( y(i+1) - y(i) ) + ( y(i) - y(i-1) );
% end
% width(length(y)) = 2*(y(end)-y(end-1));
% width = width' ./ 2;
% widths = repmat(width, 1, nfreq);
% figure(3); clf('reset');
% surface (freq, ydb, widths, 'EdgeColor', 'none');

% width = (y(2:end) - y(1:end-1))';
% 
% for k=1:length(freq)
%   fsn(2:end,k) = fsn(2:end,k) ./ width;
% end
% scale PDF with width of bin
%fsn = fsn ./ widths;
%fsn = fsn ./ max(fsn(:));

%fsn(fsn>0.03) = NaN;
figure(2); clf('reset');
ax = surface (freq, ydb, fsn, 'EdgeColor', 'none');
set(gca, 'XScale', 'log');
shading interp;

box on;
set(gca,'TickDir','out');


%ylabel ('Power [Pa^2/Hz] [dB]');
ylabel ('Power [dB]');
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
xlim([0 100]);
%ylim ([ya(1) 40]);
colorbar;
