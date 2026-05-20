classdef ntExciter_05 < audioPlugin
   properties
      bypass = false;
      kill_dry = false;
      clip_bypass = false;
      f_bpf_gui = 5000;
      q_bpf_gui = 0.707;
      g_bpf_gui = 0;
      g_out_gui = 0;
      g_clip_gui = 0;
      softness_procent = 100;
      dc_offset = 0.5;
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
      g_out = 1;
      g_dry = 1;
      alpha_softClip = 0.5;
      % g_clip_recipr = 1;
      f_hpf = 20;
      alpha_hpf = 0;
      state = zeros(2, 2);

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
         audioPluginParameter('dc_offset', ...
         'DisplayName', 'DC Offset', ...
         'Mapping', {'lin', 0, 1}, ...
         'Label', '', ...
         'Style', 'rotary', ...
         'Layout', [2, 5]), ...
         audioPluginParameter('softness_procent', ...
         'DisplayName', 'Softness', ...
         'Mapping', {'lin', 0, 100}, ...
         'Label', '%', ...
         'Style', 'rotary', ...
         'Layout', [2, 6]), ...
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
         'Layout', [4, 5]), ...
         audioPluginParameter('bypass', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 6], ...
         'DisplayName', 'Master Bypass'), ...
         audioPluginGridLayout( ...
         'RowHeight', [30, 150, 15, 100, 15], ...
         'ColumnWidth', [100, 100, 100, 100, 100, 100], ...
         'RowSpacing', 30), ...
         'PluginName', '', ...
         'VendorName', 'NT', ...
         'VendorVersion', '0.5.0', ...
         'InputChannels', 2, ...
         'OutputChannels', 2, ...
         'BackgroundImage', 'logo.png' ...
      );
   end
   methods (Static)
      function y = soft_clip_asym(x, g_up, g_up_recipr, g_dn, g_dn_recipr, alpha)
         if x > 0
            tmp = x .* g_up;
         else
            tmp = x .* g_dn;
         end

         if tmp > 1
            y = 1;
         elseif tmp < -1
            y = -1;
         else
            y = (1 + alpha) .* tmp - alpha .* tmp .^ 3;
         end

         if x > 0
            y = y .* g_up_recipr .* 0.6667;
         else
            y = y .* g_dn_recipr .* 0.6667;
         end
      end
      function y = soft_clip_2nd(x, g, g_recipr, alpha, dc)
         x_dc = 0.5 .* g .* x + dc;
         if x_dc > 1
            y_dc = 1;
         elseif x_dc < 0
            y_dc = 0;
         else
            y_dc = (1 + alpha) .* x_dc - alpha .* x_dc .^ 2;
         end
         y = 2 .* (y_dc - dc) .* g_recipr;
      end
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

            % Coeffs for soft clip
            g_cl = p.g_clip;
            g_cl_recipr = 1 / g_cl;
            alpha = p.alpha_softClip;
            dc = p.dc_offset;
            xn1_hpf = p.state(1, :);
            yn1_hpf = p.state(2, :);
            a_hpf = p.alpha_hpf;

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
               x_clip = y_bpf;

               % distort output from filter
               if ~p.clip_bypass
                  x_hpf = [0 0];
                  x_hpf(1) = p.soft_clip_2nd(x_clip(1), g_cl, g_cl_recipr, alpha, dc);
                  x_hpf(2) = p.soft_clip_2nd(x_clip(2), g_cl, g_cl_recipr, alpha, dc);
                  y_hpf = a_hpf .* (yn1_hpf + x_hpf - xn1_hpf);
                  xn1_hpf = x_hpf;
                  yn1_hpf = y_hpf;
               else
                  y_hpf = x_clip;
               end

               % adjust gain.
               x_sum = y_hpf .* p.g_bpf;

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

            p.state(1, :) = xn1_hpf;
            p.state(2, :) = yn1_hpf;

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
         p.alpha_softClip = 1 - ((100 - p.softness_procent) / 100);

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

         z = 2 .* pi .* p.f_hpf ./ p.fs;
         p.alpha_hpf = 1 ./ (z + 1);
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
      function set.softness_procent(p, val)
         p.softness_procent = val;
         update(p);
      end
      function set.dc_offset(p, val)
         p.dc_offset = val;
         update(p);
      end
   end
end
