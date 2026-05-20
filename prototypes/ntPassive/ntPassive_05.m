classdef ntPassive_05 < audioPlugin
   properties
      bypass = false;
      bypass_clip = false;
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
      symmetry_procent = 100;

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

      gain_clip_up = 1;
      gain_clip_dn = 1;

      state = zeros(10, 2);

      bypass_oversampling = false;
      oversampling_uses_iir = false;
      oversampling_factor = 8;
      n_coeffs_fir = 1;
      b_coeffs_fir = zeros(192, 1);
      oversampling_fir_mult = 1;
      alpha_softClip = 0.5;

      % The delay line is double length for dual storage.
      interpolation_delay_line = zeros(192 * 2, 2);
      antialiasing_delay_line = zeros(192 * 2, 2);

      % The only state_fir is the current location in the delay lines.
      state_fir = [1 1];

      b_coeffs_iir = zeros(2, 4);
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
      n_delay_line_interpolation = 192;
      n_delay_line_antialiasing = 192;
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
         audioPluginParameter('symmetry_procent', ...
         'DisplayName', 'Symmetry', ...
         'Mapping', {'lin', 50, 100}, ...
         'Label', '%', ...
         'Style', 'rotary', ...
         'Layout', [2, 4]), ...
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
         audioPluginParameter('bypass_clip', ...
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
   end
   methods
      function y = softClip(p, x, g, g_recipr)
         y = x;
         % if ~p.bypass_clip
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
         % end
      end
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

         % Load filter states
         xn1_bp_lpf = p.state(1, :);
         yn1_bp_lpf = p.state(2, :);
         xn1_bp_hpf = p.state(3, :);
         yn1_bp_hpf = p.state(4, :);
         xn1_lpf = p.state(5, :);
         yn1_lpf = p.state(6, :);
         xn1_hpf = p.state(7, :);
         yn1_hpf = p.state(8, :);

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

         g_cl_up = p.gain_clip_up;
         g_cl_dn = p.gain_clip_dn;
         g_cl_up_recipr = 1 / g_cl_up;
         g_cl_dn_recipr = 1 / g_cl_dn;

         for i = 1:n
            x_local = x(i, :);
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
            if p.bypass_clip
               y_fx = (x_local + y_bp + y_lpf + y_hpf) .* g;
            else
               y_sum = x_local + y_bp_cl + y_lpf_cl + y_hpf_cl;
               y_fx = softClip(p, y_sum, g_cl, g_cl_recipr);
            end

            y(i, :) = y_fx;
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
      end
      function y = process_iir(p, x)
         n = length(x);
         y = zeros(n, 2);

         % Load filter states
         xn1_bp_lpf = p.state(1, :);
         yn1_bp_lpf = p.state(2, :);
         xn1_bp_hpf = p.state(3, :);
         yn1_bp_hpf = p.state(4, :);
         xn1_lpf = p.state(5, :);
         yn1_lpf = p.state(6, :);
         xn1_hpf = p.state(7, :);
         yn1_hpf = p.state(8, :);

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

         g_cl_lf = p.gain_clip_lf;
         g_cl_mf = p.gain_clip_mf;
         g_cl_hf = p.gain_clip_hf;

         g_cl_lf_recipr = 1 / g_cl_lf;
         g_cl_mf_recipr = 1 / g_cl_mf;
         g_cl_hf_recipr = 1 / g_cl_hf;

         L = p.oversampling_factor;

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

         g_cl_up = p.gain_clip_up;
         g_cl_dn = p.gain_clip_dn;
         g_cl_up_recipr = 1 / g_cl_up;
         g_cl_dn_recipr = 1 / g_cl_dn;

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

               x_eq = accum .* L .* b0_iir;

               % Apply effect
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
               if p.bypass_clip
                  y_fx = x_eq + y_bp + y_lpf + y_hpf;
               else
                  y_fx = [0 0];
                  y_sum = x_eq + y_bp_cl + y_lpf_cl + y_hpf_cl;
                  y_fx(1) = p.soft_clip_asym(y_sum(1), g_cl_up, g_cl_up_recipr, g_cl_dn, g_cl_dn_recipr, 0.5);
                  y_fx(2) = p.soft_clip_asym(y_sum(2), g_cl_up, g_cl_up_recipr, g_cl_dn, g_cl_dn_recipr, 0.5);
               end

               % Apply antialiasing filter
               % Process first stage
               accum = (y_fx ... %.* b_iir(1, 1) ...
                  + 2 .* xn_iir2(1, :) ...
                  + xn_iir2(2, :) ...
                  - a_iir(1, 1) .* yn_iir2(1, :) ...
                  - a_iir(2, 1) .* yn_iir2(2, :));

               % Store feed forward state for first stage.
               xn_iir2(2, :) = xn_iir2(1, :);
               xn_iir2(1, :) = y_fx;

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

         % Store states
         p.state(1, :) = xn1_bp_lpf;
         p.state(2, :) = yn1_bp_lpf;
         p.state(3, :) = xn1_bp_hpf;
         p.state(4, :) = yn1_bp_hpf;
         p.state(5, :) = xn1_lpf;
         p.state(6, :) = yn1_lpf;
         p.state(7, :) = xn1_hpf;
         p.state(8, :) = yn1_hpf;

         % Store state for iir filter for next round
         p.state_y_iir1 = yn_iir1;
         p.state_x_iir1 = xn_iir1;
         p.state_y_iir2 = yn_iir2;
         p.state_x_iir2 = xn_iir2;
      end
      function y = process_fir(p, x)
         n = length(x);
         y = zeros(n, 2);

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
         i_store_in = p.state(9, 1);
         i_store_out = p.state(10, 1);

         n_dl_in = p.n_delay_line_interpolation;
         n_dl_out = p.n_delay_line_antialiasing;
         L = p.oversampling_factor;
         M = p.oversampling_fir_mult;
         b_fir = p.b_coeffs_fir;
         n_fir = p.n_coeffs_fir;

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

         g_cl_lf = p.gain_clip_lf;
         g_cl_mf = p.gain_clip_mf;
         g_cl_hf = p.gain_clip_hf;

         g_cl_lf_recipr = 1 / g_cl_lf;
         g_cl_mf_recipr = 1 / g_cl_mf;
         g_cl_hf_recipr = 1 / g_cl_hf;

         g_cl_up = p.gain_clip_up;
         g_cl_dn = p.gain_clip_dn;
         g_cl_up_recipr = 1 / g_cl_up;
         g_cl_dn_recipr = 1 / g_cl_dn;

         for i = 1:n
            x_local = x(i, :);

            % Store twice
            p.interpolation_delay_line(i_store_in, :) = x_local;
            p.interpolation_delay_line(i_store_in + n_dl_in, :) = x_local;

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

               x_eq = accum .* L;

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
               if p.bypass_clip
                  y_fx = x_eq + y_bp + y_lpf + y_hpf;
               else
                  y_sum = x_eq + y_bp_cl + y_lpf_cl + y_hpf_cl;
                  y_fx(1) = p.soft_clip_asym(y_sum(1), g_cl_up, g_cl_up_recipr, g_cl_dn, g_cl_dn_recipr, 0.5);
                  y_fx(2) = p.soft_clip_asym(y_sum(2), g_cl_up, g_cl_up_recipr, g_cl_dn, g_cl_dn_recipr, 0.5);
               end

               % Store to delay line for antialiasing filter.
               p.antialiasing_delay_line(i_store_out, :) = y_fx;
               p.antialiasing_delay_line(i_store_out + n_dl_out, :) = y_fx;

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

         % Store states
         p.state(1, :) = xn1_bp_lpf;
         p.state(2, :) = yn1_bp_lpf;
         p.state(3, :) = xn1_bp_hpf;
         p.state(4, :) = yn1_bp_hpf;
         p.state(5, :) = xn1_lpf;
         p.state(6, :) = yn1_lpf;
         p.state(7, :) = xn1_hpf;
         p.state(8, :) = yn1_hpf;
         p.state(9, 1) = i_store_in;
         p.state(10, 1) = i_store_out;
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

         sym = (p.symmetry_procent + 1) / 100;
         p.gain_clip_up = p.gain_clip;
         if p.symmetry_procent == 100
            p.gain_clip_dn = p.gain_clip;
         else
            p.gain_clip_dn = p.gain_clip * sym;
         end

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
         p.state = zeros(10, 2);
         p.state(9, :) = 1;
         p.state(10, :) = 1;
         p.b_coeffs_fir = zeros(192, 1);
         p.interpolation_delay_line = zeros(192 * 2, 2);
         p.antialiasing_delay_line = zeros(192 * 2, 2);
         fs_hi = p.fs * p.oversampling_factor;
         p.n_coeffs_fir = p.oversampling_factor * p.oversampling_fir_mult;
         p.b_coeffs_fir(1:p.n_coeffs_fir) = windowMethod(22e3, p.n_coeffs_fir, fs_hi);
         fs_hi = p.fs * p.oversampling_factor;
         b = zeros(3, 4);
         a = zeros(3, 4);

         for i = 1:p.n_bq_stages
            [b(:, i), a(:, i)] = makeLPF(fs_hi, 20e3, p.q_iir(i));
         end
         p.b_coeffs_iir = b(2:3, :);
         p.a_coeffs_iir = a(2:3, :);
         b0_prod = prod(b(1, :));
         p.b0_coeff_iir = b0_prod;
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
      function set.symmetry_procent(p, val)
         p.symmetry_procent = val;
         update(p);
      end
   end
end
