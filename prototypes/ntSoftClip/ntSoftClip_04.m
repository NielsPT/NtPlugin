classdef ntSoftClip_04 < audioPlugin
   % Oversampling, asymmetrically soft clipping plugin
   properties
      bypass = false;
      oversampling_mode = oversamplingEnum.iir_2x;
      g_db = 0;
      g_clip_db = 0;
      softness_procent = 70;
      symmetry_procent = 100;
   end
   properties (Access = private)
      fs = 44100;
      bypass_oversampling = false;
      oversampling_uses_iir = false;

      gain_out = 0;
      gain_clip_up = 1;
      gain_clip_dn = 1;
      oversampling_factor = 8;
      n_coeffs_fir = 1;
      b_coeffs_fir = zeros(192, 1);
      oversampling_fir_mult = 1;
      alpha_softClip = 0.5;

      % The delay line is double length for dual storage.
      interpolation_delay_line = zeros(192 * 2, 2);
      antialiasing_delay_line = zeros(192 * 2, 2);

      % The only state_fir is the current location in the delay lines.
      state_fir = 1;

      b_coeffs_iir = zeros(3, 4);
      a_coeffs_iir = zeros(2, 4);
      q_iir = [0.50979558;
               0.60134489;
               0.89997622;
               2.5629154];
      b0_coeff_iir = 1;

      % TODO: A single state array instead.
      state_x_iir1 = zeros(2, 2);
      state_y_iir1 = zeros(8, 2);
      state_x_iir2 = zeros(2, 2);
      state_y_iir2 = zeros(8, 2);
   end
   properties (Constant)
      n_bq_stages = 4;
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
         audioPluginParameter('symmetry_procent', ...
         'DisplayName', 'Symmetry', ...
         'Mapping', {'lin', 50, 100}, ...
         'Label', '%', ...
         'Style', 'rotary', ...
         'Layout', [2, 2]), ...
         audioPluginParameter('softness_procent', ...
         'DisplayName', 'Softness', ...
         'Mapping', {'lin', 0, 100}, ...
         'Label', '%', ...
         'Style', 'rotary', ...
         'Layout', [2, 3]), ...
         audioPluginParameter('g_db', ...
         'DisplayName', 'Trim', ...
         'Mapping', {'lin', -24, 24}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [4, 2]), ...
         audioPluginParameter('bypass', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 3], ...
         'DisplayName', 'Master Bypass'), ...
         audioPluginParameter('oversampling_mode', ...
         'DisplayName', 'Oversampling', ...
         'Mapping', {'enum', 'Off', '2 x IIR', '4 x IIR', '8 x IIR', ...
            '2 x FIR LQ', '4 x FIR LQ', '8 x FIR LQ', ...
            '2 x FIR HQ', '4 x FIR HQ', '8 x FIR HQ', ...
         }, ...
         'Label', '', ...
         'Style', 'dropdown', ...
         'Layout', [6, 1]), ...
         audioPluginGridLayout( ...
         'RowHeight', [30, 150, 15, 100, 15, 20, 15], ...
         'ColumnWidth', [100, 100, 100], ...
         'RowSpacing', 30), ...
         'PluginName', 'ntSoftClip', ...
         'VendorName', 'NT', ...
         'VendorVersion', '0.4.0', ...
         'InputChannels', 2, ...
         'OutputChannels', 2, ...
         'BackgroundImage', 'logo.png' ...
      );
   end
   methods (Static)
      function y = soft_clip_asym(x, g_up, g_up_recipr, g_dn, g_dn_recipr, alpha)
         y = x;
         if x(1) > 0
            x(1) = x(1) .* g_up;
         else
            x(1) = x(1) .* g_dn;
         end

         if x(2) > 0
            x(2) = x(2) .* g_up;
         else
            x(2) = x(2) .* g_dn;
         end

         if x(1) > 1
            y(1) = 1;
         elseif x(1) < -1
            y(1) = -1;
         else
            y(1) = (1 + alpha) .* x(1) - alpha .* x(1) .^ 3;
         end

         if x(2) > 1
            y(2) = 1;
         elseif x(2) < -1
            y(2) = -1;
         else
            y(2) = (1 + alpha) .* x(2) - alpha .* x(2) .^ 3;
         end

         if x > 0
            y = y .* g_up_recipr .* 0.6667;
         else
            y = y .* g_dn_recipr .* 0.6667;
         end
      end
   end
   methods
      function y = process(p, x)
         x(~isfinite(x)) = 0;
         n = length(x);
         y = zeros(n, 2);
         i_st = p.state_fir;
         g = p.gain_out;
         g_cl_up = p.gain_clip_up;
         g_cl_dn = p.gain_clip_dn;
         g_cl_up_recipr = 1 / g_cl_up;
         g_cl_dn_recipr = 1 / g_cl_dn;
         n_dl = p.n_delay_line;
         L = p.oversampling_factor;
         n_fir = p.n_coeffs_fir;
         b_fir = p.b_coeffs_fir;
         alpha = p.alpha_softClip;

         % Copy coefficients for iir filter
         b_iir = p.b_coeffs_iir;
         a_iir = p.a_coeffs_iir;
         b0_iir = p.b0_coeff_iir;

         % Copy state for iir filter
         xn_iir1 = p.state_x_iir1;
         yn_iir1 = p.state_y_iir1;
         xn_iir2 = p.state_x_iir2;
         yn_iir2 = p.state_y_iir2;

         xn_iir1(~isfinite(xn_iir1)) = 0;
         yn_iir1(~isfinite(yn_iir1)) = 0;
         xn_iir2(~isfinite(xn_iir2)) = 0;
         yn_iir2(~isfinite(yn_iir2)) = 0;

         y_iir1_max = 0;
         y_iir2_max = 0;
         x_max = 0;

         if p.bypass
            y = x;
            return;
         end
         % TODO: move loops into branches of ifs so we check per buffer, not per sample.
         for i = 1:n
            x_local = x(i, :);
            x_abs = abs(x_local(1));
            if x_abs > x_max
               x_max = x_abs;
            end
            if p.bypass_oversampling
               y(i, :) = p.soft_clip_asym(x_local, g_cl_up, g_cl_up_recipr, g_cl_dn, g_cl_dn_recipr, alpha) .* g;
               continue;
            end

            if p.oversampling_uses_iir
               % Loop over samples at high samplerate.
               for j = 1:L
                  % Process first stage
                  accum = (x_local .* b_iir(1, 1) ...
                     + b_iir(2, 1) .* xn_iir1(1, :) ...
                     + b_iir(3, 1) .* xn_iir1(2, :) ...
                     - a_iir(1, 1) .* yn_iir1(1, :) ...
                     - a_iir(2, 1) .* yn_iir1(2, :));

                  % Store feed forward state for first stage.
                  xn_iir1(2, :) = xn_iir1(1, :);
                  xn_iir1(1, :) = x_local;

                  % Reset x_local, so that we input 0 for the rest of the iterations.
                  x_local = [0 0];

                  % Set input for following stages
                  x_iir1 = accum;

                  % Process remaining stages.
                  for k = 2:4
                     accum = (x_iir1 .* b_iir(1, k) ...
                        + b_iir(2, k) .* yn_iir1((k - 2) * 2 + 1, :) ...
                        + b_iir(3, k) .* yn_iir1((k - 2) * 2 + 2, :) ...
                        - a_iir(1, k) .* yn_iir1((k - 1) * 2 + 1, :) ...
                        - a_iir(2, k) .* yn_iir1((k - 1) * 2 + 2, :));

                     % Update feedback state for previous stage.
                     yn_iir1((k - 2) * 2 + 2, :) = yn_iir1((k - 2) * 2 + 1, :);
                     yn_iir1((k - 2) * 2 + 1, :) = x_iir1;

                     % Store result as input for next stage
                     x_iir1 = accum;
                  end

                  % Update feedback state for last stage
                  yn_iir1(8, :) = yn_iir1(7, :);
                  yn_iir1(7, :) = accum;

                  y_iir1 = accum .* L; % .* b0_iir
                  y_abs = abs(y_iir1(1));
                  if y_abs > y_iir1_max
                     y_iir1_max = y_abs;
                  end

                  % Apply effect
                  % Clip accumulated value.
                  acc_cl = p.soft_clip_asym(y_iir1, g_cl_up, g_cl_up_recipr, g_cl_dn, g_cl_dn_recipr, alpha);

                  % Apply antialiasing filter
                  % Process first stage
                  accum = (acc_cl .* b_iir(1, 1) ...
                     + b_iir(2, 1) .* xn_iir2(1, :) ...
                     + b_iir(3, 1) .* xn_iir2(2, :) ...
                     - a_iir(1, 1) .* yn_iir2(1, :) ...
                     - a_iir(2, 1) .* yn_iir2(2, :));

                  % Store feed forward state for first stage.
                  xn_iir2(2, :) = xn_iir2(1, :);
                  xn_iir2(1, :) = acc_cl;

                  % Set input for following stages
                  x_iir2 = accum;

                  % Process remaining stages.
                  for k = 2:4
                     accum = (x_iir2 .* b_iir(1, k) ...
                        + b_iir(2, k) .* yn_iir2((k - 2) * 2 + 1, :) ...
                        + b_iir(3, k) .* yn_iir2((k - 2) * 2 + 2, :) ...
                        - a_iir(1, k) .* yn_iir2((k - 1) * 2 + 1, :) ...
                        - a_iir(2, k) .* yn_iir2((k - 1) * 2 + 2, :));

                     % Update feedback state for previous stage.
                     yn_iir2((k - 2) * 2 + 2, :) = yn_iir2((k - 2) * 2 + 1, :);
                     yn_iir2((k - 2) * 2 + 1, :) = x_iir2;

                     % Store result as input for next stage
                     x_iir2 = accum;
                  end

                  % Update feedback state for last stage
                  yn_iir2(8, :) = yn_iir2(7, :);
                  yn_iir2(7, :) = accum;

                  y_iir2 = accum .* b0_iir;
                  y_abs = abs(y_iir2(1));
                  if y_abs > y_iir2_max
                     y_iir2_max = y_abs;
                  end
               end

               % Down sample. Any of the L samples will do for now.
               % TODO: select the first sample. Or does it matter?
               y(i, :) = y_iir2;
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
               acc_cl = p.soft_clip_asym(accum .* L, g_cl_up, g_cl_up_recipr, g_cl_dn, g_cl_dn_recipr, alpha);

               % Store to delay line for antialiasing filter.
               p.antialiasing_delay_line(i_st + j - 1, :) = acc_cl;
               p.antialiasing_delay_line(i_st + j - 1 + n_dl, :) = acc_cl;
            end

            % Apply antialiasing filter. Use the same coeffs for interpolation filter.
            accum = [0 0];
            for k = 1:p.n_coeffs_fir
               accum = accum + b_fir(k) .* p.antialiasing_delay_line(start_idx + k - 1);
            end

            % Store result
            y(i, :) = accum .* g;
         end
         % Store state for iir filter for next round
         p.state_x_iir1 = xn_iir1;
         p.state_y_iir1 = yn_iir1;
         p.state_x_iir2 = xn_iir2;
         p.state_y_iir2 = yn_iir2;

         p.state_fir = i_st;

         % disp("x: " + x_max + ", y1_max: " + y_iir1_max + ", y2_max: " + y_iir2_max);
      end
      function update(p)
         p.gain_out = 10 ^ (p.g_db / 20);
         disp("updating soft clipper")
         gain_clip = 10 ^ (p.g_clip_db / 20);
         sym = (p.symmetry_procent + 1) / 100;
         p.gain_clip_up = gain_clip;
         if p.symmetry_procent == 100
            p.gain_clip_dn = gain_clip;
         else
            p.gain_clip_dn = gain_clip * sym;
         end
         p.alpha_softClip = 0.5 - ((100 - p.softness_procent) / 200);
      end
      function update_oversampling(p)
         disp("updating oversampling")
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

         disp("updating iir filter")
         fs_hi = p.fs * p.oversampling_factor;
         b = zeros(3, 4);
         a = zeros(3, 4);

         for i = 1:p.n_bq_stages
            [b(:, i), a(:, i)] = makeLPF(fs_hi, 22e3, p.q_iir(i));
         end
         b
         a
         % b0_prod = prod(b(1, :));
         % p.b0_coeff_iir = b0_prod;
         p.b_coeffs_iir = b; % b(2:3, :);
         p.a_coeffs_iir = a(2:3, :);
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
      function set.softness_procent(p, val)
         p.softness_procent = val;
         update(p);
      end
      function set.symmetry_procent(p, val)
         p.symmetry_procent = val;
         update(p);
      end
   end
end
