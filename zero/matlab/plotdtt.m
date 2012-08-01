function plotdtt (t, d, magnitudes, fig)
% Converts and shifts dtt file before plotting
% Returns timestamps and data values in the range of [-2^30-1 2^30-1]
%
% Arguments:
% t and d as returned by readdtt
% magnitudes, plot some common magnitude thresholds
% fig, figure to plot on, if less than 0 use default

assert (length(d) > 1, 'No data.');

if (~exist('magnitudes', 'var')), magnitudes = false; end
if (~exist('fig', 'var')), figure(1); fig = gca(1); end

if (fig > 0)
  cla(fig, 'reset');
end

plot (fig, t,d, '-');

if magnitudes
  hold(fig, 'on');

  % Plot magnitude one eq. at 10 km distance, centered around mean of data
  % mb = 0 @ 10km:  2.5 uBar => 2.5 * 100 000 uPa
  % mb = 1 @ 10km: 25.0 uBar => 25  * 100 000 uPa

  % Hydrophone sensitivity (AQ-18): -167 dB re 1 uPa [V]
  % Full scale voltage range: 5 V (after pre-amp and buffer)
  Ha = -167; % dB [V/uPa]
  reWater = 1; % uPa

  p0  = 2.5 *100000; % uPa (mb = 0)
  dBre0 = 20 * log10(p0/reWater);

  p1  = 25 * 100000; % uPa (mb = 1)
  dBre1 = 20 * log10(p1/reWater);

  p2  = 250 * 100000; % uPa (mb = 2)
  dBre2 = 20 * log10(p2/reWater);

  p3  = 250 * 100000 * (10^2) / (50^2); % uPa (mb = 2 @ 50km)
  dBre3 = 20 * log10(p3/reWater);

  A0  = 10^((Ha + dBre0) / 20);
  A1  = 10^((Ha + dBre1) / 20);
  A2  = 10^((Ha + dBre2) / 20);
  A3  = 10^((Ha + dBre3) / 20);

  t0 = min(t);
  t1 = max(t);
  
  plot (fig, [t0 t1],  [1 1] * (A3/5*(2^30-1)) + mean(d), 'c');
  plot (fig, [t0 t1],  [1 1] * (A2/5*(2^30-1)) + mean(d), 'k');
  plot (fig, [t0 t1],  [1 1] * (A1/5*(2^30-1)) + mean(d), 'r');
  plot (fig, [t0 t1],  [1 1] * (A0/5*(2^30-1)) + mean(d), 'g');

  plot (fig, [t0 t1], -[1 1] * (A3/5*(2^30-1)) + mean(d), 'c');
  plot (fig, [t0 t1], -[1 1] * (A2/5*(2^30-1)) + mean(d), 'k');
  plot (fig, [t0 t1], -[1 1] * (A1/5*(2^30-1)) + mean(d), 'r');
  plot (fig, [t0 t1], -[1 1] * (A0/5*(2^30-1)) + mean(d), 'g');

  %legend (fig, 'Data', 'mb = 2 @ 50km threshold', 'mb = 2 @ 10km threshold', 'mb = 1 @ 10km threshold', 'mb = 0 @ 10km threshold');
end

  function txt = mydatatip (obj, event)
    pos = event.Position;
    txt = {['Value: ', num2str(pos(2))], ['Time:  ', datestr(btime2datenum(pos(1)))]};
  end

h = datacursormode (get(fig, 'Parent'));
set (h, 'UpdateFcn', @mydatatip, 'SnapToDataVertex', 'on');


end