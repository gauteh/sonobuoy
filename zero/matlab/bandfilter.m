function [passed, blocked] = bandfilter (d, low, high)
% Filter d for frequencies between low and high and return passed and
% blocked

n = length (d);
N = 2^nextpow2 (n);

freq = 250; % Sample rate

assert (low >= 0 && low <= (freq /2 ), 'Bad low frequency');
assert (high >= 0 && high <= (freq / 2), 'Bad high frequency');

Ft = fft (d, N);
F  = freq * [ - N/2 : (N / 2 - 1) ] / N;

lown  = floor((low / freq) * N);
highn = ceil((high / freq) * N);

Blocked = zeros (N, 1);

Blocked((lown+1):(highn)) = Ft(lown+1:highn);
Blocked(end-(highn):end-(lown)) = Ft(end-(highn):end-(lown));

Passed = Ft - Blocked;

figure (3);
clf ('reset');
subplot (3, 1, 1);
plot (F, fftshift(abs(Ft)));
title ('Original');
xlabel ('Frequency (Hz)');

subplot (3, 1, 2);
plot (F, fftshift (abs(Passed)));
title ('Passed');
xlabel ('Frequency (Hz)');

subplot (3, 1, 3);
plot (F, fftshift (abs(Blocked)));
title ('Blocked');
xlabel ('Frequency (Hz)');

passed = ifft (Passed, N);
passed = passed(1:n);
blocked = ifft (Blocked, N);
blocked = blocked(1:n);

end
