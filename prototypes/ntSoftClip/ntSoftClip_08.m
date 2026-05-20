classdef ntSoftClip_08 < audioPlugin
   % Oversampling, asymmetrically soft clipping plugin
   properties
      bypass = false;
      oversampling_mode = oversamplingEnum.iir_2x;
      g_db = 0;
      g_clip_db = 0;
      symmetry_procent = 100;
      dc_offset = 0;
      square_clip = true;
   end
   properties (Access = private)
      fs = 44100;
      bypass_oversampling = false;
      oversampling_uses_iir = false;
      f_hpf = 20;
      alpha_hpf = 0;

      gain_out = 0;
      gain_clip_up = 1;
      gain_clip_dn = 1;

      oversampling_factor = 8;
      n_coeffs_fir = 1;
      b_coeffs_fir = zeros(192, 1);
      oversampling_fir_mult = 1;

      % The delay line is double length for dual storage.
      interpolation_delay_line = zeros(192 * 2, 2);
      antialiasing_delay_line = zeros(192 * 2, 2);

      % The only state_fir is the current location in the delay lines.
      state_fir = [1 1];

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

      state = zeros(2, 2);
   end
   properties (Constant)
      n_bq_stages = 4;
      n_delay_line_interpolation = 192;
      n_delay_line_antialiasing = 192;
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
         audioPluginParameter('dc_offset', ...
         'DisplayName', 'DC Offset', ...
         'Mapping', {'lin', -1, 1}, ...
         'Label', '', ...
         'Style', 'rotary', ...
         'Layout', [2, 4]), ...
         audioPluginParameter('square_clip', ...
         'Mapping', {'enum', '3rd', '2nd'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 1], ...
         'DisplayName', 'Order'), ...
         audioPluginParameter('g_db', ...
         'DisplayName', 'Trim', ...
         'Mapping', {'lin', -24, 24}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [4, 3]), ...
         audioPluginParameter('bypass', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 4], ...
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
         'ColumnWidth', [100, 100, 100, 100], ...
         'RowSpacing', 30), ...
         'PluginName', 'ntSoftClip', ...
         'VendorName', 'NT', ...
         'VendorVersion', '0.8.0', ...
         'InputChannels', 2, ...
         'OutputChannels', 2, ...
         'BackgroundImage', 'logo.png' ...
      );
   end
   methods (Static)
      function y = soft_clip_2nd_simple(x)
         x_dc = 0.5 .* x + 0.5;
         if x_dc > 1
            y_dc = 1;
         elseif x_dc < 0
            y_dc = 0;
         else
            y_dc = 2 .* x_dc - x_dc .^ 2;
         end
         y = 2 .* y_dc - 1;
      end
      function y = soft_clip_3rd_simple(x)
         if x > 1
            y = 0.6667;
         elseif x < -1
            y = -0.6667;
         else
            y = (1.35 * x - 0.35 * x ^ 3) .* 0.6667;
         end
      end
   end
   methods
      function y = process(p, x)
         x(~isfinite(x)) = 0;
         if p.bypass
            y = x;
            return;
         end
         if p.bypass_oversampling
            y = process_direct(p, x);
            return;
         end
         if p.oversampling_uses_iir
            y = process_iir(p, x);
            return;
         end
         y = process_fir(p, x);
      end
      function y = process_direct(p, x)
         n = length(x);
         y = zeros(n, 2);
         g = p.gain_out;
         g_cl_up = p.gain_clip_up;
         g_cl_dn = p.gain_clip_dn;
         g_cl_up_recipr = 1 / g_cl_up;
         g_cl_dn_recipr = 1 / g_cl_dn;
         dc = p.dc_offset;
         xn1_hpf = p.state(1, :);
         yn1_hpf = p.state(2, :);
         a_hpf = p.alpha_hpf;

         for i = 1:n
            x_cl = x(i, :);
            x_asym = [0 0];
            if x_cl(1) > 0
               x_asym(1) = x_cl(1) .* g_cl_up;
            else
               x_asym(1) = x_cl(1) .* g_cl_dn;
            end
            if x_cl(2) > 0
               x_asym(2) = x_cl(2) .* g_cl_up;
            else
               x_asym(2) = x_cl(2) .* g_cl_dn;
            end
            x_dc = x_asym + dc;
            y_dc = [0 0];
            if p.square_clip
               y_dc(1) = p.soft_clip_2nd_simple(x_dc(1));
               y_dc(2) = p.soft_clip_2nd_simple(x_dc(2));
            else
               y_dc(1) = p.soft_clip_3rd_simple(x_dc(1));
               y_dc(2) = p.soft_clip_3rd_simple(x_dc(2));
            end
            x_asym_recipr = y_dc - dc;
            y_asym = [0 0];
            if x_asym_recipr(1) > 0
               y_asym(1) = x_asym_recipr(1) .* g_cl_up_recipr;
            else
               y_asym(1) = x_asym_recipr(1) .* g_cl_dn_recipr;
            end
            if x_asym_recipr(2) > 0
               y_asym(2) = x_asym_recipr(2) .* g_cl_up_recipr;
            else
               y_asym(2) = x_asym_recipr(2) .* g_cl_dn_recipr;
            end
            x_hpf = y_asym;
            y_hpf = a_hpf .* (yn1_hpf + x_hpf - xn1_hpf);
            xn1_hpf = x_hpf;
            yn1_hpf = y_hpf;
            y(i, :) = y_asym .* g;
         end

         p.state(1, :) = xn1_hpf;
         p.state(2, :) = yn1_hpf;
      end
      function y = process_iir(p, x)
         n = length(x);
         y = zeros(n, 2);
         g = p.gain_out;
         g_cl_up = p.gain_clip_up;
         g_cl_dn = p.gain_clip_dn;
         g_cl_up_recipr = 1 / g_cl_up;
         g_cl_dn_recipr = 1 / g_cl_dn;
         L = p.oversampling_factor;
         dc = p.dc_offset;

         % DC-blocker
         xn1_hpf = p.state(1, :);
         yn1_hpf = p.state(2, :);
         a_hpf = p.alpha_hpf;

         % Copy coefficients for iir filter
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

         for i = 1:n
            x_local = x(i, :);

            % Loop over samples at high samplerate.
            for j = 1:L
               % Process first stage
               accum = (x_local ... %.* b_iir(1, 1) ...
                  + 2 .* xn_iir1(1, :) ...
                  + xn_iir1(2, :) ...
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
                  accum = (x_iir1 ...
                     + 2 .* yn_iir1((k - 2) * 2 + 1, :) ...
                     + yn_iir1((k - 2) * 2 + 2, :) ...
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

               y_iir1 = accum .* L .* b0_iir;

               % Apply effect
               % Clip accumulated value.
               x_cl = y_iir1;
               x_asym = [0 0];
               if x_cl(1) > 0
                  x_asym(1) = x_cl(1) .* g_cl_up;
               else
                  x_asym(1) = x_cl(1) .* g_cl_dn;
               end
               if x_cl(2) > 0
                  x_asym(2) = x_cl(2) .* g_cl_up;
               else
                  x_asym(2) = x_cl(2) .* g_cl_dn;
               end
               x_dc = x_asym + dc;
               y_dc = [0 0];
               if p.square_clip
                  y_dc(1) = p.soft_clip_2nd_simple(x_dc(1));
                  y_dc(2) = p.soft_clip_2nd_simple(x_dc(2));
               else
                  y_dc(1) = p.soft_clip_3rd_simple(x_dc(1));
                  y_dc(2) = p.soft_clip_3rd_simple(x_dc(2));
               end
               x_asym_recipr = y_dc - dc;
               y_asym = [0 0];
               if x_asym_recipr(1) > 0
                  y_asym(1) = x_asym_recipr(1) .* g_cl_up_recipr;
               else
                  y_asym(1) = x_asym_recipr(1) .* g_cl_dn_recipr;
               end
               if x_asym_recipr(2) > 0
                  y_asym(2) = x_asym_recipr(2) .* g_cl_up_recipr;
               else
                  y_asym(2) = x_asym_recipr(2) .* g_cl_dn_recipr;
               end
               x_hpf = y_asym;
               y_hpf = a_hpf .* (yn1_hpf + x_hpf - xn1_hpf);
               xn1_hpf = x_hpf;
               yn1_hpf = y_hpf;
               y_cl = y_asym .* g;

               % Apply antialiasing filter
               % Process first stage
               accum = (y_cl ... %.* b_iir(1, 1) ...
                  + 2 .* xn_iir2(1, :) ...
                  + xn_iir2(2, :) ...
                  - a_iir(1, 1) .* yn_iir2(1, :) ...
                  - a_iir(2, 1) .* yn_iir2(2, :));

               % Store feed forward state for first stage.
               xn_iir2(2, :) = xn_iir2(1, :);
               xn_iir2(1, :) = y_cl;

               % Set input for following stages
               x_iir2 = accum;

               % Process remaining stages.
               for k = 2:4
                  accum = (x_iir2 ...
                     + 2 .* yn_iir2((k - 2) * 2 + 1, :) ...
                     + yn_iir2((k - 2) * 2 + 2, :) ...
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
            end

            % Down sample. Any of the L samples will do for now.
            % TODO: select the first sample. Or does it matter?
            y(i, :) = accum .* g .* b0_iir;
         end

         % Store state for iir filter for next round
         p.state_y_iir1 = yn_iir1;
         p.state_x_iir1 = xn_iir1;
         p.state_y_iir2 = yn_iir2;
         p.state_x_iir2 = xn_iir2;

         % State for DC blocker
         p.state(1, :) = xn1_hpf;
         p.state(2, :) = yn1_hpf;
      end
      function y = process_fir(p, x)
         n = length(x);
         y = zeros(n, 2);
         g = p.gain_out;
         g_cl_up = p.gain_clip_up;
         g_cl_dn = p.gain_clip_dn;
         g_cl_up_recipr = 1 / g_cl_up;
         g_cl_dn_recipr = 1 / g_cl_dn;
         dc = p.dc_offset;

         % DC-blocker
         xn1_hpf = p.state(1, :);
         yn1_hpf = p.state(2, :);
         a_hpf = p.alpha_hpf;
         i_store_in = p.state_fir(1);
         i_store_out = p.state_fir(2);

         n_dl_in = p.n_delay_line_interpolation;
         n_dl_out = p.n_delay_line_antialiasing;
         L = p.oversampling_factor;
         M = p.oversampling_fir_mult;
         b_fir = p.b_coeffs_fir;
         n_fir = p.n_coeffs_fir;

         for i = 1:n
            % Store twice
            p.interpolation_delay_line(i_store_in, :) = x(i, :);
            p.interpolation_delay_line(i_store_in + n_dl_in, :) = x(i, :);

            % Calculate index for store to delay line. Skip oversampling_factor samples.
            % Delay line is in the high samplerate.
            i_store_in = i_store_in + 1;
            if i_store_in > n_dl_in
               i_store_in = 1;
            end

            % Start at the second duplicate in delay line to avoid negative index.
            i_read_in = i_store_in + n_dl_in;

            % Loop over upsampled samples.
            for j = 1:L

               % Apply interpolation filter.
               accum = [0 0];
               for k = 1:M
                  accum = accum + b_fir((k - 1) * L + j) .* p.interpolation_delay_line(i_read_in - k + 1, :);
               end

               y_fir = accum .* L;

               % Clip accumulated value.
               x_cl = y_fir;
               x_asym = [0 0];
               if x_cl(1) > 0
                  x_asym(1) = x_cl(1) .* g_cl_up;
               else
                  x_asym(1) = x_cl(1) .* g_cl_dn;
               end
               if x_cl(2) > 0
                  x_asym(2) = x_cl(2) .* g_cl_up;
               else
                  x_asym(2) = x_cl(2) .* g_cl_dn;
               end
               x_dc = x_asym + dc;
               y_dc = [0 0];
               if p.square_clip
                  y_dc(1) = p.soft_clip_2nd_simple(x_dc(1));
                  y_dc(2) = p.soft_clip_2nd_simple(x_dc(2));
               else
                  y_dc(1) = p.soft_clip_3rd_simple(x_dc(1));
                  y_dc(2) = p.soft_clip_3rd_simple(x_dc(2));
               end
               x_asym_recipr = y_dc - dc;
               y_asym = [0 0];
               if x_asym_recipr(1) > 0
                  y_asym(1) = x_asym_recipr(1) .* g_cl_up_recipr;
               else
                  y_asym(1) = x_asym_recipr(1) .* g_cl_dn_recipr;
               end
               if x_asym_recipr(2) > 0
                  y_asym(2) = x_asym_recipr(2) .* g_cl_up_recipr;
               else
                  y_asym(2) = x_asym_recipr(2) .* g_cl_dn_recipr;
               end
               x_hpf = y_asym;
               y_hpf = a_hpf .* (yn1_hpf + x_hpf - xn1_hpf);
               xn1_hpf = x_hpf;
               yn1_hpf = y_hpf;
               y_cl = y_asym .* g;

               % Store to delay line for antialiasing filter.
               p.antialiasing_delay_line(i_store_out, :) = y_cl;
               p.antialiasing_delay_line(i_store_out + n_dl_out, :) = y_cl;

               i_store_out = i_store_out + 1;
               if i_store_out > n_dl_out
                  i_store_out = 1;
               end
            end

            % Apply antialiasing filter. Use the same coeffs for interpolation filter.
            i_read_out = i_store_out + n_dl_out - n_fir;
            accum = [0 0];
            for k = 1:p.n_coeffs_fir
               accum = accum + b_fir(k) .* p.antialiasing_delay_line(i_read_out + k - 1, :);
            end

            % Store result
            y(i, :) = accum .* g;
         end

         % State for DC blocker
         p.state(1, :) = xn1_hpf;
         p.state(2, :) = yn1_hpf;

         % Store locations in delay lines
         p.state_fir(1) = i_store_in;
         p.state_fir(2) = i_store_out;
      end
      function update(p)
         p.gain_out = 10 ^ (p.g_db / 20);
         gain_clip = 10 ^ (p.g_clip_db / 20);
         sym = (p.symmetry_procent + 1) / 100;
         p.gain_clip_up = gain_clip;
         if p.symmetry_procent == 100
            p.gain_clip_dn = gain_clip;
         else
            p.gain_clip_dn = gain_clip * sym;
         end
         z = 2 .* pi .* p.f_hpf ./ p.fs;
         p.alpha_hpf = 1 ./ (z + 1);
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
            [b(:, i), a(:, i)] = makeLPF(fs_hi, 20e3, p.q_iir(i));
         end
         b0_prod = prod(b(1, :));
         p.b0_coeff_iir = b0_prod;
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
      function set.symmetry_procent(p, val)
         p.symmetry_procent = val;
         update(p);
      end
      function set.dc_offset(p, val)
         p.dc_offset = val;
         update(p);
      end
   end
end
