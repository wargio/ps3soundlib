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

const float PI  = 3.14159265358f;
const float PIH = 1.57079632679; // PI / 2
const float B   = 12.56637061432; // 4.0f * PI
const float C   = -0.4052847345718778; // -4.0f / (PI * PI)
const float P   = 0.225f;

float fast_sine(float x) {
    float y = B * x + C * x * (x < 0 ? -x : x);
    return P * (y * (y < 0 ? -y : y) - y) + y;
}

// x range: [-PI, PI]
float fast_cosine(float x) {
    x = (x > 0) ? -x : x;
    x += PIH;
    return fast_sine(x);
}
