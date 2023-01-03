#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pd_api.h"
#include "playdate.h"
#include "stb_image.h"
#include "../../PureDOOM.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_MALLOC PD_malloc
#define STBI_REALLOC PD_realloc
#define STBI_FREE PD_free
#include "stb_image.h"

#define MIN(A,B)		(((A)<(B)) ? (A) : (B))
#define MAX(A,B)		(((A)>(B)) ? (A) : (B))
#define CLAMP(V,A,B)	(MIN(MAX(V,A),B))

PlaydateAPI*	playdate;

const char *noiseOptions[] = {"0", "1", "2", "3", "4", "Full"};
PDMenuItem *noiseOptionMenuItem;
float		noise_value;

const char *bayernOptions[] = {"2x2","4x4"};
PDMenuItem *bayernOptionMenuItem;
int			bayern_type = 0;

int dithering_width;
int dithering_height;
byte* bluenoise_image;
byte dithering_image[400*240];

uint8_t playdate_palette[256];

// #define	AUDIO_BUFFER_SIZE 2048
// uint8_t			audioBuffer[AUDIO_BUFFER_SIZE];
// AudioSample*	pAudioSample;
// SamplePlayer*	pAudioPlayer;

void core_print(const char* str) { playdate->system->logToConsole( str ); }
void* core_malloc(int size)
{
	void* pResult = PD_malloc( (size_t)size );
	if ( pResult==NULL ) {
		playdate->system->error("failed allocation");
	}
	return pResult;
}
#define core_free PD_free
void core_gettime(int* sec, int* usec)
{
	unsigned int time = playdate->system->getCurrentTimeMilliseconds();
	int time_sec = (int)(time/1000);
	int time_millisec = (int)(time - time_sec*1000);

	*sec = time_sec;
	*usec = time_millisec*1000;
}
void core_exit(int code){
	playdate->system->error("Bye");
}
char* core_getenv(const char* var)
{
	// "DOOMWADDIR"
	if ( var[0]=='D' )
	{
		return "./wad_files";
	}

	// "HOME"
	return ".";
}

void convert_palette()
{
	// we convert only the first palette and ignore all palette effect (flash, fadings, etc.)
	uint8_t* default_palette = W_CacheLumpName("PLAYPAL", PU_CACHE);

	for (int i = 0; i < 256; i++)
	{
		uint8_t r = *default_palette++;
		uint8_t g = *default_palette++;
		uint8_t b = *default_palette++;

		int avg = ((int)r + (int)g + (int)b)/3;
		playdate_palette[i] = (uint8_t)avg;
	}
}

uint8_t* doom_framebuffer;
#define PACK_PIXEL(shift)	(playdate_palette[*(gamebuffer++)]>*(dithering++))<<shift
// #define PACK_PIXEL(shift)	(playdate_palette[*(gamebuffer++)]>128)<<shift
// #define PACK_PIXEL(shift)	(*(dithering++)>128)<<shift
//#define PACK_PIXEL(shift)	(*(doom_framebuffer++)>128)<<shift
void convert_frame_buffer()
{
	int x, y, chunkX;
	byte index;

	doom_framebuffer = doom_get_framebuffer(1);

	uint8_t* playdate_framebuffer = playdate->graphics->getFrame();
	byte* gamebuffer;
	byte* dithering;

	int originX = 40;
	int originY = 20;
	uint8_t* framebufferOrigin = playdate_framebuffer + originY*52 + originX/8;

	for (y = 0; y < SCREENHEIGHT; y++)
	{
		for (chunkX = 0; chunkX < SCREENWIDTH/32; chunkX++)
		{
			uint32_t* pChunk = (uint32_t*)framebufferOrigin;
			pChunk += (y*52)/4 + chunkX;
			gamebuffer = doom_framebuffer + y * SCREENWIDTH + chunkX*32;
			dithering = dithering_image + y * dithering_width + chunkX*32;

			*pChunk = 
				PACK_PIXEL(7) |
				PACK_PIXEL(6) |
				PACK_PIXEL(5) |
				PACK_PIXEL(4) |
				PACK_PIXEL(3) |
				PACK_PIXEL(2) |
				PACK_PIXEL(1) |
				PACK_PIXEL(0) |

				PACK_PIXEL(15) |
				PACK_PIXEL(14) |
				PACK_PIXEL(13) |
				PACK_PIXEL(12) |
				PACK_PIXEL(11) |
				PACK_PIXEL(10) |
				PACK_PIXEL(9) |
				PACK_PIXEL(8) |

				PACK_PIXEL(23) |
				PACK_PIXEL(22) |
				PACK_PIXEL(21) |
				PACK_PIXEL(20) |
				PACK_PIXEL(19) |
				PACK_PIXEL(18) |
				PACK_PIXEL(17) |
				PACK_PIXEL(16) |

				PACK_PIXEL(31) |
				PACK_PIXEL(30) |
				PACK_PIXEL(29) |
				PACK_PIXEL(28) |
				PACK_PIXEL(27) |
				PACK_PIXEL(26) |
				PACK_PIXEL(25) |
				PACK_PIXEL(24) ;
		}
	}

	playdate->graphics->markUpdatedRows( originY, originY+SCREENHEIGHT);
}

int core_update(__attribute__ ((unused)) void* userData)
{
	PDButtons button_down;
	PDButtons button_up;
	playdate->system->getButtonState(NULL, &button_down, &button_up);

	if ( button_down&kButtonLeft )	doom_key_down( DOOM_KEY_LEFT_ARROW );
	if ( button_down&kButtonRight )	doom_key_down( DOOM_KEY_RIGHT_ARROW );
	if ( button_down&kButtonUp )	doom_key_down( DOOM_KEY_UP_ARROW );
	if ( button_down&kButtonDown )	doom_key_down( DOOM_KEY_DOWN_ARROW );
	if ( button_down&kButtonA )		doom_button_down( DOOM_LEFT_BUTTON );
	if ( button_down&kButtonB )		doom_key_down( DOOM_KEY_SPACE );

	if ( button_up&kButtonLeft )	doom_key_up( DOOM_KEY_LEFT_ARROW );
	if ( button_up&kButtonRight )	doom_key_up( DOOM_KEY_RIGHT_ARROW );
	if ( button_up&kButtonUp )		doom_key_up( DOOM_KEY_UP_ARROW );
	if ( button_up&kButtonDown )	doom_key_up( DOOM_KEY_DOWN_ARROW );
	if ( button_up&kButtonA )		doom_button_up( DOOM_LEFT_BUTTON );
	if ( button_up&kButtonB )		doom_key_up( DOOM_KEY_SPACE );

	doom_update();

	convert_frame_buffer();
	playdate->system->drawFPS(0, 0);

	// uint32_t midi_msg;
	// while ((midi_msg = doom_tick_midi()) != 0);

	return 1;
}

// void audioCallback(SoundSource* c)
// {
// 	uint8_t* buffer = (uint8_t*)doom_get_sound_buffer(AUDIO_BUFFER_SIZE);
// 	memcpy( audioBuffer, buffer, AUDIO_BUFFER_SIZE);
// 	PD_log( "callback" );
// }

void load_bluenoise_image()
{
	SDFile* fileHandle;
	int fileSize;

	fileHandle = playdate->file->open("bluenoise.png.bin", kFileRead);

	playdate->file->seek(fileHandle, 0, SEEK_END);
	fileSize = playdate->file->tell(fileHandle);
	playdate->file->seek(fileHandle, 0, SEEK_SET);

	char* fileBuffer = PD_malloc(fileSize);
	playdate->file->read(fileHandle, fileBuffer, fileSize);
	bluenoise_image = (byte*)stbi_load_from_memory(fileBuffer, fileSize, &dithering_width, &dithering_height, NULL, 1);

	PD_free(fileBuffer);
	playdate->file->close(fileHandle);
}

const byte bayern_filter_22[2][2]={
	{51,204},
	{153,102},
};
const byte bayern_filter_44[4][4]={
	{15,195,61,240},
	{135,75,180,120},
	{45,225,30,210},
	{165,105,150,90}
};
void generate_dithering_image()
{
	if ( noise_value<0.f )
	{
		memcpy( dithering_image, bluenoise_image, 400*240);
		return;
	}

	// use bayen filter and ad some blue noise in it
	for (int y = 0; y < 240; y++)
	{
		for (int x = 0; x < 400; x++)
		{
			int pixel_index = x+y*400;

			float bluenoise_value = (float)bluenoise_image[pixel_index];
			float bluenoise_value_01 = bluenoise_value / 255.f;
			float bayern_value = (float)(bayern_type ? bayern_filter_44[y%4][x%4] : bayern_filter_22[y%2][x%2]);

			float noise_amplitude = MIN( bayern_value, 255.f - bayern_value);

			float final_value = bayern_value + noise_amplitude*noise_value*(bluenoise_value_01-0.5f);

			dithering_image[pixel_index] = (byte)final_value;
		}
	}
}


void noiseMenuOptionsCallback(void* userdata)
{
	float noise_type = playdate->system->getMenuItemValue( noiseOptionMenuItem );
	if ( noise_type==5 )
	{
		noise_value = -1.f;
	}
	else
	{
		noise_value = (float)noise_type / 4.f;
	}

	generate_dithering_image();
}

void bayernMenuOptionsCallback(void* userdata)
{
	bayern_type = playdate->system->getMenuItemValue( bayernOptionMenuItem );
	generate_dithering_image();
}

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, __attribute__ ((unused)) uint32_t arg)
{
	if ( event == kEventInit )
	{
		// Init the playdate
		playdate = pd;
		playdate->display->setRefreshRate(30);
		playdate->system->setUpdateCallback(core_update, NULL);
		playdate->graphics->clear( kColorBlack);

		bayernOptionMenuItem = playdate->system->addOptionsMenuItem("Bayern", bayernOptions, 2, bayernMenuOptionsCallback, NULL);
		playdate->system->setMenuItemValue( bayernOptionMenuItem, bayern_type);

		noiseOptionMenuItem = playdate->system->addOptionsMenuItem("Noise", noiseOptions, 6, noiseMenuOptionsCallback, NULL);
		playdate->system->setMenuItemValue( noiseOptionMenuItem, 2);
		noise_value = 2.f / 4.f;

		// pAudioSample = playdate->sound->sample->newSampleFromData( audioBuffer, kSound16bitStereo, DOOM_SAMPLERATE, AUDIO_BUFFER_SIZE);
		// pAudioPlayer = playdate->sound->sampleplayer->newPlayer();
		// playdate->sound->sampleplayer->setSample( pAudioPlayer, pAudioSample);
		// playdate->sound->sampleplayer->setFinishCallback( pAudioPlayer, audioCallback);
		// playdate->sound->sampleplayer->play( pAudioPlayer, 0, 1.0);

		// Load data
		load_bluenoise_image();
		generate_dithering_image( .5f );

		// Init Pure Doom
		doom_set_print( core_print );
		doom_set_malloc( core_malloc, core_free );
		doom_set_file_io( PD_open,
						PD_close,
						PD_read,
						PD_write,
						PD_seek,
						PD_tell,
						PD_eof);
		doom_set_gettime( core_gettime );
		doom_set_exit( core_exit );
		doom_set_getenv( core_getenv );

		// Init Doom
		doom_init(0, NULL, 0);
		convert_palette();
	}

	return 0;
}
