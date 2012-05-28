% Calculates instrument response
r1 = 58.33e3;  % ohm
r2 = 41.67e3;  % ohm

c1 = 47e-6;    % farad
c2 = 380e-12;  % farad

syms s i1 i2 vs;
zc1 = 1/(s*c1);
zr1 = r1;
zc2 = 1/(s*c2);
zr2 = r2;

[i1, i2] = solve ('vs = i1*(zc1 + zr1) + (i1 - i2)*zr2', ...
                  '0  = (i2 - i1)*zr2 + i2*zc2', 'i1', 'i2');
%i2 = solve ('vs = i2*(zc1 + zr1 + zr2)', 'i2');

vo = i2*sym('zc2');
H  = vo/vs;
pretty (simplify(H))
H  = simplify(subs (H));

pretty(H);

[n, d] = numden (H);

Tn = sym2poly(n);
Td = sym2poly(d);
T = tf(Tn, Td)


P = bodeoptions;
P.FreqUnits = 'Hz';
figure(1); clf('reset');
bodeplot (T, P);

if isstable(T)
  disp ('Stable.');
else
  disp ('Unstable.');
end

zpk(T)

% opa1632 low pass
UOPA = zpk([], [-(1/(1047*10^-9))], 1/(1047*10^-9))
figure(4); clf('reset');
bodeplot (UOPA, P);
title ('Bode Diagram (OPA1632)');

% upper low pass
U = zpk([], [-(1/(600*10*10^-9))], 1/(600*10*10^-9));
figure(2); clf('reset');
bodeplot (U, P);
title ('Bode Diagram (Upper low pass filter)');

% final response
C = UOPA * U * T;
figure(3); clf('reset');
bodeplot (C, P);
title ('Bode Diagram (Complete analog response)');  
