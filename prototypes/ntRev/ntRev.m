classdef ntRev < audioPlugin
   properties
      fs = 48000;
      tPre = 25;
      nPre = 0;
      t60 = 2;
      mix_gui = 100;
      gEarly_dB = -10;
      gDiff_dB = 0;
      fLpf1 = 6000;
      damp = 0.13;
      tLateDel = 50;
      oEarly = 8;
      oLate = 8;
      mix = 1;
      bypass = false;

      modFreq = 4;
      modPhase = zeros(8, 2);
      modDepth_gui = 2;
      modDepth = 0.01;

      gDriveIn_gui = 8;
      gDriveOut_gui = 0;
      gDriveIn = 1;
      gDriveOut = 1;

      dLineEarly = zeros(19200 * 2, 2);
      dLineLate = zeros(19200, 2, 8);
      dLineLateDel = zeros(19200, 2);
      nEarly = zeros(8, 2);
      nLate = zeros(8, 2);
      nLateDel = 1;
      gEarly = [1 1.02 0.818 0.635 0.719 0.267 0.242 0.2;
            1 1.02 0.818 0.635 0.719 0.267 0.242 0.2]';
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
      PluginInterface = audioPluginInterface(...
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
         'Layout', [2, 2]), ...
         audioPluginParameter('tLateDel', ...
         'DisplayName', 'Late Delay', ...
         'Mapping', {'lin', 0, 200}, ...
         'Label', 'ms', ...
         'Style', 'rotary', ...
         'Layout', [2, 3]), ...
         audioPluginParameter('fLpf1', ...
         'DisplayName', 'Damping Freq', ...
         'Mapping', {'log', 20, 20000}, ...
         'Label', 'Hz', ...
         'Style', 'rotary', ...
         'Layout', [2, 4]), ...
         audioPluginParameter('damp', ...
         'DisplayName', 'Damping Amount', ...
         'Mapping', {'lin', 0, 1}, ...
         'Label', 'x', ...
         'Style', 'rotary', ...
         'Layout', [2, 5]), ...
         audioPluginParameter('gDriveIn_gui', ...
         'DisplayName', 'Input Drive', ...
         'Mapping', {'lin', -20, 20}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [4, 1]), ...
         audioPluginParameter('gEarly_dB', ...
         'DisplayName', 'Early Reflections', ...
         'Mapping', {'lin', -100, 0}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [4, 3]), ...
         audioPluginParameter('gDiff_dB', ...
         'DisplayName', 'Reverb Tail', ...
         'Mapping', {'lin', -100, 0}, ...
         'Label', 'dB', ...
         'Style', 'rotary', ...
         'Layout', [4, 4]), ...
         audioPluginParameter('mix_gui', ...
         'DisplayName', 'Mix', ...
         'Mapping', {'lin', 0, 100}, ...
         'Label', '%', ...
         'Style', 'rotary', ...
         'Layout', [4, 5]), ...
         audioPluginParameter('bypass', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [6, 5], ...
         'DisplayName', 'Master Bypass'), ...
         audioPluginParameter('modFreq', ...
         'DisplayName', 'Mod freq', ...
         'Mapping', {'log', 0.1, 10}, ...
         'Label', 'Hz', ...
         'Style', 'rotary', ...
         'Layout', [6, 1]), ...
         audioPluginParameter('modDepth_gui', ...
         'DisplayName', 'Mod Depth', ...
         'Mapping', {'log', 0.01, 100}, ...
         'Label', '%', ...
         'Style', 'rotary', ...
         'Layout', [6, 2]), ...
         audioPluginGridLayout(...
         'RowHeight', [30, 150, 15, 150, 15, 150, 15], ...
         'ColumnWidth', [100, 100, 100, 100, 100], ...
         'RowSpacing', 30), ...
         'PluginName', 'ntReverb', ...
         'VendorName', 'NT', ...
         'InputChannels', 2, ...
         'OutputChannels', 2 ...
         );
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
                  yClipperIn(1) = (1.5 .* xClipperIn(1) - 0.5 .* xClipperIn(1).^3) ./ 1.5;
               end

               if xClipperIn(2) > 1
                  yClipperIn(2) = 1;
               elseif xClipperIn(2) < -1
                  yClipperIn(2) = -1;
               else
                  yClipperIn(2) = (1.5 .* xClipperIn(2) - 0.5 .* xClipperIn(2).^3) ./ 1.5;
               end
               yClipperIn = yClipperIn ./ p.gDriveIn;

               p.dLineEarly(p.stLocEarly, :) = yClipperIn;
               yEarly = [0 0];
               for j = 1:8
                  loc = p.stLocEarly - p.nEarly(j, 1) - p.nPre;
                  loc(loc < 1) = loc + 19200 * 2;
                  yEarly(2) = yEarly(2) + p.dLineEarly(loc, 2) .* p.gEarly(j, 2);
                  loc = p.stLocEarly - p.nEarly(j, 2) - p.nPre;
                  loc(loc < 1) = loc + 19200 * 2;
                  yEarly(1) = yEarly(1) + p.dLineEarly(loc, 1) .* p.gEarly(j, 1);
               end
               yLate = [0 0];
               p.infTime = p.infTime + 1;
               nMod = round(sin(2 .* pi .* p.modFreq ./ p.fs .* p.infTime + ...
                  p.modPhase .* pi / 180) .* p.nLate .* p.modDepth);
               for j = 1:8
                  accum = [0 0];
                  for k = 1:8
                     loc = p.stLocLate - p.nLate(k, 1) - nMod(k);
                     loc(loc < 1) = loc + 19200;
                     accum(1) = accum(1) + p.dLineLate(loc, 1, k) .* A(j, k);
                     loc = p.stLocLate - p.nLate(k, 2) - nMod(k);
                     loc(loc < 1) = loc + 19200;
                     accum(2) = accum(2) + p.dLineLate(loc, 2, k) .* A(j, k);
                  end
                  accum = accum + yClipperIn .* p.inGain(j);
                  accumLpf = p.aLpf1 * accum + (1 - p.aLpf1) * stLpf1_loc(j);
                  stLpf1_loc(j, :) = accumLpf;
                  accumLs = accumLpf * p.damp + accum * (1 - p.damp);
                  p.dLineLate(p.stLocLate, :, j) = accumLs .* p.gLate(j, :);
                  loc = p.stLocLate - p.nLate(j, 1) - nMod(j);
                  loc(loc < 1) = loc + 19200;
                  yLate(1) = yLate(1) + p.dLineLate(loc, 1, j) .* p.outGain(j);
                  loc = p.stLocLate - p.nLate(j, 2) - nMod(j);
                  loc(loc < 1) = loc + 19200;
                  yLate(2) = yLate(2) + p.dLineLate(loc, 2, j) .* p.outGain(j);
               end
               p.dLineLateDel(p.stLocLate, :) = yLate;
               loc = p.stLocLate - p.nLateDel;
               loc(loc < 1) = loc + 19200;
               yLateDel = p.dLineLateDel(loc, :);
               yClipperOut = (1.5 .* yLateDel - 0.5 .* yLateDel.^3);
               y(i, :) = (1 - p.mix) .* x(i, :) + p.mix .* (yClipperOut * p.aLate + yEarly * p.aEarly);
            end
            p.stLocLate = p.stLocLate + 1;
            p.stLocEarly = p.stLocEarly + 1;
            p.stLocLate(p.stLocLate > 19200) = 1;
            p.stLocEarly(p.stLocEarly > 19200 * 2) = 1;
         end
         p.stLpf1 = stLpf1_loc;
      end
      function update(p)
         p.gLate = 10.^(-3 .* (p.nLate ./ (p.t60 .* p.fs))) ./ sqrt(8);
         p.aLpf1 = 1 - exp(-2 .* pi .* p.fLpf1 ./ p.fs);
         p.aEarly = 10^(p.gEarly_dB ./ 20);
         p.aLate = 10^(p.gDiff_dB ./ 20);
         p.modDepth = p.modDepth_gui / 100;
         p.modPhase(:, 1) = linspace(0, 360, 8)';
         p.modPhase(:, 2) = linspace(360, 0, 8)';
         p.nLateDel = ceil(p.tLateDel * p.fs / 1000);
         p.mix = p.mix_gui / 100;
         p.nPre = ceil(p.tPre * p.fs / 1000);
         p.gDriveIn = 10^(p.gDriveIn_gui ./ 20);
         p.gDriveOut = 10^(p.gDriveOut_gui ./ 20);
      end
      function reset(p)
         p.fs = getSampleRate(p);
         initDelay(p);
         p.dLineLate = zeros(19200, 2, 8);
         update(p);
      end
      function initDelay(p)
         t = primes(p.fs / 4) * 0.1;
         tLate = t(1:16:end);
         p.nLate(:, 1) = round(tLate(2:9) * p.fs / 1000);
         tLate = t(2:16:end);
         p.nLate(:, 2) = round(tLate(2:9) * p.fs / 1000);
         tEarly = t(3:16:end);
         p.nEarly(:, 1) = round(tEarly(4:11) * p.fs / 1000);
         p.nEarly(:, 2) = round(tEarly(3:10) * p.fs / 1000);
         %disp(['tEarly: ', num2str(tEarly)]);
         % disp(['nLate L ', num2str(p.nLate(:, 1)')]);
         % disp(['nLate R ', num2str(p.nLate(:, 2)')]);
      end
   end
end
