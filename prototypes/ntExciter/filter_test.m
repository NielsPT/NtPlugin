fs = 48000;
t = 0.1;
n = t * fs;
n_ax = 0:n - 1;
f_ax = n_ax / t;

f = 1000;
q = 100;

c = calculateCoeffs(fs, f, q);

z = 1 / (2 * q);
a = 1 - exp(-z * pi / (1 - z ^ 2) .^ 0.5);

x = [1; zeros(n - 1, 1)];
y = zeros(n, 1);
xn2_bpf = 0;
xn1_bpf = 0;
yn2_bpf = 0;
yn1_bpf = 0;
for i = 1:n
  % apply band pass filter
  yn0_bpf = (c(1, 1) .* x(i) ...
    + c(1, 2) .* xn2_bpf ...
    - c(2, 1) .* yn1_bpf ...
    - c(2, 2) .* yn2_bpf);

  % update states
  xn2_bpf = xn1_bpf;
  xn1_bpf = x(i);
  yn2_bpf = yn1_bpf;
  yn1_bpf = yn0_bpf;
  y(i) = yn0_bpf / q;
end

fig = figure();
semilogx(f_ax, 20 * log10(abs(fft(y))));
grid on
xlim([20 20e3]);
saveas(fig, "out/filter_test.png")

function c = calculateCoeffs(fs, f, q)
  w0 = 2 * pi * f / fs;
  alpha = sin(w0) / (2 * q);

  b0 = sin(w0) / 2;
  b2 = -sin(w0) / 2;
  a0 = 1 + alpha;
  a1 = -2 * cos(w0);
  a2 = 1 - alpha;

  c = [b0 b2;
       a1 a2] ./ a0;
end
