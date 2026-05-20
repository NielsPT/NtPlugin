t = primesInRange(4, 80, 8)

function next = nextPrime(last)
   n = last;
   done = false;
   while done == false
      n = n + 1;
      if isprime(n) == true
         next = n;
         return;
      end
   end
end
function prim = primesInRange(min, max, count)
   p_all = primes(max);
   p_min2max = 0;
   for i = 1:length(p_all)
      if p_all(i) > min
         p_min2max = [p_min2max p_all(i)];
      end
   end
   p_min2max = p_min2max(2:end);
   dist = round(length(p_min2max) / count);
   prim = p_min2max(1:dist:end);
   while length(prim) < count
      prim = [prim nextPrime(prim(end))];
      disp("added");
   end
   while length(prim) > count
      prim = prim(1:end - 1);
      disp("removed");
   end
   disp("count = " + count + ", result = " + length(prim) + ", dist = " + dist);
end
