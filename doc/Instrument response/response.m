% Calculates instrument response
r1 = 58.33e3;  % ohm
r2 = 41.67e3;  % ohm

c1 = 47e-6;    % farad
c2 = 330e-12;  % farad

%% Transfer function of Hydrophone decoupling, including first high pass and low
%  pass filters and OPA629.

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
H  = simplify(subs (H));

[n, d] = numden (H);

Tn = sym2poly(n);
Td = sym2poly(d);
T = tf(Tn, Td);


P = bodeoptions;
P.FreqUnits = 'Hz';
figure(1); clf('reset');
bodeplot (T, P);
title ('Bode plot of hydrophone decoupling with initial low pass and OPA629');

disp ('Hydrophone decoupling up to OPA629:')
zpk(T)
if isstable(T)
  disp ('System is stable.');
else
  disp ('System is unstable.');
end

%% opa1632 low pass
disp ('OPA1632. low passing:');
UOPA = zpk([], [-(1/(1047*10^-9))], 1/(1047*10^-9))
figure(4); clf('reset');
bodeplot (UOPA, P);
title ('Bode Diagram (OPA1632)');

%% upper low pass
disp ('Upper low pass (ADS1282, analog):');
U = zpk([], [-(1/(600*10*10^-9))], 1/(600*10*10^-9))
figure(2); clf('reset');
bodeplot (U, P);
title ('Bode Diagram (Upper low pass filter)');

%% final response
disp ('Final response:');
C = UOPA * U * T
figure(3); clf('reset');
bodeplot (C, P);
title ('Bode Diagram (Complete analog response)');  

% attenuation at nyquist
disp ('Attenuation at Nyquist:');
a = abs(freqresp(C, 512e3*2*pi))
adb = 20*log10(a)