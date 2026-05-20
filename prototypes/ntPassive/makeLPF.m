function [b, a] = makeLPF(fs, fc, Q)
   % Computes coefficients for hi shelving Equalizer% b = [b0 b1 b2] Nominator
   % a = [a0 a1 a2] Denominator
   % fs = sampling frequency
   % fc = Center EQ frequence
   % Q = bandwidth of EQ
   % Based on the Audio-EQ-Cookbook:
   % http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
   w0 = 2 * pi * fc ./ fs;
   alpha = sin(w0) ./ (2 * Q);

   b0 = (1 - cos(w0)) ./ 2;
   b1 = 1 - cos(w0);
   b2 = (1 - cos(w0)) ./ 2;
   a0 = 1 + alpha;
   a1 = -2 * cos(w0);
   a2 = 1 - alpha;

   a1 = a1 ./ a0;
   a2 = a2 ./ a0;

   b0 = b0 ./ a0;
   b1 = b1 ./ a0;
   b2 = b2 ./ a0;

   a0 = 1;

   a = [a0 a1 a2];
   b = [b0 b1 b2];
end
