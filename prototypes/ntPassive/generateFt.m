function ft = generateFt(fc, n, fs)
   ft = zeros(n, 1);
   hz_pr_bin = fs / n;
   ft(ceil(1:fc / hz_pr_bin)) = 1;
   ft((ceil((fs - fc) / hz_pr_bin) + 1):end) = 1;
end
