classdef ntExciter_02 < audioPlugin
   properties
      bypass = false;
      kill_dry = false;
      f_bpf_gui = 5000;
      q_bpf_gui = 0.707;
      g_bpf_gui = 0;
      g_out_gui = 0;
      hardness = 0;
      g_clip_gui = 0;
   end
   properties (Access = private)
      fs = 48000;
      f_bpf = 0;
      q_bpf = 0;
      g_bpf = 0;
      b_bpf = zeros(1, 3);
      a_bpf = zeros(1, 3);
      s_bpf = zeros(4, 2);

      g_clip = 1;
      a_clip = 0.5;
      g_out = 1;
      g_dry = 1;
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
         'Mapping', {'log', 0.1, 40}, ...
         'Label', '', ...
         'Style', 'rotary', ...
         'Layout', [2, 2]), ...
         audioPluginParameter('g_clip_gui', ...
         'DisplayName', 'Soft Clip Gain', ...
         'Mapping', {'lin', -40, 40}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [2, 3]), ...
         audioPluginParameter('hardness', ...
         'DisplayName', 'Character', ...
         'Mapping', {'lin', 0, 100}, ...
         'Label', '%', ...
         'Style', 'rotary', ...
         'Layout', [2, 4]), ...
         audioPluginParameter('g_bpf_gui', ...
         'DisplayName', 'Level', ...
         'Mapping', {'lin', -40, 40}, ...
         'Label', '', ...
         'Style', 'rotary', ...
         'Layout', [2, 5]), ...
         audioPluginParameter('kill_dry', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 3], ...
         'DisplayName', 'Dry'), ...
         audioPluginParameter('g_out_gui', ...
         'DisplayName', 'Output', ...
         'Mapping', {'lin', -40, 40}, ...
         'Label', '', ...
         'Style', 'rotary', ...
         'Layout', [4, 4]), ...
         audioPluginParameter('bypass', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 5], ...
         'DisplayName', 'Master Bypass'), ...
         audioPluginGridLayout( ...
         'RowHeight', [30, 150, 15, 100, 15], ...
         'ColumnWidth', [100, 100, 100, 100, 100, 100], ...
         'RowSpacing', 30), ...
         'PluginName', '', ...
         'VendorName', 'NT', ...
         'VendorVersion', '0.2.0', ...
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
               x_clip = y_bpf .* p.g_clip .* p.g_bpf .* 1 / (1 + p.a_clip);

               % distort output from filter
               % apply distortion
               y_clip = [0 0];
               if x_clip(1) > 1
                  y_clip(1) = 1;
               elseif x_clip(1) < -1
                  y_clip(1) = -1;
               else
                  y_clip(1) = ((1 + p.a_clip) .* x_clip(1) - p.a_clip .* x_clip(1) .^ 3);
               end

               if x_clip(2) > 1
                  y_clip(2) = 1;
               elseif x_clip(2) < -1
                  y_clip(2) = -1;
               else
                  y_clip(2) = ((1 + p.a_clip) .* x_clip(2) - p.a_clip .* x_clip(2) .^ 3);
               end

               % adjust gain
               x_gain = y_clip ./ p.g_clip;

               % gain output from filter
               x_sum = x_gain;

               % sum result
               y(i, :) = (x(i, :) * p.g_dry + x_sum) .* p.g_out;
            end
            %store state
            p.s_bpf(1, :) = yn1_bpf;
            p.s_bpf(2, :) = yn2_bpf;
            p.s_bpf(3, :) = xn1_bpf;
            p.s_bpf(4, :) = xn2_bpf;
         end
      end
      function reset(p)
         p.fs = getSampleRate(p);
         update(p);
      end
      function update(p)
         p.f_bpf = p.f_bpf_gui; % TODO: glide
         p.q_bpf = p.q_bpf_gui;
         p.g_bpf = 10 ^ (p.g_bpf_gui / 20);
         p.g_out = 10 ^ (p.g_out_gui / 20);
         p.a_clip = 0.5 - (p.hardness / 200);
         p.g_clip = 10 ^ (p.g_clip_gui / 20);
         if p.kill_dry
            p.g_dry = 0;
         else
            p.g_dry = 1;
         end
         calculateCoeffs(p);
      end
      function calculateCoeffs(p)
         w0 = 2 * pi * p.f_bpf / p.fs;
         alpha = sin(w0) / (2 * p.q_bpf);

         b0 = sin(w0) / 2;
         b2 = -sin(w0) / 2;
         a0 = 1 + alpha;
         a1 = -2 * cos(w0);
         a2 = 1 - alpha;

         p.b_bpf = [b0 b2] ./ a0;
         p.a_bpf = [a1 a2] ./ a0;
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
      function set.hardness(p, val)
         p.hardness = val;
         update(p);
      end
      function set.kill_dry(p, val)
         p.kill_dry = val;
         update(p);
      end
   end
end
