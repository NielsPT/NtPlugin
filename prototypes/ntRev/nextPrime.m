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
