#if __GNUG__ >= 2
#  pragma implementation
#endif

#include "Mixer.h"

#include <SDL_audio.h>
#include <SDL_error.h>

#include <cstdio>
#include <unistd.h>

#define	TRAILING_VOC_SLOP 32

#include <SDL_audio.h>
#include <SDL_timer.h>
#include <csignal>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <cstdlib>
#include <cstring>

#include "Audio.h"


//---- Mixer ---------------------------------------------------------

Mixer::~Mixer()
{ }




 /* The audio function callback takes the following parameters:
     stream:  A pointer to the audio buffer to be filled
     len:     The length (in bytes) of the audio buffer
 */
 
void fill_audio(void *udata, Uint8 *stream, int len)
 {
	 extern	Audio	audio;
	cerr << "fill_audio: " << len << endl;
	 audio.mixer->fill_audio_func(udata,stream,len);
}

void	Mixer::advance(void)
{
	MixBuffer m=buffers.front();
	buffers.pop_front();
	m.length=0;
	m.num_samples=0;
	memset(m.buffer,silence,buffer_length);
	buffers.push_back(m);
}

void Mixer::fill_audio_func(void *udata,Uint8 *stream,int len)
{
	cout << "fill_audio_func: " << len << endl;
	advance();
	if(buffers.begin()->num_samples==0)
		{
		cout << "No more audio data" << endl;
		SDL_PauseAudio(1);
		return;
		}
	if((unsigned)len>buffers.front().length)
		len=buffers.front().length;
	SDL_MixAudio(stream, buffers.begin()->buffer, len, SDL_MIX_MAXVOLUME);
}

void	Mixer::play(Uint8 *sound_data,Uint32 len)
{
	list<MixBuffer>::iterator it=buffers.begin();
	size_t	offset,rlen;

	len-=64;
	++it;

	for(size_t i=0;i<len/buffer_length+1;i++)
		{
		if(it==buffers.end())
			{
			continue;
			}
		offset=i*buffer_length;
		if(offset+buffer_length>len)
			rlen=len-offset;
		else
			rlen=buffer_length;
		SDL_MixAudio(it->buffer,sound_data+offset,rlen,SDL_MIX_MAXVOLUME);
		++it->num_samples;
		if(it->length<rlen)
			it->length=rlen;
		++it;
		}
		
	SDL_PauseAudio(0);
	SDL_UnlockAudio();
	
}

Mixer::Mixer(Uint32 __buffer_size,Uint32 ringsize,Uint8 silence_value)
{
	buffer_length=__buffer_size;
	ring_size=ringsize;
	silence=silence_value;
	for(size_t i=0;i<ringsize;i++)
		{
		MixBuffer	tmp(buffer_length,silence);
		buffers.push_back(tmp);
		}
}


