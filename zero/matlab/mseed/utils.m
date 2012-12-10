% utility functions for quickly generating segments


% makesegment shortcuts, assumes data has been loaded into variables:


gak2 = @(r, t, d, sdlag)( makesegment('G3', 'GAK2', r, t, d, sdlag));
gak3 = @(r, t, d, sdlag)( makesegment('G3', 'GAK3', r, t, d, sdlag));
gak4 = @(r, t, d, sdlag)( makesegment('G3', 'GAK4', r, t, d, sdlag));
