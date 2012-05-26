% Calculates instrument response
r1 = 58.33e3;
r2 = 41.67e3;

c1 = 47e-6;
c2 = 380e-12;

syms s i1 i2 vs;
zc1 = 1/(s*c1);
zr1 = r1;
zc2 = 1/(s*c2);
zr2 = r2;

[i1, i2] = solve ('vs = i1*(zc1 + zr1) + (i1 - i2)*zr2', ...
           '0  = (i2 - i1)*zr2 + i2*zc2', 'i1', 'i2');

vo = i2*sym('zc2');
H  = vo/vs;
pretty (simplify(H))
H  = subs (H);

pretty(H);

[n, d] = numden (H);

Tn = sym2poly(n);
Td = sym2poly(d);
T = tf(Tn, Td)


P = bodeoptions;
P.FreqUnits = 'Hz';

bodeplot (T, P);
