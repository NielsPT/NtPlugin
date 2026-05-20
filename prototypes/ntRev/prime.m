prim = primes(5000);
disp(['max prime: ', num2str(prim(end))]);

% prim = prim(round(end / 4):end);
n = length(prim);
prim = prim(randi(n, 8, 1));
prim(prim < 500) = 500 + randi(20, 1, 1);

t = primes(1000) * 0.1;
disp(['max t: ', num2str(t(end))]);
t = t(1:16:end);
t = t(3:10);
n = round(t * 48);
