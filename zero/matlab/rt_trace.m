function t = rt_trace(buoy, fig)
% rt_trace (buoy, figure): checks for new data and updates plot with trace
% continuosly for specified buoy. Specifiy which figure to use.
%
% Returns the timer which checks for new data, stop this to stop the
% script.

lastid          = 0;
n_traces        = 5;
show_magnitudes = true;
show_legend     = false;
seconds_axis    = true;
figure (fig);

function updateplot_fcn (obj, event)
  % Check for new batch and plot

  mpath  = sprintf ('/home/gaute/dev/gautebuoy/zero/log/%s/', buoy);
  files = sprintf ('/home/gaute/dev/gautebuoy/zero/log/%s/*.DTT', buoy);

  a = dir(files);
  l = length(a);

  % Sorting by id number
  n = [];
  for i=1:l
    n = [n sscanf(a(i).name, '%d.DTT')];
  end

  [n, i] = sort(n);

  if (length(n)) < 1
    disp ('No data.');
    return;
  end

  if (n(end) > lastid)
    disp ('New data..');

    [t, d] = readrangedtt (n(end-n_traces+1:end), mpath);
    if seconds_axis
      t = linspace (0, length(t)/250, length(t));
    end
    plotdtt (t, d, show_magnitudes, false, gca(fig), seconds_axis);
    if ~show_legend
      legend off;
    end


    if length(t) == (n_traces * 40960)
      lastid = n(end);
    end
  end
end

t  = timer ('ExecutionMode', 'fixedDelay', 'BusyMode', 'drop', 'Period', 5.0, 'TimerFcn', @updateplot_fcn, 'StartDelay', 2);
start (t);

disp ('rt_trace: Timer started, use delete (t) to stop and remove timer.');

end



