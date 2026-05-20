% clear all;
fig = figure;

p.fs = 48000;
p.bypass = false;
p.trigged = true;
p.triple = false;
p.modFreq = 0.2;
p.modDepth = 0.66;
p.modPhase = 90;
p.mix = 0;
p.mix_gui = 50;
p.len_delayLine = 48000;
p.dLine = zeros(48001, 2);
p.store_loc = 1;
p.t_delay = 0.03;
p.n_delay = 0;
p.tCount = 1;
p.state = zeros(8, 1);
p.a_rect = 0;
p.t_rect = 0.5;
p.f_diff = 0;
p.sens = 0.1;
p.f_lpf1 = 6000;
p.st_lpf1 = zeros(1, 2);
p.a_lpf1 = 0;
p.samplesSinceTrig = 0;
p.n_reTrig = 500;
p.t_reTrig = 1/64;
p.a_avg = 0;
p.t_avg = 5;
p.a_diff_smooth = 0;
p.t_diff_smooth = 0.002;

p.mix = p.mix_gui / 100;
p.n_delay = round(p.t_delay * p.fs);
p.state(6) = p.n_delay;
p.a_rect = exp(-1 / (p.t_rect * p.fs));
p.a_lpf1 = 1 - exp(-2 .* pi .* p.f_lpf1 ./ p.fs);
p.a_avg = 1 - exp(-1 / (p.t_avg * p.fs));
p.a_diff_smooth = 1 - exp(-1 / (p.t_diff_smooth * p.fs));
p.n_reTrig = p.t_reTrig * p.fs;

p.state(7) = 0.15;

[xl, p.fs] = audioread('../di gtr1.wav');

x = [xl(p.fs:p.fs * 3) xl(p.fs:p.fs * 3)];

[y, res_rect, res_avg, res_diff, res_norm, res_trig] = process(p, x);

% res_norm(1:100000) = 0;

n_ax = 0:length(x) - 1;
t_ax = n_ax / p.fs;

plot(t_ax, x(:, 1));
hold on
plot(t_ax, res_rect);
plot(t_ax, res_avg);
plot(t_ax, res_diff);
plot(t_ax, res_norm);
plot(t_ax, res_trig);
legend('org', 'rect', 'avg', 'diff', 'norm', 'trig');
saveas(fig, 'plot.png');

function [y, res_rect, res_avg, res_diff, res_norm, res_trig] = process(p, x)
   n = length(x);
   y = zeros(n, 2);
   lLoc = [0 0];

   y_rect_1 = p.state(1);
   y_rect_2 = p.state(2);
   y_diff_1 = p.state(3);
   y_diff_2 = p.state(4);
   n_mod = [p.state(5), p.state(6)];
   y_avg_1 = p.state(7);
   y_diff_smooth_1 = p.state(8);
   res_rect = ones(n, 1);
   res_diff = ones(n, 1);
   res_avg = ones(n, 1);
   res_norm = ones(n, 1);
   res_trig = zeros(n, 1);
   sum_diff = 0;

   x(~isreal(x)) = 0;

   if (p.bypass == true)
      y = x;
   else
      for i = 1:n
         p.dLine(p.store_loc, :) = x(i, :);

         if (p.trigged == true)
            % LPF
            y_lpf = p.a_lpf1 * x(i, :) + (1 - p.a_lpf1) * p.st_lpf1;
            p.st_lpf1 = y_lpf;

            % rectify and smooth
            y_rect = max(max(abs(y_lpf)), p.a_rect * y_rect_1 + ...
               (1 - p.a_rect) * max(abs(y_lpf)));
            res_rect(i, 1) = y_rect;

            % Take average
            y_avg = p.a_avg * y_rect + (1 - p.a_avg) * y_avg_1;
            y_avg_1 = y_avg;
            res_avg(i, 1) = y_avg;

            % differentiate
            y_diff = y_rect - y_rect_2;
            res_diff(i, 1) = y_diff;

            % store state
            y_rect_2 = y_rect_1;
            y_rect_1 = y_rect;

            % compare
            if (y_diff > y_diff_1) && (y_diff > 0)
               sum_diff = sum_diff + y_diff;
            else
               sum_diff = 0;
            end
            y_norm = sum_diff / y_avg;
            res_norm(i, 1) = y_norm;

            trig = false;
            if y_norm > p.sens
               if p.samplesSinceTrig > p.n_reTrig
                  trig = true;
                  p.samplesSinceTrig = 0;
               end
            end
            p.samplesSinceTrig = p.samplesSinceTrig + 1;

            % store state
            y_diff_2 = y_diff_1;
            y_diff_1 = y_diff;

            % calculate delay
            if trig == true
               if p.triple == false
                  n_mod(1) = 0;
               else
                  n_mod(1) = round((rand * 2 - 1) * p.n_delay * p.modDepth) + p.n_delay;
               end
               n_mod(2) = round((rand * 2 - 1) * p.n_delay * p.modDepth) + p.n_delay;
               res_trig(i, 1) = 0.25;
               disp("trigged, diff = " + num2str(y_norm) + ", t_mod = " + num2str(n_mod / p.fs));
            end

         else
            % calculate modulated delay time
            n_mod(1) = round(sin(2 .* pi .* p.modFreq ./ p.fs .* p.tCount) .* ...
               p.n_delay .* p.modDepth) + p.n_delay;
            n_mod(2) = round(sin(2 .* pi .* p.modFreq ./ p.fs .* p.tCount + ...
               p.modPhase * pi / 180) .* p.n_delay .* p.modDepth) + p.n_delay;
            p.tCount = p.tCount + 1;
         end

         lLoc(1) = p.store_loc - n_mod(1);
         if (lLoc(1) < 1)
            lLoc(1) = lLoc(1) + p.len_delayLine;
         end
         lLoc(2) = p.store_loc - n_mod(2);
         if (lLoc(2) < 1)
            lLoc(2) = lLoc(2) + p.len_delayLine;
         end

         if p.triple == true
            y(i, 1) = p.mix .* p.dLine(lLoc(1), 1) + (1 - p.mix) .* x(i, 1);
            y(i, 2) = p.mix .* p.dLine(lLoc(2), 2) + (1 - p.mix) .* x(i, 2);
         else
            y(i, 1) = x(i, 1);
            y(i, 2) = p.dLine(lLoc(2), 2);
         end
         p.store_loc = p.store_loc + 1;
         p.store_loc(p.store_loc > p.len_delayLine) = 1;
      end
   end
   p.state = [y_rect_1; y_rect_2; y_diff_1; y_diff_2; ...
            n_mod(1); n_mod(2); y_avg_1; y_diff_smooth_1];
end
