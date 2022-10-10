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

void compute_w_coefficients(double n, double a, double b, COMPLEX* W)
{
     double twoPiOverN = 0;
     double s = 0;
	 double c = 0;
	 int n_int = n;

	twoPiOverN = 2.0 * 3.1415926535897932384626434 / n;
	for (int k = a; k <= b; k++) {
		n_int = n;
		c = cos(twoPiOverN * k);
		(W[k]).re = c;
		(W[n_int - k]).re = c;
		s = sin(twoPiOverN * k);
		(W[k]).im = -s;
		(W[n_int - k]).im = s;
	}
}

double factor(double n_d)
{
	double ret_val = n_d;
	double found = 0;
	double smaller_n = n_d - 1;
	for (int r=smaller_n; ((r>1) && (found==0)); r--) {
		int n = n_d;
		if ((n % r) == 0) {
			ret_val = n/r;
			found = 1;
		}
	}

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

		for (int tmp_1 = 0; (j < (r-r4)); tmp_1++) {
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
			j += 4;
		}

		for (int tmp_2=0; j < r; tmp_2++) {
			offset = offset_d;
			ip_ind = ip_ind_d;
			j_ind = j_ind_d;

			out[offset + j_ind] = in[offset + ip_ind];
			ip_ind_d = ip_ind + 1;
			j_ind_d += m_d;
			j += 1;
		}
	}
}
void fft_twiddle_gen1(COMPLEX * in, COMPLEX * out,
				  COMPLEX * W, double r, double m,
				  double nW, double nWdnti, double nWdntm, double offset)
{
	double r0 = 0;
	double i0 = 0;
	double rt = 0;
	double it = 0;
	double rw = 0;
	double iw = 0;
	double l1 = 0;
	double l0 = 0;
	int l0_int = l0;
	int computed_offset = 0;
     for (int q = 0; q < r; q++) {
		r0 = 0;
		i0 = 0;
		rt = 0;
		it = 0;
		rw = 0;
		iw = 0;
		l1 = nWdnti + nWdntm * q;
		l0 = 0;
		l0_int = l0;
		computed_offset = 0;

		for (int j = 0; j < r; j++) {
			l0_int = l0;
			rw = (W[l0_int]).re;
			iw = (W[l0_int]).im;
			computed_offset = offset + m*j;
			rt = (in[computed_offset]).re;
			it = (in[computed_offset]).im;
			r0 += rt * rw - it * iw;
			i0 += rt * iw + it * rw;
			l0 += l1;
			if (l0 > nW) {
				l0 -= nW;
			}
		}
		computed_offset = offset + m*q;
		(out[computed_offset]).re = r0;
		(out[computed_offset]).im = i0;
     }
}

void fft_twiddle_gen(double i, double i1, COMPLEX * in, COMPLEX * out, COMPLEX * W,
                         double nW, double nWdn, double r, double m, double offset)
{
	double w = i;
	for (int tmp_36 = 0; w < i1; tmp_36++) {
		fft_twiddle_gen1(in, out, W, r, m, nW, nWdn * w, nWdn * m, w+offset);
		w += 1;
	}
}

void fft_while(double n_orig, COMPLEX * in, COMPLEX * out, int *factors, COMPLEX * W, double nW) {
	double n = n_orig;
	double factor_count = 0;
	int factor_count_int = factor_count;
	double r = factors[factor_count_int];
	double m = n / r;
	int n_int = n_orig;
	double* offsets = malloc(n_int * sizeof(int));
	double* offsets_count = malloc(n_int * sizeof(int));
	double last_offset = 0;
	int last_offset_int = last_offset;
	double invert = 0;
	offsets[0] = 0;
	offsets_count[0] = 1;

	double tmp = 0;

	while (r < n) {
		for (int k = 0; k <= last_offset; k++) {
			if (invert) {
				unshuffle(0, m, out, in, r, m, offsets[k]);
			} else {
				unshuffle(0, m, in, out, r, m, offsets[k]);
			}
		}

		double max_offset = last_offset;
		for (int i = 0; i <= max_offset; i++) {
			double cur_offset = offsets[i];
			double x = m;
			for (int tmp_3 = 0; x < n; tmp_3++) {
				last_offset += 1;
				last_offset_int = last_offset;
				offsets[last_offset_int] = cur_offset+x;
				x += m;
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

	while (factor_count >= 0) {
		factor_count_int = factor_count; 
		last_offset = offsets_count[factor_count_int] - 1;
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
		if (factor_count_int < 0) {
			factor_count_int = 0;
		}
		r = factors[factor_count_int];
		n = m*r;
		invert = !invert;
	}

	tmp = offsets[0] + offsets_count[0];

	free(offsets);
	free(offsets_count);
}

void fft(double n, COMPLEX * in, COMPLEX * out)
{
     int* factors = malloc(40 * sizeof(int));		/* allows FFTs up to at least 3^40 */
     double l = n;
     double r = 0;
	 int n_copy_int = n;

     printf("Computing coefficients ");
     COMPLEX* W = malloc((n_copy_int + 1) * sizeof(COMPLEX));
     compute_w_coefficients(n, 0, n / 2, W);
     printf(" completed!\n");

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
	 fft_while(n_copy_int, in, out, factors, W, n);
     printf(" completed!\n");

     free(W);
     return;
}

int main(int argc, char *argv[]) {
	double N = 100;
	COMPLEX* in = malloc(sizeof(COMPLEX) * N);
	for (int h = 0; h < N; ++h) {
		(in[h]).re = sqrt(h*9283) - h*h + h + 98 - h*12;
		(in[h]).im = sqrt(h*231) - h*h + h + 22 - h*34;
	}
	
	COMPLEX* out = malloc(sizeof(COMPLEX) * N);
	fft(N, in, out);
	for (int w = 0; w < N; ++w) {
		printf("%f %f\n", (out[w]).re, (out[w]).im);
	}

	double tmp_val = in[0].re + in[0].im + out[0].re + out[0].im;

	free(in);
	free(out);
}