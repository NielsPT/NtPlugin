classdef ntExciter_03 < audioPlugin
   properties
      bypass = false;
      kill_dry = false;
      clip_bypass = false;
      f_bpf_gui = 5000;
      q_bpf_gui = 0.707;
      g_bpf_gui = 0;
      g_out_gui = 0;
      g_clip_gui = 0;
   end
   properties (Access = private)
      fs = 48000;
      f_bpf = 0;
      q_bpf = 0;
      g_bpf = 0;
      b_bpf = zeros(1, 2);
      a_bpf = zeros(1, 2);
      b_step = zeros(1, 2);
      a_step = zeros(1, 2);

      s_bpf = zeros(4, 2);

      g_clip = 1;
      g_clip_recipr = 1;
      g_out = 1;
      g_dry = 1;

      t_glide = 0.1;
      n_glide = 4410;
      n_step = 0;
   end
   properties (Constant)
      PluginInterface = audioPluginInterface( ...
         audioPluginParameter('f_bpf_gui', ...
         'DisplayName', 'Freq', ...
         'Mapping', {'log', 20, 10e3}, ...
         'Label', 'Hz', ...
         'Style', 'rotary', ...
         'Layout', [2, 1]), ...
         audioPluginParameter('q_bpf_gui', ...
         'DisplayName', 'Q', ...
         'Mapping', {'log', 0.1, 100}, ...
         'Label', '', ...
         'Style', 'rotary', ...
         'Layout', [2, 2]), ...
         audioPluginParameter('g_bpf_gui', ...
         'DisplayName', 'Level', ...
         'Mapping', {'lin', -40, 40}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [2, 3]), ...
         audioPluginParameter('g_clip_gui', ...
         'DisplayName', 'Drive', ...
         'Mapping', {'lin', -40, 40}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [2, 4]), ...
         audioPluginParameter('clip_bypass', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 1], ...
         'DisplayName', 'Drive'), ...
         audioPluginParameter('kill_dry', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 2], ...
         'DisplayName', 'Dry'), ...
         audioPluginParameter('g_out_gui', ...
         'DisplayName', 'Output', ...
         'Mapping', {'lin', -40, 40}, ...
         'Label', '', ...
         'Style', 'rotary', ...
         'Layout', [4, 3]), ...
         audioPluginParameter('bypass', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 4], ...
         'DisplayName', 'Master Bypass'), ...
         audioPluginGridLayout( ...
         'RowHeight', [30, 150, 15, 100, 15], ...
         'ColumnWidth', [100, 100, 100, 100], ...
         'RowSpacing', 30), ...
         'PluginName', '', ...
         'VendorName', 'NT', ...
         'VendorVersion', '0.3.0', ...
         'InputChannels', 2, ...
         'OutputChannels', 2, ...
         'BackgroundImage', 'logo.png' ...
      );
   end
   methods
      function y = process(p, x)
         x(~isfinite(x)) = 0;
         n = length(x);
         y = zeros(n, 2);
         if p.bypass
            y = x;
         else
            %load coefficients for band pass
            a1_bpf = p.a_bpf(1, 1);
            a2_bpf = p.a_bpf(1, 2);
            b0_bpf = p.b_bpf(1, 1);
            b2_bpf = p.b_bpf(1, 2);

            %set up states for band pass
            yn1_bpf = p.s_bpf(1, :);
            yn2_bpf = p.s_bpf(2, :);
            xn1_bpf = p.s_bpf(3, :);
            xn2_bpf = p.s_bpf(4, :);

            for i = 1:n
               % load next input sample
               xn0_bpf = x(i, :);

               % apply band pass filter
               yn0_bpf = (b0_bpf .* xn0_bpf ...
                  + b2_bpf .* xn2_bpf ...
                  - a1_bpf .* yn1_bpf ...
                  - a2_bpf .* yn2_bpf);

               % update states
               xn2_bpf = xn1_bpf;
               xn1_bpf = xn0_bpf;
               yn2_bpf = yn1_bpf;
               yn1_bpf = yn0_bpf;

               % store output sample
               y_bpf = yn0_bpf;

               % adjust gain
               x_clip = y_bpf .* p.g_clip .* p.g_bpf;

               if ~p.clip_bypass
                  % distort output from filter
                  y_clip = [0 0];
                  if x_clip(1) > 1
                     y_clip(1) = 1;
                  elseif x_clip(1) < -1
                     y_clip(1) = -1;
                  else
                     y_clip(1) = 1.5 .* x_clip(1) - 0.5 .* x_clip(1) .^ 3;
                  end

                  if x_clip(2) > 1
                     y_clip(2) = 1;
                  elseif x_clip(2) < -1
                     y_clip(2) = -1;
                  else
                     y_clip(2) = 1.5 .* x_clip(2) - 0.5 .* x_clip(2) .^ 3;
                  end
               else
                  y_clip = x_clip;
               end

               % adjust gain. -6 dB to make have a nice balance at 0 dB.
               x_sum = y_clip .* p.g_clip_recipr .* 0.5;

               % sum result
               y(i, :) = (x(i, :) * p.g_dry + x_sum) .* p.g_out;

               % Glide coeffs
               if p.n_step >= 0
                  p.n_step = p.n_step - 1;
                  b0_bpf = b0_bpf + p.b_step(1);
                  b2_bpf = b2_bpf + p.b_step(2);
                  a1_bpf = a1_bpf + p.a_step(1);
                  a2_bpf = a2_bpf + p.a_step(2);
               end
            end
            %store state
            p.s_bpf(1, :) = yn1_bpf;
            p.s_bpf(2, :) = yn2_bpf;
            p.s_bpf(3, :) = xn1_bpf;
            p.s_bpf(4, :) = xn2_bpf;

            % store the (maybe) modified coeffs
            p.b_bpf = [b0_bpf b2_bpf];
            p.a_bpf = [a1_bpf a2_bpf];
         end
      end
      function reset(p)
         p.fs = getSampleRate(p);
         p.n_glide = round(p.fs * p.t_glide);
         update(p);
         c = calculateCoeffs(p);
         p.b_bpf = c(1, :);
         p.a_bpf = c(2, :);
      end
      function update(p)
         p.f_bpf = p.f_bpf_gui;
         p.q_bpf = p.q_bpf_gui;
         p.g_bpf = 10 ^ (p.g_bpf_gui / 20);
         p.g_out = 10 ^ (p.g_out_gui / 20);
         p.g_clip = 10 ^ (p.g_clip_gui / 20);
         p.g_clip_recipr = 1 / p.g_clip;
         if p.kill_dry
            p.g_dry = 0;
         else
            p.g_dry = 1;
         end
         c_next = calculateCoeffs(p);
         c_curr = [p.b_bpf; p.a_bpf];
         c_diff = c_next - c_curr;
         c_steps = c_diff ./ p.n_glide;
         p.b_step = c_steps(1, :);
         p.a_step = c_steps(2, :);
         p.n_step = p.n_glide;
      end
      function c = calculateCoeffs(p)
         w0 = 2 * pi * p.f_bpf / p.fs;
         alpha = sin(w0) / (2 * p.q_bpf);

         b0 = sin(w0) / 2;
         b2 = -sin(w0) / 2;
         a0 = 1 + alpha;
         a1 = -2 * cos(w0);
         a2 = 1 - alpha;

         c = [b0 b2;
              a1 a2] ./ a0;
      end
      function set.f_bpf_gui(p, val)
         p.f_bpf_gui = val;
         update(p);
      end
      function set.q_bpf_gui(p, val)
         p.q_bpf_gui = val;
         update(p);
      end
      function set.g_bpf_gui(p, val)
         p.g_bpf_gui = val;
         update(p);
      end
      function set.g_out_gui(p, val)
         p.g_out_gui = val;
         update(p);
      end
      function set.g_clip_gui(p, val)
         p.g_clip_gui = val;
         update(p);
      end
      function set.kill_dry(p, val)
         p.kill_dry = val;
         update(p);
      end
      function set.clip_bypass(p, val)
         p.clip_bypass = val;
         update(p);
      end
   end
end
