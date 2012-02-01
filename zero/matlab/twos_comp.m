function [twos, sign] = twos_comp(d)
% compute the 2's complement number for 32 bit long
% Based on Tal Levinger,   13/07/04

lut_2 = 2.^(0:1:31) -1;

len = length (d);
twos = nan (len, 1);
carry = nan (len, 1);

for j=1:len
  x = d(j);
  if bitget(x, 32) == 0,
      sign = 1;
      twos(j) = x;
  else
      x = bitget(x, 1:32);
      sign = -1;
      x = ~x;

      carry(j) = 1;
      for i=1:16,
          y = x(i) + carry(j);
          if (y == 2),
              carry(i) = 1;
              x(i) = 0;
          else
              x(i) = y;
              carry(j) = 0;
          end
      end
      twos(j) = sign * sum(x .* lut_2);
  end
end
