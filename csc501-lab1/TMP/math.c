#include <math.h>

double log(double x) {
	
	if(x <= 0) {
		kprintf("\nError! 'x' in log(x) must be positive.\n");
		return -1;
	}

	/*** 
	x = (1+tmp)/(1-tmp)
	tmp = (x-1)/(x+1)
	lnx = ln((1+tmp)/(1-tmp))
		= ln(1+tmp) - ln(1-tmp) Taylor series
		= 2(tmp + tmp^3/3+ ... + tmp^(2n+1)/(2n+1))
	***/
	
	double tmp = (x-1)/(x+1);
	double res = 0;
	int n = 20;		// set n to control precison
	int i, exp;
	for(i = 0; i < 20; i++) {
		exp = 2*i+1;
		res += pow(tmp, exp)/exp;
	}
	res *= 2;

	return res;
}

/** x rasied to the y-th power.     **/
/** if y is not a int, it will be the 
integer part of y when passes the 
prarameters.						**/
double pow(double x, int y) {

	// special cases
	if(is_double_equal(x, 0)&& (y == 0)) {
		kprintf("\nBase and exponent can't be 0 at the same time!\n");
		return -1;		// error code=-1
	}

	int is_exp_positve = 1;
	if(y < 0) {
		is_exp_positve = 0;
		y = -y;
	}

	double res = 1;
	double last_pow = x;
	while(y > 0) {
		if(y & 1 == 1) {
			res = res * last_pow;
		}
		y = y >> 1;
		last_pow = last_pow * last_pow;
	}

	// if y < 0, result = 1/result
	if(is_exp_positve) {
		return res;	
	} 
	else {
		return 1/res;
	}
}

double expdev(double lambda) {
	double random;
    do {
        random = (double) rand() / RAND_MAX;
    }
    while (random == 0.0);
    return -log(random)/lambda;
}

int is_double_equal(double x, double y) {

	static const double PRECISION = 1e-12;		/* set comparison precison */
	return  (x-y <= PRECISION)&&(x-y >= -PRECISION);
}