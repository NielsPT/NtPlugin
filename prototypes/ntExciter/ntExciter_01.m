classdef ntExciter_01 < audioPlugin
   properties
      bypass = false;
      rel_gain = true;
      kill_dry = false;
      notch_bypass = false;
      f_bpf_gui = 5000;
      q_bpf_gui = 0.707;
      g_bpf_gui = 0;
      g_out_gui = 0;
      hardness = 0;
      clipG_gui = 0;
      bell_notch = true;
   end
   properties (Access = private)
      fs = 48000;
      f_bpf = 0;
      q_bpf = 0;
      g_bpf = 0;
      b_bpf = zeros(1, 3);
      a_bpf = zeros(1, 3);
      s_bpf = zeros(4, 2);

      b_notch = zeros(1, 3);
      a_notch = zeros(1, 3);
      s_notch = zeros(4, 2);

      clipG = 1;
      alpha_softClip = 0.5;
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
         audioPluginParameter('g_bpf_gui', ...
         'DisplayName', 'Gain', ...
         'Mapping', {'lin', -40, 40}, ...
         'Label', '', ...
         'Style', 'rotary', ...
         'Layout', [2, 3]), ...
         audioPluginParameter('clipG_gui', ...
         'DisplayName', 'Soft Clip Gain', ...
         'Mapping', {'lin', -40, 40}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [2, 4]), ...
         audioPluginParameter('hardness', ...
         'DisplayName', 'Character', ...
         'Mapping', {'lin', 0, 100}, ...
         'Label', '%', ...
         'Style', 'rotary', ...
         'Layout', [2, 5]), ...
         audioPluginParameter('bell_notch', ...
         'Mapping', {'enum', 'Notch', 'Bell'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 1], ...
         'DisplayName', 'Notch type'), ...
         audioPluginParameter('notch_bypass', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 2], ...
         'DisplayName', 'Notch'), ...
         audioPluginParameter('kill_dry', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 3], ...
         'DisplayName', 'Dry'), ...
         audioPluginParameter('rel_gain', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 4], ...
         'DisplayName', 'Relative Gain'), ...
         audioPluginParameter('g_out_gui', ...
         'DisplayName', 'Output', ...
         'Mapping', {'lin', -100, 40}, ...
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
         'VendorVersion', '0.1.0', ...
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
            a0_bpf = p.a_bpf(1, 1);
            a1_bpf = p.a_bpf(1, 2);
            a2_bpf = p.a_bpf(1, 3);
            b0_bpf = p.b_bpf(1, 1);
            b1_bpf = p.b_bpf(1, 2);
            b2_bpf = p.b_bpf(1, 3);

            %set up states for band pass
            yn1_bpf = p.s_bpf(1, :);
            yn2_bpf = p.s_bpf(2, :);
            xn1_bpf = p.s_bpf(3, :);
            xn2_bpf = p.s_bpf(4, :);

            %load coefficients for notch
            a0_notch = p.a_notch(1, 1);
            a1_notch = p.a_notch(1, 2);
            a2_notch = p.a_notch(1, 3);
            b0_notch = p.b_notch(1, 1);
            b1_notch = p.b_notch(1, 2);
            b2_notch = p.b_notch(1, 3);

            %set up states for notch
            yn1_notch = p.s_notch(1, :);
            yn2_notch = p.s_notch(2, :);
            xn1_notch = p.s_notch(3, :);
            xn2_notch = p.s_notch(4, :);

            for i = 1:n
               % load next input sample
               xn0_bpf = x(i, :);

               % apply band pass filter
               yn0_bpf = (b0_bpf .* xn0_bpf ...
                  + b1_bpf .* xn1_bpf ... % TODO: b1 is alway 0?
                  + b2_bpf .* xn2_bpf ...
                  - a1_bpf .* yn1_bpf ...
                  - a2_bpf .* yn2_bpf) ...
                  / a0_bpf; %TODO: optimize out a0.

               % update states
               xn2_bpf = xn1_bpf;
               xn1_bpf = xn0_bpf;
               yn2_bpf = yn1_bpf;
               yn1_bpf = yn0_bpf;

               % store output sample
               y_bpf = yn0_bpf;

               % adjust gain
               x_clip = y_bpf .* p.clipG .* 1 / (1 + p.alpha_softClip);

               % distort output from filter
               % apply distortion
               if x_clip(1) > 1
                  y_clip(1) = 1;
               elseif x_clip(1) < -1
                  y_clip(1) = -1;
               else
                  y_clip(1) = ((1 + p.alpha_softClip) .* x_clip(1) - p.alpha_softClip .* x_clip(1) .^ 3);
               end

               if x_clip(2) > 1
                  y_clip(2) = 1;
               elseif x_clip(2) < -1
                  y_clip(2) = -1;
               else
                  y_clip(2) = ((1 + p.alpha_softClip) .* x_clip(2) - p.alpha_softClip .* x_clip(2) .^ 3);
               end

               % adjust gain
               x_notch = y_clip ./ p.clipG;

               if ~p.notch_bypass
                  % apply notch filter
                  xn0_notch = x_notch;
                  yn0_notch = (b0_notch .* xn0_notch ...
                     + b1_notch .* xn1_notch ... % TODO: b1 is alway 0?
                     + b2_notch .* xn2_notch ...
                     - a1_notch .* yn1_notch ...
                     - a2_notch .* yn2_notch) ...
                     / a0_notch; %TODO: optimize out a0.

                  % update states
                  xn2_notch = xn1_notch;
                  xn1_notch = xn0_notch;
                  yn2_notch = yn1_notch;
                  yn1_notch = yn0_notch;

                  % store output sample
                  y_notch = yn0_notch;
               else
                  % or bypass
                  y_notch = x_notch;
               end

               % gain output from filter
               x_sum = y_notch .* p.g_bpf;

               % sum result
               y(i, :) = (x(i, :) * p.g_dry + x_sum) .* p.g_out;
            end
            %store state
            p.s_bpf(1, :) = yn1_bpf;
            p.s_bpf(2, :) = yn2_bpf;
            p.s_bpf(3, :) = xn1_bpf;
            p.s_bpf(4, :) = xn2_bpf;

            p.s_notch(1, :) = yn1_notch;
            p.s_notch(2, :) = yn2_notch;
            p.s_notch(3, :) = xn1_notch;
            p.s_notch(4, :) = xn2_notch;
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
         p.alpha_softClip = 0.5 - (p.hardness / 200);
         p.clipG = 10 ^ (p.clipG_gui / 20);
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

         if p.rel_gain
            b0 = sin(w0) / 2;
            b1 = 0;
            b2 = -sin(w0) / 2;
            a0 = 1 + alpha;
            a1 = -2 * cos(w0);
            a2 = 1 - alpha;
         else
            b0 = alpha;
            b1 = 0;
            b2 = -alpha;
            a0 = 1 + alpha;
            a1 = -2 * cos(w0);
            a2 = 1 - alpha;
         end

         p.b_bpf = [b0 b1 b2];
         p.a_bpf = [a0 a1 a2];

         if p.bell_notch
            A = 1/8;
            b0 = 1 + alpha * A;
            b1 = -2 * cos(w0);
            b2 = 1 - alpha * A;
            a0 = 1 + alpha / A;
            a1 = -2 * cos(w0);
            a2 = 1 - alpha / A;
         else
            b0 = 1;
            b1 = -2 * cos(w0);
            b2 = 1;
            a0 = 1 + alpha;
            a1 = -2 * cos(w0);
            a2 = 1 - alpha;
         end

         p.b_notch = [b0 b1 b2];
         p.a_notch = [a0 a1 a2];
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
      function set.clipG_gui(p, val)
         p.clipG_gui = val;
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
      function set.rel_gain(p, val)
         p.rel_gain = val;
         update(p);
      end
      function set.notch_bypass(p, val)
         p.notch_bypass = val;
         update(p);
      end
      function set.bell_notch(p, val)
         p.bell_notch = val;
         update(p);
      end
   end
end
