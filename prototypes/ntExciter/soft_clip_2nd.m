function y = soft_clip_2nd(x, g, g_recipr, a)
   x_dc = 0.5 .* g .* x + 0.5;
   if x_dc > 1
      y_dc = 1;
   elseif x_dc < 0
      y_dc = 0;
   else
      y_dc = (1 + a) .* x_dc - a * x_dc .^ 2;
   end
   y = 2 .* (y_dc - 0.5) .* g_recipr;
end
