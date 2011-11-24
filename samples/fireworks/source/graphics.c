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


#include "graphics.h"

#ifndef VIRTUAL_WIDTH
#define VIRTUAL_WIDTH  768
#endif

#ifndef VIRTUAL_HEIGHT
#define VIRTUAL_HEIGHT 512
#endif


// virtual screen

u32 *virtual_scr= NULL;

gcmContextData *context; // Context to keep track of the RSX buffer.

videoResolution res; // Screen Resolution

int currentBuffer = 0;
s32 *buffer[2]; // The buffer we will be drawing into.


void rescaler(u32 *scr, int wscr, int hscr, u32 *buf, int wbuf, int hbuf) {

	int n,m;

	int w, h, cws, chs, cwb, chb, ws, wb;

	// screen correction offset
	if(wscr> res.width) wscr =res.width; else  scr+=(res.width-wscr)/2;
	if(hscr> res.height) hscr =res.height; else  scr+= res.width*(res.height-hscr)/2;


	h= hscr; if(h<hbuf) h= hbuf;

	w= wscr; if(w<wbuf) w= wbuf;

	chs=chb=0;

	for(n=0; n<h; n++)
	{
	
	cws=cwb=ws=wb=0;

		for(m=0; m<w; m++) {

			scr[ws]= buf[wb]; 
		 
			cws+= wscr; if(cws>=w) {cws-=w; ws++;}
			cwb+= wbuf; if(cwb>=w) {cwb-=w; wb++;}
		}
    
	chs+= hscr; if(chs>=h) {chs-=h; scr+=res.width;}
	chb+= hbuf; if(chb>=h) {chb-=h; buf+=wbuf;}

	}

}


void waitFlip() { // Block the PPU thread untill the previous flip operation has finished.
	while(gcmGetFlipStatus() != 0) 
		usleep(200);
	gcmResetFlipStatus();
}

void flip(s32 buffer) {
	assert(gcmSetFlip(context, buffer) == 0);
	rsxFlushBuffer(context);
	gcmSetWaitFlip(context); // Prevent the RSX from continuing until the flip has finished.
}

u8 aspect;

// Initilize everything. You can probally skip over this function.
void init_screen() {
	// Allocate a 1Mb buffer, alligned to a 1Mb boundary to be our shared IO memory with the RSX.
	void *host_addr = memalign(1024*1024, 1024*1024);
	assert(host_addr != NULL);

	// Initilise Reality, which sets up the command buffer and shared IO memory
	context = rsxInit(0x10000, 1024*1024, host_addr); 
	assert(context != NULL);

	videoState state;
	assert(videoGetState(0, 0, &state) == 0); // Get the state of the display
	assert(state.state == 0); // Make sure display is enabled

	// Get the current resolution
	assert(videoGetResolution(state.displayMode.resolution, &res) == 0);
	
	// Configure the buffer format to xRGB
	videoConfiguration vconfig;
	memset(&vconfig, 0, sizeof(videoConfiguration));
	vconfig.resolution = state.displayMode.resolution;
	vconfig.format = VIDEO_BUFFER_FORMAT_XRGB;
	vconfig.pitch = res.width * 4;
	aspect=vconfig.aspect=state.displayMode.aspect;

	assert(videoConfigure(0, &vconfig, NULL, 0) == 0);
	assert(videoGetState(0, 0, &state) == 0); 

	s32 buffer_size = 4 * res.width * res.height; // each pixel is 4 bytes
	printf("buffers will be 0x%x bytes\n", buffer_size);
	
	gcmSetFlipMode(GCM_FLIP_VSYNC); // Wait for VSYNC to flip

	// Allocate two buffers for the RSX to draw to the screen (double buffering)
	buffer[0] = rsxMemalign(16, buffer_size);
	buffer[1] = rsxMemalign(16, buffer_size);
	assert(buffer[0] != NULL && buffer[1] != NULL);

	u32 offset[2];
	assert(rsxAddressToOffset(buffer[0], &offset[0]) == 0);
	assert(rsxAddressToOffset(buffer[1], &offset[1]) == 0);
	// Setup the display buffers
	assert(gcmSetDisplayBuffer(0, offset[0], res.width * 4, res.width, res.height) == 0);
	assert(gcmSetDisplayBuffer(1, offset[1], res.width * 4, res.width, res.height) == 0);

	gcmResetFlipStatus();
	flip(currentBuffer);
}



void scr_init(void) {
	init_screen();

	// alloc memory for virtual screen

	virtual_scr = malloc (VIRTUAL_WIDTH*VIRTUAL_HEIGHT*4);

}

void scr_flip() {

		waitFlip();

		// re-scale and draw virtual screen to real screen

		// 480P / 576P
		if(res.width<1280) {

			rescaler((u32 *) buffer[currentBuffer], res.width-80 , res.height-32, virtual_scr, VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
		
		} else if(res.width==1280) {
        // 720P
			rescaler((u32 *) buffer[currentBuffer], res.width-120 , res.height-32, virtual_scr, VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
		
		} else {
		// 1080i
			rescaler((u32 *) buffer[currentBuffer], res.width-160 , res.height-112, virtual_scr, VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
		
		}
		
		flip(currentBuffer);
		currentBuffer^=1;
}

inline int get_virtual_w(void) {
	return VIRTUAL_WIDTH;
}

inline int get_virtual_h(void) {
	return VIRTUAL_HEIGHT;
}

// console



int PX=0, PY=0;
u32 color=0xffffffff;

void PutChar(int x, int y, u8 c, unsigned e_color) {

	int n, m, k;

	u32 r=255, g=255, b=255, a;
	u32 r2, g2, b2;

	b= e_color & 0xff;
	g= (e_color>>8) & 0xff;
	r= (e_color>>16) & 0xff;

	if(x < 0 || y < 0 || x > (VIRTUAL_WIDTH-16) || y > (VIRTUAL_HEIGHT-32)) return;

	if(c < 32) return;

	c-=32;
	
	u32 * scr= (u32 *) virtual_scr;
	
	scr+= x + y * VIRTUAL_WIDTH;

	k= c * 16 * 32;

	// chars 16 x 32  2 bits intensity (ANSI charset from 32 to 255)

	for(n=0; n<32; n++) {
		for(m=0; m<16; m++) {

			a=((font[k>>2]>>((k & 3)<<1)) & 3)*85;

			if(a!=0) {
				r2= (r * a)>>8;
				g2= (g * a)>>8;
				b2= (b * a)>>8;
				
				e_color= (r2<<16) | (g2<<8) | (b2) | 0xff000000;

				scr[m]= e_color;

			}
		k++;
		}

	scr+= VIRTUAL_WIDTH;
	}	
}


static void (*scr_cls_func) (void)= NULL;

void scr_register_cls( void (*cls_func) (void)) {

	scr_cls_func= cls_func;

}

void cls() {
	if(!scr_cls_func)
		memset((void *) virtual_scr, 0, VIRTUAL_WIDTH*VIRTUAL_HEIGHT*4);
	else
		scr_cls_func();

	PX= 0; PY= 0;
	
}



void s_printf( char *format, ...) {
	va_list	opt;
	static  u8 buff[2048];

	memset(buff, 0, 2048);
	va_start(opt, format);
	vsprintf( (void *) buff, format, opt);
	va_end(opt);

	u8 *buf = (u8 *) buff;
	while(*buf) {

		if(*buf == '\n') {PX= 0; PY+= 32;}
		else if(*buf == 9) PX+=16;
		else if(*buf >= 32) {
			PutChar(PX, PY, *buf, color);
			PX+=16;
		}

	buf++;

	if(PX < 0) PX= 0;
	if(PX > (VIRTUAL_WIDTH-16)) {PX= 0; PY+= 32;}
	if(PY < 0) PY= 0;
	if(PY > (VIRTUAL_HEIGHT-32)) {cls();}
	
	}


return;
}
