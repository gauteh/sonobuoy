function tim = rt_trace(buoy, fig)
% rt_trace (buoy, figure): checks for new data and updates plot with trace
% continuosly for specified buoy. Specifiy which figure to use.
%
% Returns the timer which checks for new data, stop/delete this to stop the
% script.

lastid          = 0;
n_traces        = 5;
show_magnitudes = true;
show_legend     = false;
seconds_axis    = true;
figure (fig);

filelength = 40960; % 40 references of 1024 long batches
partlyread = false;
newread    = 0;

t = [];
d = [];

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
    
    newrange = n(end)-4:n(end);
    
    % read new
    [nt, nd] = readrangedtt (newrange, mpath);
    t = nt;
    d = nd;
    
    if seconds_axis
      t = linspace (0, length(t)/250, length(t));
    end
    
    plotdtt (t, d, show_magnitudes, false, gca(fig), seconds_axis);
    if ~show_legend
      legend off;
    end


    if length(t) >= (n_traces * filelength)
      lastid = n(end);
      partlyread = false;
       
    else
      partlyread = true;
    end
    
    if seconds_axis
      xlabel ('Seconds');
    else
      xlabel ('Time [us]');
    end
    title (sprintf('Continuous trace: %s (last id: %d)', buoy, n(end)));
    ylabel ('Amplitude');
  end
end

tim  = timer ('ExecutionMode', 'fixedDelay', 'BusyMode', 'drop', 'Period', 10.0, 'TimerFcn', @updateplot_fcn, 'StartDelay', 2);
start (tim);

disp ('rt_trace: Timer started, use delete (tim) to stop and remove the returned timer. Or delete(timerfindall()).');

end



