/*
Copyright (C) 2000  Dancer A.L Vesperman

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#if (__GNUG__ >= 2) && (!defined WIN32)
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
#if defined(MACOS)
  #include <stat.h>
#else
  #include <sys/stat.h>
  #include <sys/types.h>
#endif
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
	// cerr << "fill_audio: " << len << endl;
	 audio->mixer->fill_audio_func(udata,stream,len);
}

void	Mixer::advance(void)
{
	MixBuffer m(buffers.front());
	buffers.pop_front();
	m.length=0;
	m.num_samples=0;
	memset(m.buffer,silence,buffer_length);
	buffers.push_back(m);
}

void	compress_audio_sample(Uint8 *buf,int len)
{
	return;
	Uint8	*dbuf=new Uint8[len*2];
	Uint8	*source=buf;
	Uint8	*dest=dbuf;
	while(len>0)
		{
		// Left channel
		*dest=(*source+*(source+2))/2;
		++dest;
		++source;
		// Right channel
		*dest=(*source+*(source+2))/2;
		++dest;
		source+=3;
		len-=2;
		}
	memcpy(buf,dbuf,len);
	delete [] dbuf;
}

void Mixer::fill_audio_func(void *udata,Uint8 *stream,int len)
{
#if DEBUG
	// cout << "fill_audio_func: " << len << endl;
	// cout << "fill_audio_func(aux): " << auxilliary_audio << endl;
#endif
	advance();
	if(buffers.begin()->num_samples==0&&auxilliary_audio==-1)
		{
#if DEBUG
		cerr << "No more audio data" << endl;
#endif
		SDL::PauseAudio(1);
		return;
		}
	if(auxilliary_audio!=-1)
		{
#if DEBUG
		//cerr << "Mixing auxilliary data" << endl;
#endif
		Uint8	*temp_buffer=new Uint8[len];
		size_t	sofar=0;
		memset(temp_buffer,silence,len);
		while((len-sofar))
			{
			int ret=read(auxilliary_audio,(char*)temp_buffer+sofar,len-sofar);
			if(ret==-1)
				break;
			sofar+=ret;
			}
		if(len-sofar)
			{
			perror("read");
			close(auxilliary_audio);
			auxilliary_audio=-1;
			delete [] temp_buffer;
			return;
			}
		compress_audio_sample(temp_buffer,len);
		SDL::MixAudio(stream, temp_buffer, len, SDL_MIX_MAXVOLUME);
		delete [] temp_buffer;
		}
	if(buffers.begin()->num_samples)
		{
#if DEBUG
		cerr << "Mixing sample data" << endl;
#endif
		if((unsigned)len>buffers.front().length)
			len=buffers.front().length;
		SDL::MixAudio(stream, buffers.begin()->buffer, len, SDL_MIX_MAXVOLUME);
		}
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
		SDL::MixAudio(it->buffer,sound_data+offset,rlen,SDL_MIX_MAXVOLUME);
		++it->num_samples;
		if(it->length<rlen)
			it->length=rlen;
		++it;
		}
		
	SDL::PauseAudio(0);
	SDL::UnlockAudio();
	
}

Mixer::Mixer(Uint32 __buffer_size,Uint32 ringsize,Uint8 silence_value) : auxilliary_audio(-1)
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


void	Mixer::set_auxilliary_audio(int fh)
{
	auxilliary_audio=fh;
#if DEBUG
	cout << "Auxilliary audio stream: " << auxilliary_audio << endl;
#endif
	SDL::PauseAudio(0);
	SDL::UnlockAudio();
}
