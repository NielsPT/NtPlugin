function b = windowMethod(fc, n, fs)
   f_resp = generateFt(fc, n, fs);
   t_resp = real(fft(f_resp) / n);
   % t_resp = [t_resp(2:end); t_resp(1)];
   b_full = [t_resp(floor(end / 2) + 1:end); t_resp(1:floor(end / 2))];
   b = b_full .* hanning(length(b_full));
end
