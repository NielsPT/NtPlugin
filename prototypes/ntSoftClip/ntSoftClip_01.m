classdef ntSoftClip_01 < audioPlugin
   properties
      bypass = false;
      oversampling_mode = oversamplingEnum.fir_4x_lq;
      g_db = 0;
      g_clip_db = 0;
   end
   properties (Access = private)
      bypass_oversampling = false;
      oversampling_uses_iir = false;

      gain_out = 0;
      gain_clip = 1;
      oversampling_factor = 8;
      n_coeffs_fir = 1;
      b_coeffs_fir = zeros(192, 1);
      oversampling_fir_mult = 1;
      fs = 44100;

      % The delay line is double length for dual storage.
      interpolation_delay_line = zeros(192 * 2, 2);
      antialiasing_delay_line = zeros(192 * 2, 2);

      % The only state is the current location in the delay lines.
      state = 1;
   end
   properties (Constant)
      n_delay_line = 192;
      oversampling_fir_mult_lq = 12;
      oversampling_fir_mult_hq = 24;

      PluginInterface = audioPluginInterface( ...
         audioPluginParameter('g_clip_db', ...
         'DisplayName', 'Wide band drive', ...
         'Mapping', {'lin', -40, 40}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [2, 1]), ...
         audioPluginParameter('g_db', ...
         'DisplayName', 'Trim', ...
         'Mapping', {'lin', -24, 0}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [2, 2]), ...
         audioPluginParameter('bypass', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [2, 3], ...
         'DisplayName', 'Master Bypass'), ...
         audioPluginParameter('oversampling_mode', ...
         'DisplayName', 'Oversampling', ...
         'Mapping', {'enum', 'Off', '2 x IIR', '4 x IIR', '8 x IIR', ...
            '2 x FIR LQ', '4 x FIR LQ', '8 x FIR LQ', ...
            '2 x FIR HQ', '4 x FIR HQ', '8 x FIR HQ', ...
         }, ...
         'Label', '', ...
         'Style', 'dropdown', ...
         'Layout', [4, 1]), ...
         audioPluginGridLayout( ...
         'RowHeight', [30, 100, 15, 20, 15], ...
         'ColumnWidth', [100, 100, 100], ...
         'RowSpacing', 30), ...
         'PluginName', 'ntSoftClip_01', ...
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
         i_st = p.state;
         g = p.gain_out;
         g_cl = p.gain_clip;
         g_cl_recipr = 1 / g_cl;
         n_dl = p.n_delay_line;
         L = p.oversampling_factor;
         n_fir = p.n_coeffs_fir;
         b_fir = p.b_coeffs_fir;

         if p.bypass
            y = x;
            return;
         end
         for i = 1:n
            x_local = x(i, :);
            if p.bypass_oversampling
               y(i, :) = softClip(p, x_local, g_cl, g_cl_recipr) .* g;
               continue;
            end

            % Store twice
            p.interpolation_delay_line(i_st, :) = x(i, :);
            p.interpolation_delay_line(i_st + p.n_delay_line, :) = x(i, :);

            for j = 2:p.oversampling_factor
               % Fill in the zeros.
               % TODO: This can be optimized out.
               p.interpolation_delay_line(i_st + j - 1, :) = 0;
               p.interpolation_delay_line(i_st + p.n_delay_line + j - 1, :) = 0;
            end

            % Calculate index for store to delay line. Skip oversampling_factor samples.
            % Delay line is in the high samplerate.
            i_st = i_st + p.oversampling_factor;
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
                     accum = accum + b_fir(k) .* p.interpolation_delay_line(fir_idx + k - 1);
                  end

                  % Clip accumulated value.
                  acc_cl = softClip(p, accum .* L, g_cl, g_cl_recipr);

                  % Store to delay line for antialiasing filter.
                  p.antialiasing_delay_line(i_st + j - 1, :) = acc_cl;
                  p.antialiasing_delay_line(i_st + j - 1 + n_dl, :) = acc_cl;
               end

               % Apply antialiasing filter. Use the same coeffs for interpolation filter.
               accum = [0 0];
               for k = 1:p.n_coeffs_fir
                  accum = accum + b_fir(k) .* p.antialiasing_delay_line(start_idx + k - 1);
               end
            end

            % Store result
            y(i, :) = accum .* g;
            p.state = i_st;
         end
      end
      function y = softClip(p, x, g, g_recipr)
         y = x;
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
      function update(p)
         p.gain_out = 10 ^ (p.g_db / 20);
         p.gain_clip = 10 ^ (p.g_clip_db / 20);
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
         p.b_coeffs_fir = zeros(192, 1);
         p.interpolation_delay_line = zeros(192 * 2, 2);
         p.antialiasing_delay_line = zeros(192 * 2, 2);
         fs_hi = p.fs * p.oversampling_factor;
         p.n_coeffs_fir = p.oversampling_factor * p.oversampling_fir_mult;
         p.b_coeffs_fir(1:p.n_coeffs_fir) = windowMethod(22e3, p.n_coeffs_fir, fs_hi);
         update(p);
      end
      function enable_oversampling_fir(p, oversampling_factor, hq)
         p.bypass_oversampling = false;
         p.oversampling_factor = oversampling_factor;
         if hq
            p.oversampling_fir_mult = p.oversampling_fir_mult_hq;
         else
            p.oversampling_fir_mult = p.oversampling_fir_mult_lq;
         end
         p.oversampling_uses_iir = false;
      end
      function enable_oversampling_iir(p, oversampling_factor)
         p.bypass_oversampling = false;
         p.oversampling_factor = oversampling_factor;
         p.oversampling_uses_iir = true;
      end
      function disable_oversampling(p)
         p.bypass_oversampling = true;
      end
      function reset(p)
         p.fs = getSampleRate(p);
         update_oversampling(p);
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
      function set.bypass(p, val)
         p.bypass = val;
         update(p);
      end
      function set.oversampling_mode(p, val)
         p.oversampling_mode = val;
         update_oversampling(p);
      end
   end
end
