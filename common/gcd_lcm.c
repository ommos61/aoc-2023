
long gcd(long a, long b) {
    if (b == 0) {
        return a;
    } else {
    	return gcd(b, a % b);
    }
}

long lcm(long *in, unsigned cnt) {
    long ans = in[0];
    for (unsigned i = 0; i < cnt; i++) {
	ans = (ans * in[i]) / gcd(ans, in[i]);
    }
    return ans;
}
