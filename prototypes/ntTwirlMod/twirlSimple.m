classdef twirlSimple < audioPlugin
   properties
      fs = 0;
      tDel = 1.3;
      nDel = 0;
      fLpf1 = 350;
      damp = 0.5;
      freq = 0.1;
      bypass = false;
      phaseLevel = false;
      delayFilter = true;
      sumInput = true;
      aLpf1 = 0;
      stLpf1 = [0 0];
      tCount = 0;
      nSine = 0;
      storeLoc = 1;
      dLine = zeros(9600 * 2, 2);
   end
   properties (Constant)
      PluginInterface = audioPluginInterface(...
         audioPluginParameter('freq', ...
         'DisplayName', 'Frequency', ...
         'Mapping', {'log', 0.01, 1}, ...
         'Label', 'Hz', ...
         'Style', 'rotary', ...
         'Layout', [2, 1]), ...
         audioPluginParameter('bypass', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 1], ...
         'DisplayName', 'Master Bypass'), ...
         audioPluginGridLayout(...
         'RowHeight', [30, 150, 15, 90, 15], ...
         'ColumnWidth', [100], ...
         'RowSpacing', 30), ...
         'PluginName', 'Twirl', ...
         'VendorName', 'NT', ...
         'InputChannels', 2, ...
         'OutputChannels', 2, ...
         'BackgroundImage', 'logo.png' ...
         );
   end
   methods

      function y = process(p, x)
         n = length(x);
         y = zeros(length(x), 2);
         x(isnan(x)) = 0;
         stLpf1_loc = p.stLpf1;
         yPhase = [0 0];
         for i = 1:n
            if p.bypass == true
               y(i, :) = x(i, :);
            else
               xMono = sum(x(i, :)) / 2;
               w = 2 .* pi .* p.freq ./ p.fs .* p.tCount;
               sinW(1) = sin(w + pi / 2);
               sinW(2) = sin(w + pi);
               if p.phaseLevel == true
                  if p.sumInput == true
                     yPhase = sinW .* xMono;
                  else
                     yPhase = sinW .* x(i, :);
                  end
               end
               if p.delayFilter == true
                  xLpf = [0 0];
                  if p.phaseLevel == true
                     xDel = yPhase;
                  else
                     if p.sumInput == true
                        xDel = xMono;
                     else
                        xDel = x(i, :);
                     end
                  end
                  p.dLine(p.storeLoc, :) = xDel;
                  loc = p.storeLoc - floor(p.nDel * (sinW(2) + 1) / 2);
                  loc(loc < 1) = loc + 9600;
                  xLpf(1) = p.dLine(loc, 1);

                  loc = p.storeLoc - floor(p.nDel * (sinW(1) + 1) / 2);
                  loc(loc < 1) = loc + 9600;
                  xLpf(2) = p.dLine(loc, 2);

                  yLpf = p.aLpf1 * xLpf + (1 - p.aLpf1) * stLpf1_loc;
                  stLpf1_loc = yLpf;

                  yLs = p.damp * yLpf + (1 - p.damp) * xLpf;

                  y(i, 1) = yLs(1) * (sinW(1)) + xLpf(1) * (1 - (sinW(1)));
                  y(i, 2) = yLs(2) * (sinW(1)) + xLpf(2) * (1 - (sinW(1)));

                  % y(i, :) = yLs; %yLpf * p.damp + xLpf .* abs([sinW(1) sinW(2)]) * (1 - p.damp);
               else
                  if p.phaseLevel == true
                     y(i, :) = yPhase;
                  else
                     y(i, :) = x(i, :);
                  end
               end
               p.storeLoc = p.storeLoc + 1;
               p.storeLoc(p.storeLoc > 9600) = 1;
            end
            p.tCount = p.tCount + 1;
         end
         p.stLpf1 = stLpf1_loc;
      end
      function set.freq(p, val)
         p.freq = val;
         update(p);
      end
      function update(p)
         p.nDel = round(p.tDel / 1000 * p.fs);
      end
      function reset(p)
         p.fs = getSampleRate(p);
         update(p);
      end
   end
end
