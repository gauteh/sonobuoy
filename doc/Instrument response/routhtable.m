function [rtable, nr, nl, nj] = routhtable (T)

% Lagar og analyserer Routh-Hurwitz tabell.
% Author:  Gaute Hope <eg@gaute.vetsj.com>
% Created: 2010-04-16
%
% Usage: [rtable, nr, nl, nj] = routhtable (T)
%
%        T = Symbolsk transferfunksjon
%
%        rtable - Routh-Hurwitz tabell (symbolsk)
%        nr     - Poler i RHP
%        nl     - Poler i VHP
%        nj     - Poler på jw-akse

% hent transfer funksjon og lagre i T
% example 6.2, s 294
syms s e;

% T = 200 / poly2sym ([1 6 11 6 200], 's'); % standard
% T = 10 / poly2sym([1 2 3 6 5 3], 's'); % first column zero
% T = 10 / poly2sym ([1 7 6 42 8 56], 's'); % row of zeros
% T = 20 / poly2sym ([1 1 12 22 39 59 48 38 20], 's'); % row of zeros

[~, deng] = numden(T);
d = sym2poly (deng);

h = length (d);
w = round (h/2);

% sett opp table, initiellverdi 0
% TODO: gjer den ein breiare for å forenkle koden som bereknar
%       rekkene > 2
rtable = sym(zeros (h, w + 1));

% fyll inn første to rekker
for x=1:h
  y = mod(x + 1, 2) + 1;
  rtable (y, round (x / 2)) = d(x);
end

firstnull = false;
rownull = false;
nrownull = 0;

% berekn gjenståande rekker
for n=3:h
  for x=1:w
    a = -det ( [rtable(n-2, 1) rtable(n-2, x + 1);
               rtable(n-1, 1) rtable(n-1, x + 1)]) / rtable (n-1, 1);

    rtable (n,x) = a;
  end

  % sjekk om det er 0 i første kolonne aleine
  if rtable(n, 1) == 0
    nulls = 0;
    for j=2:w-1
      if rtable(n, j) == 0; nulls = nulls + 1; end
    end

    if nulls == (w-2)
      % heile rekka er null
      % erstatt i henhold til eksempel 6.4 s. 297
      rownull = true;
      nrownull = n;

      f = poly2sym (rtable (n-1, 1:w), 's');
      f = diff (f, 's');
      f = sym2poly (f);
      rtable (n,1:w) = [f 0];

    elseif nulls == 0
      rtable (n,1) = e; % erstatt med e = +- 0
      firstnull = true;
    end
  end
end

rtable = simplify(rtable(1:h, 1:w));

fprintf ('Routh-Hurwitz table:\n');
disp (rtable);

% analyse
fprintf ('Analyse: ')

% standard
if ~rownull && ~firstnull
  disp ('Standard table')
  n = 0;
  for y=2:h
    if double (rtable(y-1, 1) * rtable(y, 1)) < 0; n = n +1; end
  end

  nr = n;
  nl = (h-1) - n;
  nj = 0;

  fprintf ('Polar i RHP: %i\n', n);
  fprintf ('Polar i VHP: %i\n', nl);
  if n > 0; disp ('Systemet er ustabilt.');
  else      disp ('Systemet er stabilt.');
  end

elseif rownull
  disp ('Row of zeros');

  % polar på jw aksa
  nr = 0;
  for y=nrownull:h
    if double (rtable(y-1, 1) * rtable(y, 1)) < 0; nr = nr +1; end
  end

  nj = (h - nrownull +1) - (nr*2);
  fprintf ('Polar på jw-aksa: %i\n', nj);

  % sjekkar for fortegnsskift
  for y=2:(nrownull-1)
    if double (rtable(y-1, 1) * rtable(y, 1)) < 0; nr = nr +1; end
  end

  nl = h - nr - nj - 1;

  fprintf ('Polar i RHP: %i\n', nr);
  fprintf ('Polar i VHP: %i\n', nl);
  if nr > 0; disp ('Systemet er ustabilt.');
  else       disp ('Systemet er stabilt.');
  end

elseif firstnull
  disp ('Zero in first column');
  na = 0; % for positiv e
  nb = 0; % for negativ e
  for y=2:h
    % positiv e
    p = subs (rtable (y-1,1), e, 1);
    c = subs (rtable (y,1), e, 1);
    if p*c < 0; na = na +1; end

    % negativ e
    p = subs (rtable (y-1,1), e, -1);
    c = subs (rtable (y,1), e, -1);
    if p*c < 0; nb = nb +1; end
  end

  fprintf ('Fortengsskift (e = 0+): %i\n', na);
  fprintf ('Fortengsskift (e = 0-): %i\n', nb);

  fprintf ('Polar i RHP (e = 0+): %i\n', na);
  fprintf ('Polar i VHP (e = 0+): %i\n\n', (h-1) - na);

  fprintf ('Polar i RHP (e = 0-): %i\n', nb);
  fprintf ('Polar i VHP (e = 0-): %i\n', (h-1) - nb);

  nr = [na nb];
  nl = [((h-1) - na) ((h-1) - nb)];
  nj = 0;

  if (na > 0) || (nb > 0);  disp ('Systemet er ustabilt!');
  else                      disp ('Systemet er stabilt!');
  end
end

