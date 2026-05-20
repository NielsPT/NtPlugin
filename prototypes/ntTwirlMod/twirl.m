classdef twirl < audioPlugin
   properties
      fs = 0;
      tDel = 1.3;
      nDel = 0;
      fLpf1 = 350;
      damp = 0.5;
      freq = 0.1;
      bypass = false;
      phaseLevel = false;
      delay = true;
      sumInput = true;
      aLpf1 = 0;
      stLpf1 = [0 0];
      tCount = 0;
      nSine = 0;
      storeLoc = 1;
      dLine = zeros(9600 * 2, 2);
      rect = true;
      filter = false;
   end
   properties (Constant)
      PluginInterface = audioPluginInterface(...
         audioPluginParameter('freq', ...
         'DisplayName', 'Frequency', ...
         'Mapping', {'log', 0.01, 1}, ...
         'Label', 'Hz', ...
         'Style', 'rotary', ...
         'Layout', [2, 1]), ...
         audioPluginParameter('tDel', ...
         'DisplayName', 'Delay', ...
         'Mapping', {'log', 0.1, 10}, ...
         'Label', 'ms', ...
         'Style', 'rotary', ...
         'Layout', [2, 2]), ...
         audioPluginParameter('fLpf1', ...
         'DisplayName', 'Damping Freq', ...
         'Mapping', {'log', 20, 20000}, ...
         'Label', 'Hz', ...
         'Style', 'rotary', ...
         'Layout', [2, 3]), ...
         audioPluginParameter('damp', ...
         'DisplayName', 'Damping Amount', ...
         'Mapping', {'lin', 0, 1}, ...
         'Label', 'x', ...
         'Style', 'rotary', ...
         'Layout', [2, 4]), ...
         audioPluginParameter('phaseLevel', ...
         'Mapping', {'enum', 'Out', 'In'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 1], ...
         'DisplayName', 'Level'), ...
         audioPluginParameter('delay', ...
         'Mapping', {'enum', 'Out', 'In'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 2], ...
         'DisplayName', 'Delay'), ...
         audioPluginParameter('filter', ...
         'Mapping', {'enum', 'Out', 'In'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 3], ...
         'DisplayName', 'Filter'), ...
         audioPluginParameter('sumInput', ...
         'Mapping', {'enum', 'Out', 'In'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 4], ...
         'DisplayName', 'Sum Input'), ...
         audioPluginParameter('bypass', ...
         'Mapping', {'enum', 'In', 'Out'}, ...
         'Style', 'vtoggle', ...
         'Layout', [4, 5], ...
         'DisplayName', 'Master Bypass'), ...
         audioPluginGridLayout(...
         'RowHeight', [30, 150, 15, 90, 15], ...
         'ColumnWidth', [100, 100, 100, 100, 100], ...
         'RowSpacing', 30), ...
         'PluginName', 'Twirl Advanced', ...
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
         yDel = [0 0];
         yFilter = [0 0];
         for i = 1:n
            if p.bypass == true
               y(i, :) = x(i, :);
            else
               xMono = sum(x(i, :)) / 2;
               w1 = pi .* p.freq ./ p.fs .* p.tCount + pi / 4;
               w2 = 2 .* pi .* p.freq ./ p.fs .* p.tCount;
               w5 = 2 .* pi .* p.freq ./ p.fs .* p.tCount + pi / 2;
               sinW1 = (sin(w1));
               sinW2 = (sin(w1 - pi / 2));
               sinW3 = (sin(w2));
               sinW4 = (sin(w2 - pi));
               sinW5 = (sin(w5));
               sinW6 = (sin(w5) - pi);
               if p.phaseLevel == true
                  if p.sumInput == true
                     % yPhase(1) = abs(sinW1) * xMono;
                     % yPhase(2) = abs(sinW2) * xMono;
                     yPhase(1) = ((sinW3 + 1) / 2) * xMono;
                     yPhase(2) = ((sinW4 + 1) / 2) * xMono;
                  else
                     yPhase(1) = ((sinW3 + 1) / 2) * x(i, 1);
                     yPhase(2) = ((sinW4 + 1) / 2) * x(i, 2);
                  end
               end
               if p.delay == true
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
                  if p.rect == true
                     loc = p.storeLoc - floor(p.nDel * abs(sinW2));
                  else
                     loc = p.storeLoc - floor(p.nDel * (sinW2 + 1) / 2);
                  end
                  loc(loc < 1) = loc + 9600;
                  yDel(1) = p.dLine(loc, 1);

                  if p.rect == true
                     loc = p.storeLoc - floor(p.nDel * abs(sinW1));
                  else
                     loc = p.storeLoc - floor(p.nDel * (sinW1 + 1) / 2);
                  end
                  loc(loc < 1) = loc + 9600;
                  yDel(2) = p.dLine(loc, 2);
               end
               if p.filter == true
                  if p.delay == true
                     xLpf = yDel;
                  else
                     if p.phaseLevel == true
                        xLpf = yPhase;
                     else
                        if p.sumInput == true
                           xLpf = [xMono xMono];
                        else
                           xLpf = x(i, :);
                        end
                     end
                  end
                  yLpf = p.aLpf1 * xLpf + (1 - p.aLpf1) * stLpf1_loc;
                  stLpf1_loc = yLpf;

                  yLs = p.damp * yLpf + (1 - p.damp) * xLpf;

                  % yTmp(1) = yLs(1) * abs(sinW2) + xLpf(1) * abs(sinW1);
                  % yTmp(2) = yLs(2) * abs(sinW1) + xLpf(2) * abs(sinW2);

                  %sinW5(sinW5 > 0) = 0;

                  wDiff = (sinW5 + 1) / 2;
                  yFilter = (yLs * wDiff + xLpf * (1 - wDiff));

                  % y(i, :) = yLs; %yLpf * p.damp + xLpf .* abs([sinW1 sinW2]) * (1 - p.damp);
               end
               if (p.phaseLevel == true) && (p.delay == false) && (p.filter == false)
                  y(i, :) = yPhase;
               elseif (p.delay == true) && (p.filter == false)
                  y(i, :) = yDel;
               elseif (p.filter == true)
                  y(i, :) = yFilter;
               else
                  y(i, :) = x(i, :);
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
      function set.tDel(p, val)
         p.tDel = val;
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
      function set.sumInput(p, val)
         p.sumInput = val;
         update(p);
      end
      function update(p)
         p.nDel = round(p.tDel / 1000 * p.fs);
         p.aLpf1 = 1 - exp(-2 .* pi .* p.fLpf1 ./ p.fs);
      end
      function reset(p)
         p.fs = getSampleRate(p);
         update(p);
      end
   end
end
