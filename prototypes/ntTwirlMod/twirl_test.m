p.fs = 48000;
p.tCount = 1:48000;
p.freq = 2;
w1 = pi .* p.freq ./ p.fs .* p.tCount + pi / 4;
w2 = 2 .* pi .* p.freq ./ p.fs .* p.tCount;
sinW1 = (sin(w1));
sinW2 = (sin(w1 - pi / 2));
sinW3 = (sin(w2));
sinW4 = (sin(w2 - pi));
w5 = 2 .* pi .* p.freq ./ p.fs .* p.tCount + pi / 2;
sinW5 = (sin(w5));

sinW5(sinW5 > 0) = 0;
wDiff = (sinW5 + 1);

fig = figure();
plot(tCount, (sinW3 + 1) / 2);
hold on;
plot(tCount, (sinW4 + 1) / 2);
plot(tCount, abs(sinW1), 'LineWidth', 1.2);
plot(tCount, abs(sinW2), 'LineWidth', 1.2);
plot(tCount, wDiff, '--', 'LineWidth', 1.3);

saveas(fig, 'sines.png');
