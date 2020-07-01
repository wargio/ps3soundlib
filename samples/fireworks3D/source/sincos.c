/* 
	Copyright (C) 1985, 2010  Francisco Mu?z "Hermes" <www.elotrolado.net>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "sincos.h"

#include <math.h>

static int sine_table[16384];
static int cosine_table[16384];

#define FLOAT_FIX (16384)
#define PID 6.283185307179586476925286766559

void init_fast_sine_cosine() {
	volatile int n;

	for (n = 0; n < 16384; n++) {
		sine_table[n] = (int)((double)FLOAT_FIX * sin((PID * (float)n) / 16384.0));
	}

	for (n = 0; n < 16384; n++) {
		cosine_table[n] = (int)((double)FLOAT_FIX * cos((PID * (float)n) / 16384.0));
	}
}

// fast sin (ang=16384= 360 degrees)
int sin_int(int ang) {
	int n = ang;

	if (n < 0)
		n = 16384 - n;
	n &= 16383;

	return (sine_table[n]);
}

int cosin_int(int ang) {
	int n = ang;

	if (n < 0)
		n = 16384 - n;
	n &= 16383;

	return (cosine_table[n]);
}
