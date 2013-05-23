% Transfer function for seismometer

zeros_ = [0 0 -90 -160.7 -3108]; % rad / s
poles_ = [-0.03852 + 0.03658i;
          -0.03852 - 0.03658i;
          -178;
          -135 + 160i;
          -135 - 160i;
          -671 + 1154i;
          -671 - 1154i]';
        
k_ = 3.080e5;
f0_ = 1;
S_ = 1201; % V s/m

T = zpk (zeros_, poles_, k_);

bode (T);