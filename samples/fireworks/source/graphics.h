/* 
    Copyright (C) 1985, 2010  Francisco Muñoz "Hermes" <www.elotrolado.net>

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


#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h> 
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdarg.h> 
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <rsx/gcm_sys.h>
#include <rsx/rsx.h>
#include <sysutil/video.h>


#include "font.h"


extern u32 *virtual_scr;

void rescaler(u32 *scr, int wscr, int hscr, u32 *buf, int wbuf, int hbuf);

void cls();

void scr_init(void);
void scr_flip();

void scr_register_cls( void (*cls_func) (void));

int get_virtual_w(void);
int get_virtual_h(void);

extern int PX;
extern int PY;
extern u32 color;

void s_printf( char *format, ...);

#endif

