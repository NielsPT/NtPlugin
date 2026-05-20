classdef ntPassive_04 < audioPlugin
   properties
      bypass = false;
      clip_bypass = false;
      f_lf = 100;
      g_lf_db = 0;
      f_hf = 2000;
      g_hf_db = 0;
      f_mf = 1000;
      g_mf_db = 0;
      g_db = 0;
      g_clip_db = 0;
      g_clip_lf_db = 0;
      g_clip_mf_db = 0;
      g_clip_hf_db = 0;

      oversampling_mode = oversamplingEnum.fir_4x_lq;
   end
   properties (Access = private)
      gain_lf = 0;
      gain_hf = 0;
      gain_mf = 0;
      gain_out = 0;
      alpha_bp_lpf = 0;
      alpha_bp_hpf = 0;
      alpha_lpf = 0;
      alpha_hpf = 0;
      fs = 44100;
      gain_clip = 1;
      gain_clip_recipr = 1;
      gain_clip_lf = 1;
      gain_clip_mf = 1;
      gain_clip_hf = 1;
      state = zeros(9, 2);

      bypass_oversampling = false;
      oversampling_uses_iir = false;
      oversampling_factor = 8;
      n_coeffs_fir = 1;
      oversampling_fir_mult = 1;
      b_coeffs_fir = zeros(192, 1);
      interpolation_delay_line = zeros(192 * 2, 2);
      antialiasing_delay_line = zeros(192 * 2, 2);
   end
   properties (Constant)
      n_delay_line = 192;
      oversampling_fir_mult_lq = 12;
      oversampling_fir_mult_hq = 24;
      PluginInterface = audioPluginInterface( ...
         audioPluginParameter('f_lf', ...
         'DisplayName', 'Low Freq', ...
         'Mapping', {'log', 10, 1e3}, ...
         'Label', 'Hz', ...
         'Style', 'rotary', ...
         'Layout', [2, 1]), ...
         audioPluginParameter('f_mf', ...
         'DisplayName', 'Mid Freq', ...
         'Mapping', {'log', 20, 20e3}, ...
         'Label', 'Hz', ...
         'Style', 'rotary', ...
         'Layout', [2, 2]), ...
         audioPluginParameter('f_hf', ...
         'DisplayName', 'High Freq', ...
         'Mapping', {'log', 1e3, 100e3}, ...
         'Label', 'Hz', ...
         'Style', 'rotary', ...
         'Layout', [2, 3]), ...
         audioPluginParameter('g_lf_db', ...
         'DisplayName', 'Lo Gain', ...
         'Mapping', {'lin', 0, 24}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [4, 1]), ...
         audioPluginParameter('g_mf_db', ...
         'DisplayName', 'Mid Gain', ...
         'Mapping', {'lin', 0, 24}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [4, 2]), ...
         audioPluginParameter('g_hf_db', ...
         'DisplayName', 'High Gain', ...
         'Mapping', {'lin', 0, 24}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [4, 3]), ...
         audioPluginParameter('g_clip_db', ...
         'DisplayName', 'Wide band drive', ...
         'Mapping', {'lin', -40, 40}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [4, 4]), ...
         audioPluginParameter('g_db', ...
         'DisplayName', 'Trim', ...
         'Mapping', {'lin', -24, 0}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [4, 5]), ...
         audioPluginParameter('g_clip_lf_db', ...
         'DisplayName', 'Low drive', ...
         'Mapping', {'lin', -40, 40}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [6, 1]), ...
         audioPluginParameter('g_clip_mf_db', ...
         'DisplayName', 'Mid drive', ...
         'Mapping', {'lin', -40, 40}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [6, 2]), ...
         audioPluginParameter('g_clip_hf_db', ...
         'DisplayName', 'High drive', ...
         'Mapping', {'lin', -40, 40}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [6, 3]), ...
         audioPluginParameter('clip_bypass', ...
         'DisplayName', 'All saturation', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [6, 4]), ...
         audioPluginParameter('bypass', ...
         'DisplayName', 'Master Bypass', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [6, 5]), ...
         audioPluginParameter('oversampling_mode', ...
         'DisplayName', 'Oversampling', ...
         'Mapping', {'enum', 'Off', '2 x IIR', '4 x IIR', '8 x IIR', ...
            '2 x FIR LQ', '4 x FIR LQ', '8 x FIR LQ', ...
            '2 x FIR HQ', '4 x FIR HQ', '8 x FIR HQ', ...
         }, ...
         'Label', '', ...
         'Style', 'dropdown', ...
         'Layout', [8, 1]), ...
         audioPluginGridLayout( ...
         'RowHeight', [30, 150, 15, 150, 15 100, 15, 20, 15], ...
         'ColumnWidth', [100, 100, 100, 100, 100], ...
         'RowSpacing', 30), ...
         'PluginName', 'ntPassive', ...
         'VendorName', 'NT', ...
         'VendorVersion', '0.4.0', ...
         'InputChannels', 2, ...
         'OutputChannels', 2, ...
         'BackgroundImage', 'logo.png' ...
      );
   end
   methods
      function y = softClip(p, x, g, g_recipr)
         y = x;
         if ~p.clip_bypass
            x = x .* g;
            if x(1) > 1
               y(1) = 1;
            elseif x(1) < -1
               y(1) = -1;
            else
               y(1) = 1.5 .* x(1) - 0.5 .* x(1) .^ 3;
            end

            if x(2) > 1
               y(2) = 1;
            elseif x(2) < -1
               y(2) = -1;
            else
               y(2) = 1.5 .* x(2) - 0.5 .* x(2) .^ 3;
            end
            y = y .* g_recipr .* 0.6667;
         end
      end
      % function [y, new_state] = process_sample_filters(p, x, state)

      % end
      function y = process(p, x)
         x(~isfinite(x)) = 0;
         n = length(x);
         y = zeros(n, 2);
         if p.bypass
            y = x;
            return;
         end
         % Load filter states
         xn1_bp_lpf = p.state(1, :);
         yn1_bp_lpf = p.state(2, :);
         xn1_bp_hpf = p.state(3, :);
         yn1_bp_hpf = p.state(4, :);
         xn1_lpf = p.state(5, :);
         yn1_lpf = p.state(6, :);
         xn1_hpf = p.state(7, :);
         yn1_hpf = p.state(8, :);

         % Load delay line state
         i_st = p.state(9, 1);

         % Load coeffs for first order filters
         a_hpf = p.alpha_hpf;
         a_lpf = p.alpha_lpf;
         a_bp_hpf = p.alpha_bp_hpf;
         a_bp_lpf = p.alpha_bp_lpf;

         % Load gain coeffs.
         g = p.gain_out;
         g_lf = p.gain_lf;
         g_hf = p.gain_hf;
         g_mf = p.gain_mf;

         g_cl = p.gain_clip;
         g_cl_lf = p.gain_clip_lf;
         g_cl_mf = p.gain_clip_mf;
         g_cl_hf = p.gain_clip_hf;

         g_cl_recipr = 1 / g_cl;
         g_cl_lf_recipr = 1 / g_cl_lf;
         g_cl_mf_recipr = 1 / g_cl_mf;
         g_cl_hf_recipr = 1 / g_cl_hf;

         % Load coeffs for
         n_dl = p.n_delay_line;
         L = p.oversampling_factor;
         n_fir = p.n_coeffs_fir;

         % Load coeffs for FIR filters
         b_fir = p.b_coeffs_fir;

         for i = 1:n
            x_local = x(i, :);
            if p.bypass_oversampling
               % ------ FILTERS ------
               % LPF
               y_lpf = a_lpf .* x_local + (1 - a_lpf) .* yn1_lpf;
               xn1_lpf = x_local;
               yn1_lpf = y_lpf;
               y_lpf = y_lpf .* g_lf;
               y_lpf_cl = softClip(p, y_lpf, g_cl_lf, g_cl_lf_recipr);

               % HPF
               y_hpf = a_hpf .* (yn1_hpf + x_local - xn1_hpf);
               xn1_hpf = x_local;
               yn1_hpf = y_hpf;
               y_hpf = y_hpf .* g_hf;
               y_hpf_cl = softClip(p, y_hpf, g_cl_hf, g_cl_hf_recipr);

               % Band pass
               % Band Pass LPF
               y_bp_lpf = a_bp_lpf .* x_local + (1 - a_bp_lpf) .* yn1_bp_lpf;
               xn1_bp_lpf = x_local;
               yn1_bp_lpf = y_bp_lpf;

               % Band Pass HPF
               y_bp = a_bp_hpf .* (yn1_bp_hpf + y_bp_lpf - xn1_bp_hpf);
               xn1_bp_hpf = y_bp_lpf;
               yn1_bp_hpf = y_bp;
               y_bp = y_bp .* 2 .* g_mf;
               y_bp_cl = softClip(p, y_bp, g_cl_mf, g_cl_mf_recipr);
               % ------- END OF FILTERS -------

               % Sum and master soft clip
               y_sum = g .* (x_local + y_bp_cl + y_lpf_cl + y_hpf_cl);
               y(i, :) = softClip(p, y_sum, g_cl, g_cl_recipr);
            else
               % Oversampled version
               % Store twice
               p.interpolation_delay_line(i_st, :) = x_local;
               p.interpolation_delay_line(i_st + n_dl, :) = x_local;

               for j = 1:L - 1
                  % Fill in the zeros.
                  % TODO: This can be optimized out.
                  p.interpolation_delay_line(i_st + j, :) = 0;
                  p.interpolation_delay_line(i_st + n_dl + j, :) = 0;
               end

               % Calculate index for store to delay line. Skip oversampling_factor samples.
               % Delay line is in the high samplerate.
               i_st = i_st + L;
               if i_st > n_dl
                  i_st = 1;
               end

               accum = [0 0];
               if p.oversampling_uses_iir

               else
                  % Start at the second duplicate in delay line to avoid negative index.
                  start_idx = i_st + n_dl - n_fir;

                  % Loop over upsampled samples.
                  for j = 1:L

                     % Calculate start index for FIR filter.
                     fir_idx = start_idx + j - 1;

                     % Apply interpolation filter.
                     accum = [0 0];
                     for k = 1:n_fir
                        % TODO: This multiplies by 0 in 7 of 8 iterations in 8 times mode.
                        accum = accum + b_fir(k) .* p.interpolation_delay_line(fir_idx + k - 1, :);
                     end

                     x_eq = accum * L;

                     % ------ FILTERS ------
                     % LPF
                     y_lpf = a_lpf .* x_eq + (1 - a_lpf) .* yn1_lpf;
                     xn1_lpf = x_eq;
                     yn1_lpf = y_lpf;
                     y_lpf = y_lpf .* g_lf;
                     y_lpf_cl = softClip(p, y_lpf, g_cl_lf, g_cl_lf_recipr);

                     % HPF
                     y_hpf = a_hpf .* (yn1_hpf + x_eq - xn1_hpf);
                     xn1_hpf = x_eq;
                     yn1_hpf = y_hpf;
                     y_hpf = y_hpf .* g_hf;
                     y_hpf_cl = softClip(p, y_hpf, g_cl_hf, g_cl_hf_recipr);

                     % Band pass
                     % Band Pass LPF
                     y_bp_lpf = a_bp_lpf .* x_eq + (1 - a_bp_lpf) .* yn1_bp_lpf;
                     xn1_bp_lpf = x_eq;
                     yn1_bp_lpf = y_bp_lpf;

                     % Band Pass HPF
                     y_bp = a_bp_hpf .* (yn1_bp_hpf + y_bp_lpf - xn1_bp_hpf);
                     xn1_bp_hpf = y_bp_lpf;
                     yn1_bp_hpf = y_bp;
                     y_bp = y_bp .* 2 .* g_mf;
                     y_bp_cl = softClip(p, y_bp, g_cl_mf, g_cl_mf_recipr);
                     % ------- END OF FILTERS -------

                     % Sum and master soft clip
                     y_sum = g .* (x_eq + y_bp_cl + y_lpf_cl + y_hpf_cl);
                     y_filters = softClip(p, y_sum, g_cl, g_cl_recipr);

                     % Store to delay line for antialiasing filter.
                     p.antialiasing_delay_line(i_st + j - 1, :) = y_filters;
                     p.antialiasing_delay_line(i_st + j - 1 + n_dl, :) = y_filters;
                  end

                  % Apply antialiasing filter. Use the same coeffs for interpolation filter.
                  accum = [0 0];
                  for k = 1:n_fir
                     accum = accum + b_fir(k) .* p.antialiasing_delay_line(start_idx + k - 1, :);
                  end
               end

               % Store result
               y(i, :) = accum;
            end
            % Store states
            p.state(1, :) = xn1_bp_lpf;
            p.state(2, :) = yn1_bp_lpf;
            p.state(3, :) = xn1_bp_hpf;
            p.state(4, :) = yn1_bp_hpf;
            p.state(5, :) = xn1_lpf;
            p.state(6, :) = yn1_lpf;
            p.state(7, :) = xn1_hpf;
            p.state(8, :) = yn1_hpf;
            p.state(9, 1) = i_st;
         end
      end
      function update(p)
         fs_hi = p.fs * p.oversampling_factor;
         p.gain_lf = 10 ^ (p.g_lf_db / 20) - 1;
         z = 2 .* pi .* p.f_lf ./ fs_hi;
         p.alpha_lpf = z ./ (z + 1);
         p.gain_clip_lf = 10 ^ (p.g_clip_lf_db / 20);

         p.gain_mf = 10 ^ (p.g_mf_db / 20) - 1;
         z = 2 .* pi .* p.f_mf ./ fs_hi;
         p.alpha_bp_hpf = 1 ./ (z + 1);
         p.alpha_bp_lpf = z ./ (z + 1);
         p.gain_clip_mf = 10 ^ (p.g_clip_mf_db / 20);

         p.gain_hf = 10 ^ (p.g_hf_db / 20) - 1;
         z = 2 .* pi .* p.f_hf ./ fs_hi;
         p.alpha_hpf = 1 ./ (z + 1);
         p.gain_clip_hf = 10 ^ (p.g_clip_hf_db / 20);

         p.gain_out = 10 ^ (p.g_db / 20);
         p.gain_clip = 10 ^ (p.g_clip_db / 20);
         p.gain_clip_recipr = 1 / p.gain_clip;

      end
      function update_oversampling(p)
         switch p.oversampling_mode
            case oversamplingEnum.disable
               disable_oversampling(p);
            case oversamplingEnum.iir_2x
               enable_oversampling_iir(p, 2);
            case oversamplingEnum.iir_4x
               enable_oversampling_iir(p, 4);
            case oversamplingEnum.iir_8x
               enable_oversampling_iir(p, 8);
            case oversamplingEnum.fir_2x_lq
               enable_oversampling_fir(p, 2, false);
            case oversamplingEnum.fir_4x_lq
               enable_oversampling_fir(p, 4, false);
            case oversamplingEnum.fir_8x_lq
               enable_oversampling_fir(p, 8, false);
            case oversamplingEnum.fir_2x_hq
               enable_oversampling_fir(p, 2, true);
            case oversamplingEnum.fir_4x_hq
               enable_oversampling_fir(p, 4, true);
            case oversamplingEnum.fir_8x_hq
               enable_oversampling_fir(p, 8, true);
            otherwise
               disp('This should never ever happen');
         end
         p.state = zeros(9, 2);
         p.state(9, :) = 1;
         p.b_coeffs_fir = zeros(192, 1);
         p.interpolation_delay_line = zeros(192 * 2, 2);
         p.antialiasing_delay_line = zeros(192 * 2, 2);
         fs_hi = p.fs * p.oversampling_factor;
         p.n_coeffs_fir = p.oversampling_factor * p.oversampling_fir_mult;
         p.b_coeffs_fir(1:p.n_coeffs_fir) = windowMethod(22e3, p.n_coeffs_fir, fs_hi);
         update(p);
      end
      function reset(p)
         p.fs = getSampleRate(p);
         update_oversampling(p);
         update(p);
      end
      function enable_oversampling_fir(p, os, hq)
         p.bypass_oversampling = false;
         p.oversampling_uses_iir = false;
         p.oversampling_factor = os;
         if hq
            p.oversampling_fir_mult = p.oversampling_fir_mult_hq;
         else
            p.oversampling_fir_mult = p.oversampling_fir_mult_lq;
         end
      end
      function enable_oversampling_iir(p, os)
         p.bypass_oversampling = false;
         p.oversampling_uses_iir = true;
         p.oversampling_factor = os;
      end
      function disable_oversampling(p)
         p.bypass_oversampling = true;
      end
      function set.g_lf_db(p, val)
         p.g_lf_db = val;
         update(p);
      end
      function set.g_mf_db(p, val)
         p.g_mf_db = val;
         update(p);
      end
      function set.g_hf_db(p, val)
         p.g_hf_db = val;
         update(p);
      end
      function set.f_lf(p, val)
         p.f_lf = val;
         update(p);
      end
      function set.f_mf(p, val)
         p.f_mf = val;
         update(p);
      end
      function set.f_hf(p, val)
         p.f_hf = val;
         update(p);
      end
      function set.g_db(p, val)
         p.g_db = val;
         update(p);
      end
      function set.g_clip_db(p, val)
         p.g_clip_db = val;
         update(p);
      end
      function set.g_clip_lf_db(p, val)
         p.g_clip_lf_db = val;
         update(p);
      end
      function set.g_clip_mf_db(p, val)
         p.g_clip_mf_db = val;
         update(p);
      end
      function set.g_clip_hf_db(p, val)
         p.g_clip_hf_db = val;
         update(p);
      end
      function set.oversampling_mode(p, val)
         p.oversampling_mode = val;
         update_oversampling(p);
      end
   end
end
