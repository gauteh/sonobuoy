function traces ()
% Plot traces and map of five buoys

names = [ {'One'}, {'Two'}, {'Three'}, {'Four'}, {'Five'} ];
cols  = [ 'b', 'k', 'c', 'r', 'g' ];

figure(2); clf('reset');

for i=1:5
  %subplot (5, 1, i);

  f = names{i};
  
  a = dir(sprintf('%s/*.DTT', f));
  l = length(a);

  % Sorting by id number
  n = [];
  for j=1:l
    n = [n sscanf(a(j).name, '%d.DTT')];
  end

  [~, ii] = sort(n);
  a = a(ii);

  % Read two latest ids
  t = [];
  d = [];
  for j=length(a)-2:length(a)
    fprintf('=> Reading: %s/%s..\n', names{i}, a(j).name);
    [nt, nd] = readdtt(sprintf('%s/%s', names{i}, a(j).name));
    t = [t; nt];
    d = [d; nd];
  end
  
  d = d + 2^30 * i;
  
  % Plot
  plot (d, cols(i));

  hold on;
  drawnow;
end

title ('Traces');
legend (names);
ylim([0 2^30 * (length(names)+1)]);


end
