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


#include <ppu-lv2.h>
#include <sys/spu.h>
#include <lv2/spu.h>
#include <lv2/sysfs.h>
#include <lv2/process.h>
#include <sys/systime.h>
#include <sys/thread.h>
#include <sysmodule/sysmodule.h>

#include <sysutil/sysutil.h>

#include <io/pad.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graphics.h"

#include <math.h>
#include <time.h>

#include "sincos.h"

//#define FROM_FILE

#ifndef FROM_FILE
#include "spu_soundmodule.bin.h" // load SPU Module
#else
void * spu_soundmodule_bin = NULL;
#endif

#include <soundlib/spu_soundlib.h>
#include <soundlib/audioplayer.h>

#include "m2003_mp3.bin.h"
#include "effect_mp3.bin.h"
#include "sound_ogg.bin.h"

// SPU
u32 spu = 0;
sysSpuImage spu_image;

#define SPU_SIZE(x) (((x)+127) & ~127)

padInfo padinfo;
padData paddata;


u32 inited;

#define INITED_CALLBACK     1
#define INITED_SPU          2
#define INITED_SOUNDLIB       4
#define INITED_AUDIOPLAYER  8

void release_all() {
    
	if(inited & INITED_CALLBACK)
        sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);

	if(inited & INITED_AUDIOPLAYER)
        StopAudio();
	
    if(inited & INITED_SOUNDLIB)
        SND_End();

    if(inited & INITED_SPU) {
        s_printf("Destroying SPU... ");
        scr_flip();
        sleep(1);
	    sysSpuRawDestroy(spu);
	    sysSpuImageClose(&spu_image);
    }

    inited=0;
	
}

static void sys_callback(uint64_t status, uint64_t param, void* userdata) {

     switch (status) {
		case SYSUTIL_EXIT_GAME:
				
			release_all();
			sysProcessExit(1);
			break;
      
       default:
		   break;
         
	}
}


int     explosion_size;
short   *explosion= NULL;
int     explosion_freq;
int     explosion_is_stereo;

// allocate voices for fireworks (from 8 to 15)

void snd_explo(int voice, int pos)
{
		
	if(SND_StatusVoice(8 + voice) == SND_UNUSED) {
	
        int l,r;

        // sound balance
		l=((pos- get_virtual_w() / 2) <  100) ? 128 : 80;
		r=((pos- get_virtual_w() / 2) > -100) ? 128 : 80;
        
        // play voice with 40 ms of delay (distance simulation of fireworks)
		SND_SetVoice(8 + voice, (explosion_is_stereo) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT,
            explosion_freq, 40, explosion, explosion_size, l, r, NULL);
	}

}

char filename[1024];

void demo()
{
    int frame=0;
    int n,m;
    
    int pause_voices=0;

    int button_triangle=0;
    int button_square=0;

    u32 color2;
    s16 xx,yy;
    float f;

    int current_file_audio=-1;
    s32 dir;

    char *string1;
    char first_str[]          = "Playing MP3 from Memory";
    char estrayk_str[]        = "MP3 Sample by Estrayk";
    char audio_str[]          = "Playing Audio from USB";
    char hermes_str[]         = "SPU Sound Lib Sample by Hermes";
    char press_cross_str[]    = "Press 'X' To Exit And 'O' For Effects";
    char press_triangle_str[] = "Press /\\ To play OGG/MP3 from /dev_usb/audio";
    char press_square_str[]   = "Press [] To pause fireworks sound";

    s16 stars[256][2];

    struct _fireworks {
        float x,y;
        int force;
        u32 color;
        }
    fireworks[8];

    struct _particles_d {
        float dx,dy;
        }
    particles_d[256];

    int effect_size_samples = 48000*20*4; // initial size of buffer to decode (for 48000 samples x 20 seconds and 16 bit stereo as reference)
    short *effect_samples   = (short *) malloc(effect_size_samples);
    int effect_freq;
    int effect_is_stereo;


    s_printf("Decoding Effect\n");
    scr_flip();

    // decode the mp3 effect file included to memory. It stops by EOF or when samples exceed size_effects_samples

    DecodeAudio( (void *) effect_mp3_bin, sizeof(effect_mp3_bin), effect_samples, &effect_size_samples, &effect_freq, &effect_is_stereo);

    // adjust the sound buffer sample correctly to the effect_size_samples
    {
        // SPU dma works aligned to 128 bytes. SPU module is designed to read unaligned buffers and it is better thing aligned buffers)

        short *temp = (short *)memalign(128, SPU_SIZE(effect_size_samples));

        memcpy((void *) temp, (void *) effect_samples, effect_size_samples);

        free(effect_samples);

        effect_samples =  temp;

    }

    explosion_size = 8000*3*2; // 8000 Hz * 3 seconds * 2 bytes (MONO 16 bits)

    explosion      = (short *) malloc(explosion_size);
    
    
    s_printf("Decoding Explosion\n");
    scr_flip();

    // decode the Ogg sound file included to memory. It stops by EOF or when samples exceed explosion_size

    DecodeAudio( (void *) sound_ogg_bin, sizeof(sound_ogg_bin), explosion, &explosion_size, &explosion_freq, &explosion_is_stereo);

    /* NOTE here "explosion" can be unaligned to the SPU dma and i don´t adjust the buffer
        i asume "explosion_size" is sufficient and not expensive with memory
    */

    
    /******************************************************************/
    /* IMPORTANT!!!: You must call SND_Pause(0) before to use voices. */
    /******************************************************************/

    SND_Pause(0);

    s_printf("Playing Effect\n");
    scr_flip();
    
    SND_SetVoice( 2, (effect_is_stereo) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT, effect_freq, 0, effect_samples, effect_size_samples, 255, 255, NULL);

    s_printf("Playing MP3\n");
   
    /* 
    open as memory device the 2003.mp3 sample. It return 0x666 to 0x669 value and i use it to switch internally the device used.

    PlayAudio functions and StopAudio() closes the device calling internally to mem_close()

    */

    FILE *fp = (FILE *) mem_open((void *) m2003_mp3_bin, sizeof(m2003_mp3_bin));
   
    /* Play the MP3 using the fp returned by fopen() or mem_open. Note the AUDIO_INFINITE_FLAG to play it continuously */

    if(PlayAudiofd(fp, 0,  AUDIO_INFINITE_TIME)==0) inited|= INITED_AUDIOPLAYER;

    // initializes the sin /cos int table

    init_tabsenofunc();
        
    // fix random sequence

    srand(1);

    // fireworks init
    for(m = 0; m < 8; m++)
        fireworks[m].force=-1;

    // stars init

    for(n = 0; n < 256; n++) {
        
        int ang = rand() & 16383;

        stars[n][0] = (rand() % (get_virtual_w() + 128)) - 128;
        stars[n][1] = (rand() % get_virtual_h());

        f = ((float)((1 + (rand() & 255)) * 2)) / 128.0f;

        particles_d[n].dx = f *((float) (sin_int((ang) & 16383)) / 16384.0f);
        particles_d[n].dy = f *((float) (cosin_int((ang) & 16383)) / 16384.0f);
        
        }

        
        while(1) {
            
            // clear the screen

            cls();

            // read the pads

            ioPadGetInfo(&padinfo);

            for(n = 0; n < MAX_PADS; n++) {
            
                if(padinfo.status[n]) {
                    
                    ioPadGetData(n, &paddata);
                    
                    if(paddata.BTN_CROSS && frame>60*12){
                        
                    goto out;				
        
                    }
                    
                    if(paddata.BTN_CIRCLE){
                    
                    // pressing CIRCLE play voice 5 at 144000 Hz (above its natural frequency)

                    SND_SetVoice( 5, (effect_is_stereo) ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT, 144000, 
                        0, effect_samples, effect_size_samples, 255, 255, NULL);			
        
                    }
                    if(paddata.BTN_TRIANGLE){
                    
                        if(!button_triangle) {
                            
                            play_next_audio:

                            if(current_file_audio < 0 && sysFsOpendir("/dev_usb/audio/", &dir) == 0) 
                                current_file_audio = 0;

                           
                            if(current_file_audio >= 0) {

                                while(1) {
                                    u64 read = sizeof(sysFSDirent);
                                    sysFSDirent entry;

                                    if(sysFsReaddir(dir, &entry, &read)<0 || read <= 0) {
                                        if(read> sizeof(sysFSDirent)) exit(0);
                                        sysFsClosedir(dir);
                                        current_file_audio = -2;
                                        break;
                                        }
                                  
                                    if(strstr(entry.d_name, ".mp3") || strstr(entry.d_name, ".ogg")) {
                                        
                                        sprintf(filename, "/dev_usb/audio/%s", (char *) entry.d_name);
                                        
                                        // playing Audio from USB (it stops previous Ogg/MP3)
                                        if(PlayAudio(filename, 0, AUDIO_ONE_TIME)==0) inited|= INITED_AUDIOPLAYER;
                                        break;
                                    }

                                    current_file_audio++;
                                    
                                }

                            
                            }
                            
                            // if it fails play the default mp3
                            if(current_file_audio<0) {
                                FILE *fp = (FILE *) mem_open((void *) m2003_mp3_bin, sizeof(m2003_mp3_bin));

                                filename[0] = 0;
                                
                                /* Play the MP3 using the fp returned by fopen() or mem_open. Note the AUDIO_INFINITE_FLAG to play it continuously */

                                if(PlayAudiofd(fp, 0,  AUDIO_INFINITE_TIME)==0) inited|= INITED_AUDIOPLAYER;
                            }

                            }
                        button_triangle = 1;		
        
                    } else button_triangle = 0;

                    if(paddata.BTN_SQUARE) {
                    
                        // pause fireworks voices here
                        if(!button_square) {
                            pause_voices ^= 1;
                            for(m = 8; m < 16; m++)  SND_PauseVoice(m, pause_voices);
                        }

                    button_square=1;

                    } else button_square = 0;
                   
                }
            }
            
            // Test if OGG/MP3 from device finish
            
            if(current_file_audio>=0 && (StatusAudio()==AUDIO_STATUS_EOF || StatusAudio()==AUDIO_STATUS_ERR))
                 goto play_next_audio;
                 

            // draw the background stars

            for(n = 0; n < 256; n++)
            {
            
                xx = (s16) (stars[n][0]);
                yy = (s16) (stars[n][1]);

                if(n & 1) color2 = 0xffffffff;
                else color2 = 0xffcfcfcf;
                
                if((rand() & 31) != 1) // tililar
                    {
                
                    if(!(xx < 0 || xx >= get_virtual_w() ||  yy < 0 ||  yy >= get_virtual_h()))
                            virtual_scr[xx + yy * get_virtual_w()]= color2;
                    }
            }
            
            // draw fireworks from the second 10

            if(frame > 60*10)
                for(m = 0;m < 8; m++) {
                    
                    if(fireworks[m].force <= -1 && ((rand() >> 8) & 15) == 0) {

                        fireworks[m].x = get_virtual_w() / 2 + (rand() & 511) - 255;
                        fireworks[m].y = get_virtual_h() / 2 + (rand() & 255) - 127;
                        fireworks[m].force = 127;

                        // set color for fireworks

                        switch((rand() >> 8) % 5) {
                            case 0:
                                fireworks[m].color = 0xffffff;
                            break;

                            case 1:
                                fireworks[m].color = 0xff2f2f;
                            break;

                            case 2:
                                fireworks[m].color = 0x2fff2f;
                            break;

                            case 3:
                                fireworks[m].color = 0xffff2f;
                            break;

                            case 4:
                                fireworks[m].color = 0x3f3fff;
                            break;
                            }

                        // try assing voice to firework (0 to 7)
                        if(!pause_voices)
                            snd_explo(m, fireworks[m].x);

                        }


                    f = ((float) (128 - fireworks[m].force)) / 2.0f;
                    
                    if(fireworks[m].force >= 0) {
                        int alpha;
                            
                        color2 = fireworks[m].color;
                        
                        alpha = fireworks[m].force;

                        if(alpha < 0) alpha = 0;
                        color2 |= alpha << 25;
                        
                        for(n = 0; n < 256; n++) {
                            xx = (s16) (fireworks[m].x + particles_d[n].dx * f);
                            yy = (s16) (fireworks[m].y + particles_d[n].dy * f);

                            if(!(xx < 0 || xx >= get_virtual_w() || yy < 0 ||  yy >= get_virtual_h()))
                                virtual_scr[xx + yy * get_virtual_w()] = color2;
                        
                            }

                        if(frame & 1)
                            fireworks[m].force -= 2;
                        }

                    }
                
                
                // display  estrayk_str string

                if(frame > 60*10) {
                    if(current_file_audio<0)
                        string1 = estrayk_str;
                    else
                        string1 = audio_str;
                    }
                else 
                    string1 = first_str;

                
                PX = get_virtual_w() / 2 - (strlen(string1) * 16) / 2;	

                for(n = 0; n < strlen(string1); n++) {
                    if(((frame & 32) != 0) ^ (n & 1))
                        color= 0xffffffff; 
                    else
                        color= 0xffcfcfcf; 

                    PY=132 + (8 * sin_int((((n +((frame >> 2) & 15)) * 2048)) & 16383)) / 16384;
                    s_printf("%c", string1[n]);
                    
                    }

                // display hermes_str string

                PX = get_virtual_w() / 2 - (strlen(hermes_str) *16) / 2;
                color= 0xffffffff; 
                            
                for( n = 0; n < strlen(hermes_str); n++) {

                    switch((n + (frame >> 2)) & 3) {
                        case 0:
                            color = 0xff4fff4f;
                        break;
                        
                        case 1:
                            color = 0xffff4f4f;
                        break;
                        
                        case 2:
                            color = 0xff4fffff;
                        break;
                        
                        case 3:
                            color = 0xff4f4fff;
                        break;
                        }

                    PY= 280 + (8 * sin_int((((n +((frame >> 2) & 15)) * 2048)) & 16383)) / 16384;
                    s_printf("%c", hermes_str[n]);

                    }

                PX = 0;
                color = 0xffffffff; 
                            
                // display cross / square / triangle string

                switch((frame / 120) & 3) {
                    
                    case 1:
                        string1 = press_triangle_str;
                    break;

                    case 3:
                        string1 = press_square_str;
                    break;

                    default:
                        string1 = press_cross_str;
                    break;
                }

                if( frame > 60*10)
                    {
                    PX= get_virtual_w() / 2 - (strlen(string1) * 16) / 2;

                    for( n = 0; n < strlen(string1); n++) {
                        
                        PY = 430 + (2 * sin_int((((n + ((frame >> 2) & 15)) * 2048)) & 16383)) / 16384;
                        s_printf("%c", string1[n]);

                        }
                    }

            // display statistics

            PY = 0; PX = 0;
            color= 0xffffffff;
            
            s_printf("Voices: ");
            
            for(n = 0; n < 16; n++) {

                if(SND_StatusVoice(n) == SND_WORKING)
                    color = 0xffffff00;
                else 
                    color = 0xff0000ff;

                s_printf("%c", 48 + n + 7* ( n > 9));
                }

            color = 0xffffffff;
            
        
            s_printf(" MP3 Time: %u", GetTimeAudio () / 1000);

            frame++;

            color = 0xff00ff00;
            s_printf("\n%s", filename);
          
            scr_flip();
           
            sysUtilCheckCallback();

            }
    out:
 
    cls();
    color = 0xffffffff;

    s_printf("Closing Audioplayer\n");
    scr_flip();

    StopAudio();
    inited &= ~INITED_AUDIOPLAYER;
}

int main(int argc, const char* argv[])
{
	
	u32 entry = 0;
	u32 segmentcount = 0;
	sysSpuSegment* segments;

	scr_init();
	ioPadInit(7);

	// register exit callback
	if(sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sys_callback, NULL)==0) inited |= INITED_CALLBACK;

    #ifdef FROM_FILE
    
    FILE *fp = fopen("/dev_usb/spu_soundmodule.bin", "r");

    if(!fp) return 0;

    fseek(fp, 0, SEEK_END);
    
    int len = ftell(fp);

    spu_soundmodule_bin = malloc(len);
    
    fseek(fp, 0, SEEK_SET);

    fread(spu_soundmodule_bin, 1, len, fp);

    fclose(fp);

    #endif


	s_printf("Initializing SPUs... ");
	s_printf("%08x\n", sysSpuInitialize(6, 5));
    scr_flip();

	s_printf("Initializing raw SPU... ");
	s_printf("%08x\n", sysSpuRawCreate(&spu, NULL));
    scr_flip();

	s_printf("Getting ELF information... ");
	s_printf("%08x\n", sysSpuElfGetInformation(spu_soundmodule_bin, &entry, &segmentcount));
	s_printf("\tEntry Point: %08x\n\tSegment Count: %08x\n", entry, segmentcount);
    scr_flip();

	size_t segmentsize = sizeof(sysSpuSegment) * segmentcount;
	segments = (sysSpuSegment*)memalign(128, SPU_SIZE(segmentsize)); // must be aligned to 128 or it break malloc() allocations
	memset(segments, 0, segmentsize);

	s_printf("Getting ELF segments... ");
	s_printf("%08x\n", sysSpuElfGetSegments(spu_soundmodule_bin, segments, segmentcount));
    scr_flip();

	s_printf("Loading ELF image... ");
	s_printf("%08x\n", sysSpuImageImport(&spu_image, spu_soundmodule_bin, 0));
    scr_flip();

	s_printf("Loading image into SPU... ");
	s_printf("%08x\n", sysSpuRawImageLoad(spu, &spu_image));
    scr_flip();

    inited |= INITED_SPU;

	if(SND_Init(spu)==0) inited |= INITED_SOUNDLIB;

	s_printf("Running the sample demo...\n");
    scr_flip();

    demo();

	s_printf("Exiting...\n");
    scr_flip();
	
	release_all();

    sleep(1);


	return 0;
}
