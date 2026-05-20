fs = 44100;
x = audioread("out/test.wav");
fig = figure();
n_test = round(0.001 * fs);
x_part = x(1:n_test);
n_ax = 0:(length(x_part) - 1);
plot(n_ax, x_part);
grid on
hold on
saveas(fig, "out/test.png");
