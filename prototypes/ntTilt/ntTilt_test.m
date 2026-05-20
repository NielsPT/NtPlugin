fs = 48000;
N = 2400;
n_ax = 0:N - 1;
x = [1 zeros(1, N - 1)];
f_ax = n_ax * fs / N;
fig = figure('visible', 'off');

n_stages = 8;
% freqs = [100, 200, 400, 800, 1600, 3200, 6400, 12800];
freqs = logspace(log10(20), log10(20e3), n_stages + 2);
freqs = freqs(2:end - 1);
% freqs = logspace(log10(80), log10(fs / 2), n_stages + 1)
% freqs = freqs(1:end - 1)
plot_stages = true;
plot_stages = false;
tilt_ui = [-6 -4 -2 0 2 4 6];
% tilt_ui = tilt_ui(1);
for t = tilt_ui
  g_shelf_db = -2 .* t; % / n_stages;
  g_shelf_db = g_shelf_db / n_stages;
  g = (ones(n_stages, 1) .* 10 .^ (g_shelf_db ./ 20) - 1);
  st_xn1 = zeros(n_stages, 1);
  st_yn1 = zeros(n_stages, 1);
  z = 2 .* pi .* freqs ./ fs;
  alpha = z ./ (z + 1);
  y_stages = zeros(n_stages, N);
  for i = 1:N
    for i_s = 1:n_stages
      y_lpf = alpha(i_s) .* x(i) + (1 - alpha(i_s)) .* st_yn1(i_s);
      y_hpf = alpha(i_s) * (st_yn1(i_s) + x(i) - st_xn1(i_s));
      st_xn1(i_s) = x(i);
      st_yn1(i_s) = y_lpf;
      y_stages(i_s, i) = y_lpf * g(i_s) + x(i);
    end
  end
  if plot_stages
    for i_s = 1:n_stages
      y_fft_db = 20 * log10(abs(fft(y_stages(i_s, :))));
      semilogx(f_ax, y_fft_db);
      hold on
    end
  end
  y = zeros(1, N);
  for i = 1:N
    y_shelf = 0;
    for i_s = 1:n_stages
      if i_s == 1
        x_ = x(i);
      else
        x_ = y_shelf;
      end
      y_lpf = alpha(i_s) .* x_ + (1 - alpha(i_s)) .* st_yn1(i_s);
      st_xn1(i_s) = x_;
      st_yn1(i_s) = y_lpf;
      y_shelf = (y_lpf * g(i_s) + x_);
      y_stages(i_s, i) = y_shelf;
    end
    y(i) = y_shelf;
  end
  y = y * 10 .^ (t / 20);
  y_fft_db = 20 * log10(abs(fft(y)));
  semilogx(f_ax, y_fft_db, "LineWidth", 2);
  grid on
  hold on
end
% plot([0:(3000 - 1)], ones(1, 3000), "LineWidth", 2)
xlim([20, 20e3]);
ylim([-6 6])
saveas(fig, "out/something.png");
