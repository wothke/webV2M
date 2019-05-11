/*
* This file adapts "Farbrausch V2M player" to the interface expected by my generic JavaScript player..
*
* Copyright (C) 2018 Juergen Wothke
*
* Credits:   https://github.com/jgilje/v2m-player
*
* License: This WebAudio port of the V2M lib uses the same license as the original code, 
*          i.e. The Artistic License 2.0 (see separate LICENSE file)
*/


/*
	known issues: "kZ - Glary Utilities 2.x.x.x crk.v2m": for some reason sound 
	overamplifies (seems to be linked to activated BUG_V2_FM_RANGE bug)
*/
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>     

#include <iostream>
#include <fstream>

#include <sounddef.h>
#include <v2mplayer.h>
#include <v2mconv.h>
#include <scope.h>

#define CHANNELS 2				
#define BYTES_PER_SAMPLE 2
#define SAMPLE_BUF_SIZE	1024
#define SAMPLE_FREQ	44100


float synth_buffer[SAMPLE_BUF_SIZE * CHANNELS];
int16_t sample_buffer[SAMPLE_BUF_SIZE * CHANNELS];
int samples_available= 0;
long sample_time= 0;
char* info_texts[1];

#define TEXT_MAX	255
char title_str[TEXT_MAX];

static V2MPlayer v2m;


struct StaticBlock {
    StaticBlock(){
		info_texts[0]= title_str;
    }
};

static StaticBlock g_emscripen_info;

unsigned char* buffer_converted= 0;

void * get_buffer_converted(const void * inBuffer, size_t inBufSize) {
    int version = CheckV2MVersion((unsigned char*)inBuffer, inBufSize);
    if (version < 0) {
        return NULL;		// fatal error
    }

    int converted_length;
    ConvertV2M((const unsigned char*)inBuffer, inBufSize, &buffer_converted, &converted_length);

	return buffer_converted;
}

extern "C" void emu_teardown (void)  __attribute__((noinline));
extern "C" void EMSCRIPTEN_KEEPALIVE emu_teardown (void) {
	if (buffer_converted) {
		delete buffer_converted;		// allocated using "new" (see ConvertV2M)
		buffer_converted= 0;		
	}
	
	sdClose();
	
	v2m.Close();	// redundant: already used in new Open
	
	sample_time= 0;
}

extern "C"  int emu_load_file(char *filename, void * inBuffer, uint32_t inBufSize)  __attribute__((noinline));
extern "C"  int EMSCRIPTEN_KEEPALIVE emu_load_file(char *filename, void * inBuffer, uint32_t inBufSize) {
	if (inBufSize<1000) return 1;	// just some file not found message (todo: catch in player.)
	
    sdInit();
	
	inBuffer= get_buffer_converted(inBuffer, inBufSize);

	v2m.Init();
	if (inBuffer && v2m.Open((uint8_t *)inBuffer, SAMPLE_FREQ)) {
		v2m.Play();
		
		std::string title= filename;
		title.erase( title.find_last_of( '.' ) );	// remove ext
		snprintf(title_str, TEXT_MAX, "%s", title.c_str());
		
		return 0;		
	}
	return 1;
}

extern "C" int emu_get_sample_rate() __attribute__((noinline));
extern "C" EMSCRIPTEN_KEEPALIVE int emu_get_sample_rate()
{
	return SAMPLE_FREQ;
}
 
extern "C" int emu_set_subsong(int subsong, unsigned char boost) __attribute__((noinline));
extern "C" int EMSCRIPTEN_KEEPALIVE emu_set_subsong(int track, unsigned char scope) {
	scope_activated= scope;

	return 0;
}

extern "C" const char** emu_get_track_info() __attribute__((noinline));
extern "C" const char** EMSCRIPTEN_KEEPALIVE emu_get_track_info() {
	return (const char**)info_texts;
}

extern "C" char* EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer(void) __attribute__((noinline));
extern "C" char* EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer(void) {
	return (char*)sample_buffer;
}

extern "C" long EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer_length(void) __attribute__((noinline));
extern "C" long EMSCRIPTEN_KEEPALIVE emu_get_audio_buffer_length(void) {
	return samples_available;
}

extern "C" int emu_get_current_position() __attribute__((noinline));
extern "C" int EMSCRIPTEN_KEEPALIVE emu_get_current_position() {
    return sample_time / emu_get_sample_rate();
}

extern "C" void emu_seek_position(int sec) __attribute__((noinline));
extern "C" void EMSCRIPTEN_KEEPALIVE emu_seek_position(int sec) {
	v2m.Play(sec*1000);
	sample_time= sec * emu_get_sample_rate();
}

extern "C" int emu_get_max_position() __attribute__((noinline));
extern "C" int EMSCRIPTEN_KEEPALIVE emu_get_max_position() {
	return v2m.Length(); // in sec
}

extern "C" int emu_compute_audio_samples() __attribute__((noinline));
extern "C" int EMSCRIPTEN_KEEPALIVE emu_compute_audio_samples() {
	if (emu_get_current_position() >= emu_get_max_position()) {
		samples_available= 0;
		return 1;
	}

	v2m.Render(synth_buffer, SAMPLE_BUF_SIZE);
	samples_available= SAMPLE_BUF_SIZE;
	
	// convert float to int16 /(just so the player can be merged more easyly with others)
	for (int i= 0; i<samples_available*CHANNELS; i++) {
		double v= 0x8000*(double)synth_buffer[i];
		if (v < -32760) v= -32760;
		if (v > 32760) v= 32760;
		
		sample_buffer[i]= (int16_t)v;
	}
	
	sample_time+= samples_available;
	return 0;
}

extern "C" int emu_number_trace_streams() __attribute__((noinline));
extern "C" int EMSCRIPTEN_KEEPALIVE emu_number_trace_streams() {
	return 16;
}
extern "C" const char** emu_get_trace_streams() __attribute__((noinline));
extern "C" const char** EMSCRIPTEN_KEEPALIVE emu_get_trace_streams() {
	return (const char**)get_scope_buffers();	// ugly cast to make emscripten happy
}
extern "C" const char** emu_get_trace_titles() __attribute__((noinline));
extern "C" const char** EMSCRIPTEN_KEEPALIVE emu_get_trace_titles() {
	return (const char**)get_scope_titles();
}
extern "C" const char* emu_get_voice_map() __attribute__((noinline));
extern "C" const char* EMSCRIPTEN_KEEPALIVE emu_get_voice_map() {
	return (const char*)v2m.GetVoiceMap();	// ugly cast to make emscripten happy
}
