/**********************************************************************************************/
/*  This program is part of the Barcelona OpenMP Tasks Suite                                  */
/*  Copyright (C) 2009 Barcelona Supercomputing Center - Centro Nacional de Supercomputacion  */
/*  Copyright (C) 2009 Universitat Politecnica de Catalunya                                   */
/*                                                                                            */
/*  This program is free software; you can redistribute it and/or modify                      */
/*  it under the terms of the GNU General Public License as published by                      */
/*  the Free Software Foundation; either version 2 of the License, or                         */
/*  (at your option) any later version.                                                       */
/*                                                                                            */
/*  This program is distributed in the hope that it will be useful,                           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of                            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                             */
/*  GNU General Public License for more details.                                              */
/*                                                                                            */
/*  You should have received a copy of the GNU General Public License                         */
/*  along with this program; if not, write to the Free Software                               */
/*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA            */
/**********************************************************************************************/

/* 
 * Original code from the Cilk project 
 *
 * Copyright (c) 2000 Massachusetts Institute of Technology
 * Copyright (c) 2000 Matteo Frigo
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

struct COMPLEX_struct{
     double re;
	 double im;
};

typedef struct COMPLEX_struct COMPLEX;

/* Definitions and operations for complex numbers */

/*
 * compute the W coefficients (that is, powers of the root of 1)
 * and store them into an array.
 */
void compute_w_coefficients(int n, int a, int b, COMPLEX* W)
{
     double twoPiOverN = 0;
     double s = 0;
	 double c = 0;

	  twoPiOverN = 2.0 * 3.1415926535897932384626434 / n;
	  for (int k = a; k <= b; ++k) {
	       c = cos(twoPiOverN * k);
	       (W[k]).re = (W[n - k]).re = c;
	       s = sin(twoPiOverN * k);
	       (W[k]).im = -s;
	       (W[n - k]).im = s;
	  }
}
/*
 * Determine (in a stupid way) if n is divisible by eight, then by four, else
 * find the smallest prime factor of n.
 */
double factor(double n_d)
{
	 int n = n_d;
     if (n < 2) return 1;
     if ((((((n == 64) || (n == 128)) || (n == 256)) || (n == 1024)) || (n == 2048)) || (n == 4096)) return 8;
     if ((n % 16) == 0) return 16;
     if ((n % 8) == 0) return 8;
     if ((n % 4) == 0) return 4;
     if ((n % 2) == 0) return 2;

     /* try odd numbers up to n (computing the sqrt may be slower) */
	 double ret_val = n;
	 double found = 0;
     for (int r = 3; (r < n_d) && (found == 0); r += 2) {
		n = n_d;
		if ((n % r) == 0) {
			ret_val = r;
			found = 1;
		}
	 }

     /* n is prime */
     return ret_val;
}

void unshuffle(double a, double b, COMPLEX * in, COMPLEX * out, double r, double m_d, double offset_d)
{
	int r_int = r;
	double r4 = r_int%4;

	int offset = offset_d;
	int m = m_d;

	double ip_ind_d = a * r;
	int ip_ind = ip_ind_d;
	for (int i = a; i < b; ++i) {
		double j_ind_d = i;
		int j_ind = j_ind_d;
		double j = 0;

		for (j = 0; (j < (r-r4)); j += 4) {
			offset = offset_d;
			m = m_d;
			j_ind = j_ind_d;
			ip_ind = ip_ind_d;

			out[offset + j_ind + 0] = in[offset + ip_ind + 0];
			out[offset + j_ind + m] = in[offset + ip_ind + 1];
			out[offset + j_ind + 2 * m] = in[offset + ip_ind + 2];
			out[offset + j_ind + 3 * m] = in[offset + ip_ind + 3];
			j_ind_d += 4 * m;
			ip_ind_d += 4;
		}

		for (int tmp=0; j < r; ++j) {
			offset = offset_d;
			ip_ind = ip_ind_d;
			j_ind = j_ind_d;

			out[offset + j_ind] = in[offset + ip_ind];
			ip_ind_d = ip_ind + 1;
			j_ind_d += m_d;
		}
	}
}
void fft_twiddle_gen1(COMPLEX * in, COMPLEX * out,
				  COMPLEX * W, double r, double m,
				  double nW, double nWdnti, double nWdntm, double offset)
{
     for (int k = 0; k < r; ++k) {
		double r0 = 0;
		double i0 = 0;
		double rt = 0;
		double it = 0;
		double rw = 0;
		double iw = 0;
		double l1 = nWdnti + nWdntm * k;
		double l0 = 0;
		int l0_int = l0;
		int computed_offset = 0;

		for (int j = 0; j < r; ++j) {
			l0_int = l0;
			rw = (W[l0_int]).re;
			iw = (W[l0_int]).im;
			computed_offset = offset + m*j;
			rt = (in[computed_offset]).re;
			it = (in[computed_offset]).im;
			r0 += rt * rw - it * iw;
			i0 += rt * iw + it * rw;
			l0 += l1;
			if (l0 > nW)
				l0 -= nW;
		}
		computed_offset = offset + m*k;
		(out[computed_offset]).re = r0;
		(out[computed_offset]).im = i0;
     }
}

void fft_twiddle_gen(double i, double i1, COMPLEX * in, COMPLEX * out, COMPLEX * W,
                         double nW, double nWdn, double r, double m, double offset)
{
	for (int w = i; w < i1; w++) {
		fft_twiddle_gen1(in, out, W,
					r, m, nW, nWdn * w, nWdn * m, w+offset);
	}
}

/*
 * Recursive complex FFT on the n complex components of the array in:
 * basic Cooley-Tukey algorithm, with some improvements for
 * n power of two. The result is placed in the array out. n is arbitrary. 
 * The algorithm runs in time O(n*(r1 + ... + rk)) where r1, ..., rk
 * are prime numbers, and r1 * r2 * ... * rk = n.
 *
 * n: size of the input
 * in: pointer to input
 * out: pointer to output
 * factors: list of factors of n, precomputed
 * W: twiddle factors
 * nW: size of W, that is, size of the original transform
 *
 */
void fft_while(double n_orig, COMPLEX * in, COMPLEX * out, int *factors, COMPLEX * W, double nW) {
	double n = n_orig;
	double factor_count = 0;
	int factor_count_int = factor_count;
	double r = factors[factor_count_int];
	double m = n / r;
	double* offsets = malloc(n * sizeof(int));
	double* offsets_copy = malloc(n * sizeof(int));
	double* offsets_count = malloc(n * sizeof(int));
	double last_offset = 0;
	int last_offset_int = last_offset;
	double invert = 0;
	offsets[0] = 0;
	offsets_count[0] = 1;

	for (int wo=0; r < n; wo++) {
		for (int k = 0; k <= last_offset; k++) {
			if (invert) {
				unshuffle(0, m, out, in, r, m, offsets[k]);
			} else {
				unshuffle(0, m, in, out, r, m, offsets[k]);
			}
		}

		double max_offset = last_offset;
		for (int i = 0; i <= last_offset; i++) {
			offsets_copy[i] = offsets[i];
		}

		last_offset = -1;
		for (int i = 0; i <= max_offset; i++) {
			double cur_offset = offsets_copy[i];
			for (int k = 0; k < n; 1) {
				last_offset += 1;
				last_offset_int = last_offset;
				offsets[last_offset_int] = cur_offset+k;
				k += m;
			}
		}

		n = m;
		factor_count += 1;
		factor_count_int = factor_count;
		r = factors[factor_count_int];
		m = n / r;
		offsets_count[factor_count_int] = last_offset + 1;
		invert = !invert;
	}

	for (int we=0; factor_count >= 0; we++) {
		for (int x = 0; x <= last_offset; x++) {
			if (invert) {
				fft_twiddle_gen(0, m, out, in, W, nW, nW / n, r, m, offsets[x]);
			} else {
				fft_twiddle_gen(0, m, in, out, W, nW, nW / n, r, m, offsets[x]);
			}
		}

		m = n;
		factor_count -= 1;
		factor_count_int = factor_count;
		r = factors[factor_count_int];
		n = m*r;
		invert = !invert;

		double max_offset = last_offset;
		for (int i = 0; i <= last_offset; i++) {
			offsets_copy[i] = offsets[i];
		}

		max_offset = offsets_count[factor_count_int];
		last_offset = 0;
		double offset_counter = -1;
		int offset_counter_int = offset_counter;
		for (int i = 0; i < max_offset; i++) {
			offset_counter += 1;
			last_offset_int = last_offset;
			offset_counter_int = offset_counter;
			offsets[offset_counter_int] = offsets_copy[last_offset_int];
			for (int k = 0; k < n; 1) {
				last_offset += 1;
				k += m;
			}
		}

		last_offset = offset_counter;
	}

	free(offsets);
	free(offsets_copy);
	free(offsets_count);
}

/*
 * user interface for fft_aux
 */
void fft(double n, COMPLEX * in, COMPLEX * out)
{
     int* factors = malloc(40 * sizeof(int));		/* allows FFTs up to at least 3^40 */
     double l = n;
     double r = 0;

     printf("Computing coefficients ");
     COMPLEX* W = malloc((n + 1) * sizeof(COMPLEX));
     compute_w_coefficients(n, 0, n / 2, W);
     printf(" completed!\n");

     /* 
      * find factors of n, first 8, then 4 and then primes in ascending
      * order 
      */
	 double p_loc = 0;
	 int p_loc_int = p_loc;
     do {
	  r = factor(l);
		p_loc_int = p_loc;
	  factors[p_loc_int] = r;
	  p_loc += 1;
	  l /= r;
     } while (l > 1);

     printf("Computing FFT");
	 fft_while(n, in, out, factors, W, n);
     printf(" completed!\n");

     free(W);
     return;
}

int main(int argc, char *argv[]) {
	double N = 100;
	COMPLEX* in = malloc(sizeof(COMPLEX) * N);
	for (int i = 0; i < N; ++i) {
		(in[i]).re = sqrt(i*9283) - i*i + i + 98 - i*12;
		(in[i]).im = sqrt(i*231) - i*i + i + 22 - i*34;
	}
	
	COMPLEX* out = malloc(sizeof(COMPLEX) * N);
	fft(N, in, out);
	for (int i = 0; i < N; ++i) {
		printf("%f %f\n", (out[i]).re, (out[i]).im);
	}

	free(in);
	free(out);
}