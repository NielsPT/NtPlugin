classdef ntExciter2_01 < audioPlugin
  properties
    bypass = false;
    kill_dry = false;
    clip_bypass = false;
    bypas_2nd_filter = true;
    f_bpf_gui = 5000;
    q_bpf_gui = 0.707;
    g_bpf_gui = 0;
    g_out_gui = 0;
    g_clip_gui = 0;
  end
  properties (Access = private)
    fs = 48000;
    f_bpf = 0;
    f_2nd = 10000;
    q_bpf = 0.707;
    q_2nd = 0.707;
    g_bpf = 0;
    b_bpf = zeros(1, 2);
    a_bpf = zeros(1, 2);
    b_2nd = zeros(1, 2);
    a_2nd = zeros(1, 2);
    b_step_bpf = zeros(1, 2);
    a_step_bpf = zeros(1, 2);
    b_step_2nd = zeros(1, 2);
    a_step_2nd = zeros(1, 2);

    s_bpf = zeros(4, 2);
    s_2nd = zeros(4, 2);

    g_clip = 1;
    g_clip_recipr = 1;
    g_out = 1;
    g_dry = 1;

    t_glide = 0.1;
    n_glide = 4410;
    n_step = 0;

    a_n_7th = zeros(1, 3);
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
      'DisplayName', 'Drive', ...
      'Mapping', {'lin', -40, 40}, ...
      'Label', 'dB', ...
      'Style', 'rotary', ...
      'Layout', [2, 3]), ...
      audioPluginParameter('g_clip_gui', ...
      'DisplayName', 'Level', ...
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
      audioPluginParameter('bypas_2nd_filter', ...
      'Mapping', {'enum', 'In', 'Out'}, ...
      'Style', 'vtoggle', ...
      'Layout', [4, 3], ...
      'DisplayName', '2nd Filter'), ...
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
      'ColumnWidth', [100, 100, 100, 100, 100], ...
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
    function y = soft_clip_7th(p, x)
      x_ = x / p.a_n_7th(1);
      n = 0:3;
      if x_ > 1
        y = 1;
      elseif x_ < -1
        y = -1;
      else
        y = sum(p.a_n_7th .* x_ .^ (2 .* n + 1));
      end
    end
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

        %load coefficients for band pass
        a1_2nd = p.a_2nd(1, 1);
        a2_2nd = p.a_2nd(1, 2);
        b0_2nd = p.b_2nd(1, 1);
        b2_2nd = p.b_2nd(1, 2);

        %set up states for band pass
        yn1_2nd = p.s_2nd(1, :);
        yn2_2nd = p.s_2nd(2, :);
        xn1_2nd = p.s_2nd(3, :);
        xn2_2nd = p.s_2nd(4, :);

        for i = 1:n
          % load next input sample
          x_bpf = x(i, :);

          if any(isnan(p.s_bpf))
            disp("bpf nan")
          end
          if any(isnan(p.s_2nd))
            disp("2nd nan")
          end

          % apply band pass filter
          yn0_bpf = (b0_bpf .* x_bpf ...
            + b2_bpf .* xn2_bpf ...
            - a1_bpf .* yn1_bpf ...
            - a2_bpf .* yn2_bpf);

          % update states
          xn2_bpf = xn1_bpf;
          xn1_bpf = x_bpf;
          yn2_bpf = yn1_bpf;
          yn1_bpf = yn0_bpf;

          % store output sample
          y_bpf = yn0_bpf;

          % adjust gain
          x_clip = y_bpf .* p.g_bpf;

          if ~p.clip_bypass
            % distort output from filter
            y_clip = [0 0];
            y_clip(1) = soft_clip_7th(p, x_clip(1));
            y_clip(2) = soft_clip_7th(p, x_clip(2));
          else
            y_clip = x_clip;
          end

          x_2nd = y_clip .* p.g_clip / p.q_bpf;

          if ~p.bypas_2nd_filter
            % apply second band pass filter
            yn0_2nd = (b0_2nd .* x_2nd ...
              + b2_2nd .* xn2_2nd ...
              - a1_2nd .* yn1_2nd ...
              - a2_2nd .* yn2_2nd);

            % update states
            xn2_2nd = xn1_2nd;
            xn1_2nd = x_2nd;
            yn2_2nd = yn1_2nd;
            yn1_2nd = yn0_2nd;

            % store output sample
            x_sum = yn0_2nd / p.q_2nd;
          else
            x_sum = x_2nd;
          end

          % sum result
          y(i, :) = (x(i, :) * p.g_dry + x_sum) .* p.g_out;

          % Glide coeffs
          if p.n_step >= 0
            p.n_step = p.n_step - 1;
            b0_bpf = b0_bpf + p.b_step_bpf(1);
            b2_bpf = b2_bpf + p.b_step_bpf(2);
            a1_bpf = a1_bpf + p.a_step_bpf(1);
            a2_bpf = a2_bpf + p.a_step_bpf(2);

            b0_2nd = b0_2nd + p.b_step_2nd(1);
            b2_2nd = b2_2nd + p.b_step_2nd(2);
            a1_2nd = a1_2nd + p.a_step_2nd(1);
            a2_2nd = a2_2nd + p.a_step_2nd(2);
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

        %store state
        p.s_2nd(1, :) = yn1_2nd;
        p.s_2nd(2, :) = yn2_2nd;
        p.s_2nd(3, :) = xn1_2nd;
        p.s_2nd(4, :) = xn2_2nd;

        % store the (maybe) modified coeffs
        p.b_2nd = [b0_2nd b2_2nd];
        p.a_2nd = [a1_2nd a2_2nd];
      end
    end
    function reset(p)
      p.fs = getSampleRate(p);
      p.n_glide = round(p.fs * p.t_glide);
      c = calculateCoeffs(p, p.f_bpf, p.q_bpf);
      p.b_bpf = c(1, :);
      p.a_bpf = c(2, :);
      c = calculateCoeffs(p, p.f_2nd, p.q_2nd);
      p.b_2nd = c(1, :);
      p.a_2nd = c(2, :);
      update(p);
    end
    function update(p)
      p.f_bpf = p.f_bpf_gui;
      p.f_2nd = p.f_bpf_gui * 2;
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
      c_next = calculateCoeffs(p, p.f_bpf, p.q_bpf);
      c_curr = [p.b_bpf; p.a_bpf];
      c_diff = c_next - c_curr;
      c_steps = c_diff ./ p.n_glide;
      p.b_step_bpf = c_steps(1, :);
      p.a_step_bpf = c_steps(2, :);

      c_next = calculateCoeffs(p, p.f_2nd, p.q_2nd);
      c_curr = [p.b_2nd; p.a_2nd];
      c_diff = c_next - c_curr;
      c_steps = c_diff ./ p.n_glide;
      p.b_step_2nd = c_steps(1, :);
      p.a_step_2nd = c_steps(2, :);

      p.n_step = p.n_glide;

      p.a_n_7th = p.sym_coeffs(3);
      if any(isnan(p.a_bpf))
        disp("A bpf nan")
      end
      if any(isnan(p.a_2nd))
        disp("A 2nd nan")
      end
      if any(isnan(p.b_bpf))
        disp("B bpf nan")
      end
      if any(isnan(p.b_2nd))
        disp("B 2nd nan")
      end
    end
    function a_n = sym_coeffs(p, N)
      % order = 2 * N + 1
      n = 0:N;
      a_n = (-1) .^ n .* factorial(2 .* N + 1) ./ ...
        (4 .^ N .* factorial(N) .* (2 .* n + 1) .* factorial(n) .* factorial(N - n));
    end
    function c = calculateCoeffs(p, f, q)
      w0 = 2 * pi * f / p.fs;
      alpha = sin(w0) / (2 * q);

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
    function set.bypas_2nd_filter(p, val)
      p.bypas_2nd_filter = val;
      update(p);
    end
  end
end
