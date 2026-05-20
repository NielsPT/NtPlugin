classdef ntRev_06 < audioPlugin
  properties
    fs = 48000;
    tPre = 25;
    nPre = 0;
    t60 = 1.4;
    mix_gui = 30;
    gEarly_dB = -10;
    gDiff_dB = 0;
    fLpf1 = 6000;
    damp = 0.13;
    tLateDel = 50;
    oEarly = 8;
    oLate = 8;
    mix = 1;
    serial = false;
    bypass = false;
    method = 7;
    t_early_min = 5;
    t_early_max = 100;
    t_late_min = 5;
    t_late_max = 100;

    modFreq = 4;
    modPhase = zeros(8, 2);
    modDepth_gui = 2;
    modDepth = 0.01;

    gDriveIn_gui = 4;
    gDriveOut_gui = 0;
    gDriveIn = 1;
    gDriveOut = 1;

    gLfWidth_db = 4;
    fLfWidth = 1000;
    gLfWidth_lin = 0;
    aLfWidth = 0;
    stateLfWidth = 0;
    diffusion = 0;

    dLineEarly = zeros(ntRev_06.dLineLen * 2, 2);
    dLineLate = zeros(ntRev_06.dLineLen, 2, 8);
    dLineLateDel = zeros(ntRev_06.dLineLen, 2);
    nEarly = zeros(8, 8, 2);
    nLate = zeros(8, 2);
    nLateDel = 1;
    gEarlyDiffuse = zeros(8, 8);
    stLocLate = 1;
    stLocEarly = 1;
    gLate = zeros(8, 2);
    Q = hadamard(8);
    inGain = ones(8, 1) * 0.5;
    outGain = ones(8, 1);
    stLpf1 = zeros(8, 2);
    aLpf1 = 0;
    aEarly = 0;
    aLate = 0;
    infTime = 0;
  end
  properties (Constant)
    dLineLen = 76800;
    PluginInterface = audioPluginInterface( ...
      audioPluginParameter('t60', ...
      'DisplayName', 'Reverb Time', ...
      'Mapping', {'lin', 0.1, 4}, ...
      'Label', 's', ...
      'Style', 'rotary', ...
      'Layout', [2, 1]), ...
      audioPluginParameter('tPre', ...
      'DisplayName', 'Early Delay', ...
      'Mapping', {'lin', 0, 200}, ...
      'Label', 'ms', ...
      'Style', 'rotary', ...
      'Layout', [2, 3]), ...
      audioPluginParameter('tLateDel', ...
      'DisplayName', 'Late Delay', ...
      'Mapping', {'lin', 0, 200}, ...
      'Label', 'ms', ...
      'Style', 'rotary', ...
      'Layout', [2, 4]), ...
      audioPluginParameter('fLpf1', ...
      'DisplayName', 'Damping Freq', ...
      'Mapping', {'log', 20, 20000}, ...
      'Label', 'Hz', ...
      'Style', 'rotary', ...
      'Layout', [2, 5]), ...
      audioPluginParameter('damp', ...
      'DisplayName', 'Damping Amount', ...
      'Mapping', {'lin', 0, 1}, ...
      'Label', 'x', ...
      'Style', 'rotary', ...
      'Layout', [2, 6]), ...
      audioPluginParameter('diffusion', ...
      'DisplayName', 'Diffusion', ...
      'Mapping', {'lin', 0, 1}, ...
      'Label', 'x', ...
      'Style', 'rotary', ...
      'Layout', [2, 7]), ...
      audioPluginParameter('modFreq', ...
      'DisplayName', 'Mod freq', ...
      'Mapping', {'log', 0.1, 10}, ...
      'Label', 'Hz', ...
      'Style', 'rotary', ...
      'Layout', [4, 1]), ...
      audioPluginParameter('modDepth_gui', ...
      'DisplayName', 'Mod Depth', ...
      'Mapping', {'log', 0.01, 100}, ...
      'Label', '%', ...
      'Style', 'rotary', ...
      'Layout', [4, 2]), ...
      audioPluginParameter('gDriveIn_gui', ...
      'DisplayName', 'Input Drive', ...
      'Mapping', {'lin', -20, 20}, ...
      'Label', 'dB', ...
      'Style', 'rotary', ...
      'Layout', [4, 3]), ...
      audioPluginParameter('gEarly_dB', ...
      'DisplayName', 'Early Reflections', ...
      'Mapping', {'lin', -100, 0}, ...
      'Label', 'dB', ...
      'Style', 'rotary', ...
      'Layout', [4, 4]), ...
      audioPluginParameter('gDiff_dB', ...
      'DisplayName', 'Reverb Tail', ...
      'Mapping', {'lin', -100, 0}, ...
      'Label', 'dB', ...
      'Style', 'rotary', ...
      'Layout', [4, 5]), ...
      audioPluginParameter('mix_gui', ...
      'DisplayName', 'Mix', ...
      'Mapping', {'lin', 0, 100}, ...
      'Label', '%', ...
      'Style', 'rotary', ...
      'Layout', [4, 6]), ...
      audioPluginParameter('gLfWidth_db', ...
      'DisplayName', 'LF Width', ...
      'Mapping', {'lin', 0, 12}, ...
      'Label', 'dB', ...
      'Style', 'rotary', ...
      'Layout', [4, 7]), ...
      audioPluginParameter('t_early_min', ...
      'DisplayName', 'Early first', ...
      'Mapping', {'int', 1, 20}, ...
      'Label', 'ms', ...
      'Style', 'rotary', ...
      'Layout', [6, 1]), ...
      audioPluginParameter('t_early_max', ...
      'DisplayName', 'Early last', ...
      'Mapping', {'int', 20, 500}, ...
      'Label', 'ms', ...
      'Style', 'rotary', ...
      'Layout', [6, 2]), ...
      audioPluginParameter('t_late_min', ...
      'DisplayName', 'Late First', ...
      'Mapping', {'int', 1, 20}, ...
      'Label', 'ms', ...
      'Style', 'rotary', ...
      'Layout', [6, 3]), ...
      audioPluginParameter('t_late_max', ...
      'DisplayName', 'Late Last', ...
      'Mapping', {'int', 20, 500}, ...
      'Label', 'ms', ...
      'Style', 'rotary', ...
      'Layout', [6, 4]), ...
      audioPluginParameter('bypass', ...
      'Mapping', {'enum', 'In', 'Out'}, ...
      'Style', 'vtoggle', ...
      'Layout', [6, 5], ...
      'DisplayName', 'Master Bypass'), ...
      audioPluginParameter('fLfWidth', ...
      'DisplayName', 'LF Freq', ...
      'Mapping', {'log', 200, 20000}, ...
      'Label', 'Hz', ...
      'Style', 'rotary', ...
      'Layout', [6, 7]), ...
      audioPluginGridLayout( ...
      'RowHeight', [30, 150, 15, 150, 15, 100, 15], ...
      'ColumnWidth', [100, 100, 100, 100, 100, 100, 100], ...
      'RowSpacing', 30), ...
      'PluginName', 'ntReverb', ...
      'VendorName', 'NT', ...
      'VendorVersion', '0.6.0', ...
      'InputChannels', 2, ...
      'OutputChannels', 2, ...
      'BackgroundImage', 'logo.png' ...
    );
  end
  methods (Static)
    function y = saw(x)
      n = length(x);
      y = zeros(n, 1);
      for i = 1:n
        x_ = mod(x(i), 2 * pi);
        if x_ < 0
          x_ = x_ + 2 * pi;
        end

        alpha = 2 / pi;

        if x_ < pi / 2
          y_ = x_ * alpha;
        elseif x_ < 3 * pi / 2
          y_ = -x_ * alpha + 2;
        else
          y_ = x_ * alpha - 4;
        end
        y(i) = y_;
      end
    end
  end
  methods
    function set.mix_gui(p, val)
      p.mix_gui = val;
      update(p);
    end
    function set.t60(p, val)
      p.t60 = val;
      update(p);
    end
    function set.gEarly_dB(p, val)
      p.gEarly_dB = val;
      update(p);
    end
    function set.gDiff_dB(p, val)
      p.gDiff_dB = val;
      update(p);
    end
    function set.tLateDel(p, val)
      p.tLateDel = val;
      update(p);
    end
    function set.modDepth_gui(p, val)
      p.modDepth_gui = val;
      update(p);
    end
    function set.modFreq(p, val)
      p.modFreq = val;
      update(p);
    end
    function set.fLpf1(p, val)
      p.fLpf1 = val;
      update(p);
    end
    function set.damp(p, val)
      p.damp = val;
      update(p);
    end
    function set.tPre(p, val)
      p.tPre = val;
      update(p);
    end
    function set.gDriveIn_gui(p, val)
      p.gDriveIn_gui = val;
      update(p);
    end
    function set.method(p, val)
      p.method = val;
      initDelay(p);
    end
    function set.t_early_min(p, val)
      p.t_early_min = val;
      initDelay(p);
    end
    function set.t_early_max(p, val)
      p.t_early_max = val;
      initDelay(p);
    end
    function set.t_late_min(p, val)
      p.t_late_min = val;
      initDelay(p);
    end
    function set.t_late_max(p, val)
      p.t_late_max = val;
      initDelay(p);
    end
    function set.diffusion(p, val)
      p.diffusion = val;
      update(p);
    end
    function set.gLfWidth_db(p, val)
      p.gLfWidth_db = val;
      update(p);
    end
    function set.fLfWidth(p, val)
      p.fLfWidth = val;
      update(p);
    end
    function y = process(p, x)
      n = length(x);
      A = p.Q;
      y = zeros(length(x), 2);
      x(isnan(x)) = 0;
      stLpf1_loc = p.stLpf1;
      for i = 1:n
        if p.bypass == true
          y(i, :) = x(i, :);
        else
          yClipperIn = [0 0];
          xClipperIn = x(i, :) .* p.gDriveIn;
          if xClipperIn(1) > 1
            yClipperIn(1) = 1;
          elseif xClipperIn(1) < -1
            yClipperIn(1) = -1;
          else
            yClipperIn(1) = (1.5 .* xClipperIn(1) - 0.5 .* xClipperIn(1) .^ 3) ./ 1.5;
          end

          if xClipperIn(2) > 1
            yClipperIn(2) = 1;
          elseif xClipperIn(2) < -1
            yClipperIn(2) = -1;
          else
            yClipperIn(2) = (1.5 .* xClipperIn(2) - 0.5 .* xClipperIn(2) .^ 3) ./ 1.5;
          end
          yClipperIn = yClipperIn ./ p.gDriveIn;

          p.dLineEarly(p.stLocEarly, :) = yClipperIn;
          yEarly = [0 0];
          for j = 1:8
            loc1 = p.stLocEarly - p.nEarly(j, 1, 1) - p.nPre;
            loc1(loc1 < 1) = loc1 + ntRev_05.dLineLen;
            yEarly(1) = yEarly(1) + p.dLineEarly(loc1, 1) .* p.gEarlyDiffuse(j, 1);

            loc2 = p.stLocEarly - p.nEarly(j, 1, 2) - p.nPre;
            loc2(loc2 < 1) = loc2 + ntRev_05.dLineLen;
            yEarly(2) = yEarly(2) + p.dLineEarly(loc2, 2) .* p.gEarlyDiffuse(j, 1);

            % TODO: This sounds like crap.
            if p.diffusion > 0
              for k = 2:8
                loc1 = p.stLocEarly - p.nEarly(j, k, 1) - p.nPre;
                loc1(loc1 < 1) = loc1 + ntRev_05.dLineLen;
                yEarly(1) = yEarly(1) + p.dLineEarly(loc1, 1) .* p.gEarlyDiffuse(j, k);

                loc2 = p.stLocEarly - p.nEarly(j, k, 2) - p.nPre;
                loc2(loc2 < 1) = loc2 + ntRev_05.dLineLen;
                yEarly(2) = yEarly(2) + p.dLineEarly(loc2, 2) .* p.gEarlyDiffuse(j, k);
              end
            end
          end
          if p.serial == true
            xLate = yEarly;
          else
            xLate = yClipperIn;
          end
          yLate = [0 0];
          p.infTime = p.infTime + 1;
          % TODO: down sample nMod
          nMod = round(p.saw(2 .* pi .* p.modFreq ./ p.fs .* p.infTime + ...
            p.modPhase .* pi / 180) .* p.nLate .* p.modDepth);
          for j = 1:8
            accum = [0 0];
            for k = 1:8
              loc = p.stLocLate - p.nLate(k, 1) - nMod(k, 1);
              loc(loc < 1) = loc + ntRev_05.dLineLen;
              accum(1) = accum(1) + p.dLineLate(loc, 1, k) .* A(j, k);
              loc = p.stLocLate - p.nLate(k, 2) - nMod(k, 2);
              loc(loc < 1) = loc + ntRev_05.dLineLen;
              accum(2) = accum(2) + p.dLineLate(loc, 2, k) .* A(j, k);
            end
            accum = accum + xLate .* p.inGain(j);
            accumLpf = p.aLpf1 * accum + (1 - p.aLpf1) * stLpf1_loc(j, :);
            stLpf1_loc(j, :) = accumLpf;
            accumLs = accumLpf * p.damp + accum * (1 - p.damp);
            p.dLineLate(p.stLocLate, :, j) = accumLs .* p.gLate(j, :);
            loc = p.stLocLate - p.nLate(j, 1) - nMod(j, 1);
            loc(loc < 1) = loc + ntRev_05.dLineLen;
            yLate(1) = yLate(1) + p.dLineLate(loc, 1, j) .* p.outGain(j);
            loc = p.stLocLate - p.nLate(j, 2) - nMod(j, 2);
            loc(loc < 1) = loc + ntRev_05.dLineLen;
            yLate(2) = yLate(2) + p.dLineLate(loc, 2, j) .* p.outGain(j);
          end
          p.dLineLateDel(p.stLocLate, :) = yLate;
          loc = p.stLocLate - p.nLateDel;
          loc(loc < 1) = loc + ntRev_05.dLineLen;
          yLateDel = p.dLineLateDel(loc, :);
          yClipperOut = (1.5 .* yLateDel - 0.5 .* yLateDel .^ 3);
          if p.serial == true
            xLfWidth = yClipperOut .* p.aLate;
          else
            xLfWidth = yClipperOut .* p.aLate + yEarly .* p.aEarly;
          end
          if length(xLfWidth) ~= 2
            yOut = xLfWidth;
          else
            xMid = xLfWidth(1) + xLfWidth(2);
            xSide = xLfWidth(1) - xLfWidth(2);

            yLpf = p.aLfWidth * xSide + (1 - p.aLfWidth) * p.stateLfWidth;
            p.stateLfWidth = yLpf;
            ySide = yLpf * (1 - p.gLfWidth_lin) + xSide;

            yMid = xMid;
            yOut = [0 0];
            yOut(1) = 0.5 * (yMid + ySide);
            yOut(2) = 0.5 * (yMid - ySide);
          end
          y(i, :) = (1 - p.mix) .* x(i, :) + p.mix .* yOut;
        end
        p.stLocLate = p.stLocLate + 1;
        p.stLocEarly = p.stLocEarly + 1;
        p.stLocLate(p.stLocLate > ntRev_05.dLineLen) = 1;
        p.stLocEarly(p.stLocEarly > ntRev_05.dLineLen * 2) = 1;
      end
      p.stLpf1 = stLpf1_loc;
    end
    function update(p)
      p.gLate = 10 .^ (-3 .* (p.nLate ./ (p.t60 .* p.fs))) ./ sqrt(8);
      p.aLpf1 = 1 - exp(-2 .* pi .* p.fLpf1 ./ p.fs);
      p.aEarly = 10 ^ (p.gEarly_dB ./ 20);
      p.aLate = 10 ^ (p.gDiff_dB ./ 20);
      p.modDepth = p.modDepth_gui / 100;
      p.modPhase(:, 1) = linspace(0, 360, 8)';
      p.modPhase(:, 2) = linspace(360, 0, 8)';
      p.nLateDel = ceil(p.tLateDel * p.fs / 1000);
      p.mix = p.mix_gui / 100;
      p.nPre = ceil(p.tPre * p.fs / 1000);
      p.gDriveIn = 10 ^ (p.gDriveIn_gui ./ 20);
      p.gDriveOut = 10 ^ (p.gDriveOut_gui ./ 20);
      p.aLfWidth = 1 - exp(-2 * pi * p.fLfWidth / p.fs);
      p.gLfWidth_lin = 10 ^ (p.gLfWidth_db / 20);
      gEarly = [1 1.02 0.818 0.635 0.719 0.267 0.242 0.2];
      d = [0.1 0.6 0.3 1 0.2 0.8 0.6 0.4] ./ 4;
      p.gEarlyDiffuse = [gEarly; repmat(d, 7, 1) .* p.diffusion];
      initDelay(p);
    end
    function reset(p)
      p.fs = getSampleRate(p);
      p.dLineLate = zeros(ntRev_05.dLineLen, 2, 8);
      update(p);
    end
    function initDelay(p)
      t = primesInRange(nextPrime(p.t_late_min), nextPrime(p.t_late_max), 8);
      p.nLate(:, 1) = round(t .* p.fs ./ 1000);

      t = primesInRange(nextPrime(nextPrime(p.t_late_min)), ...
        nextPrime(nextPrime(p.t_late_max)), 8);
      p.nLate(:, 2) = round(t .* p.fs ./ 1000);

      t = zeros(8, 8);
      primScale = 5;
      t(:, 1) = primesInRange(nextPrime(p.t_early_min), nextPrime(p.t_early_max), 8);
      for i = 1:8
        tc = t(i, 1);
        % TODO: This sounds like crap.
        t(i, 2:8) = abs(primesInRange(nextPrime(tc - primScale), nextPrime(tc + primScale), 7) ./ primScale);
      end
      p.nEarly(:, :, 1) = round(t .* p.fs ./ 1000);

      t = zeros(8, 8);
      t(:, 2) = primesInRange(nextPrime(p.t_early_min), nextPrime(nextPrime(p.t_early_max)), 8);
      for i = 1:8
        tc = t(i, 2);
        t(i, 2:8) = abs(primesInRange(nextPrime(tc - primScale), nextPrime(tc + primScale), 7) ./ primScale);
      end
      p.nEarly(:, :, 2) = round(t .* p.fs ./ 1000);
    end
  end
end
