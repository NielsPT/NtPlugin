function [y, next_state] = gateSc_lin (p, x, state)
   next_state = [0 0 0 0];
   state_rel = state(1);
   state_att = state(2);
   state_peak = state(3);
   state_hold = state(4);

   x_abs = abs(x);
   x_peak = max(x_abs, p.alpha_peak * state_peak + (1 - p.alpha_peak) * x_abs);
   next_state(3) = x_peak;

   if x_peak > p.thresh_lin
      target = 1;
      state_hold = p.n_hold;
   else
      target = p.range_lin;
   end

   % apply release filter
   if state_rel > target
      if state_hold > 0
         state_hold = state_hold - 1;
         temp = state_rel;
      else
         temp = p.alpha_rel * state_rel + (1 - p.alpha_rel) * target;
      end
   else
      temp = target;
   end
   next_state(1) = temp;
   next_state(4) = state_hold;

   % apply attack filter
   temp2 = p.alpha_att * state_att + (1 - p.alpha_att) * temp;
   next_state(2) = temp2;
   y = temp2;
end
