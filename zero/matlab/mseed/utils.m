% utility functions for quickly generating segments


% makesegment shortcuts, assumes data has been loaded into variables:
% r, t, d and sdlag

gak2 = @()( makesegment('G3', 'GAK2', r, t, d, sdlag));
gak3 = @()( makesegment('G3', 'GAK2', r, t, d, sdlag));
gak4 = @()( makesegment('G3', 'GAK2', r, t, d, sdlag));
