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
{ 
	SDL_DestroyMutex(stream_mutex);
}




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

void Mixer::cancel(void)
{
	while(buffers.begin()->num_samples)
		advance();
}

void Mixer::fill_audio_func(void *udata,Uint8 *stream,int len)
{
#if DEBUG
	cout << "fill_audio_func: " << len << endl;
	// cout << "fill_audio_func(aux): " << auxilliary_audio << endl;
#endif
	advance();
	// if(buffers.begin()->num_samples==0&&auxilliary_audio==-1)
	stream_lock();
	if(buffers.begin()->num_samples==0&&audio_streams.size()==0)
		{
#if DEBUG
		cerr << "No more audio data" << endl;
#endif
		SDL::PauseAudio(1);
		stream_unlock();
		return;
		}
	if(audio_streams.size()!=0)
		{
		int which=0;
		vector<list<ProducerConsumerBuf *>::iterator> close_list;
		for(list<ProducerConsumerBuf *>::iterator it=audio_streams.begin();
			it!=audio_streams.end();++it)
				{
				
				Uint8	*temp_buffer=new Uint8[len];
				int	ret=0;
				size_t	sofar=0;
				memset(temp_buffer,silence,len);
				while((len-sofar))
					{
					ret=(*it)->consume((char*)temp_buffer+sofar,len-sofar);
					if(ret<=0)
						break;
					sofar+=ret;
					}
#if DEBUG
				cerr << "(" << which <<"/"<<audio_streams.size()<< ")" << " Mixing auxilliary data " ;
				cerr << sofar << " of " << (*it)->size() << endl;
#endif
				if(len-sofar&&ret==-1)
					{
					// perror("consume");
					// delete the entry
					close_list.push_back(it);
					delete [] temp_buffer;
					continue;
					}
				compress_audio_sample(temp_buffer,len);
				SDL::MixAudio(stream, temp_buffer, len, SDL_MIX_MAXVOLUME);
				delete [] temp_buffer;
				++which;
				}
			for(vector<list<ProducerConsumerBuf *>::iterator>::iterator it=close_list.begin();it!=close_list.end();++it)
				{
				(**it)->end_consumption();
				audio_streams.erase(*it);
				}
		}
	stream_unlock();
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

// Mixer::Mixer(Uint32 __buffer_size,Uint32 ringsize,Uint8 silence_value) : auxilliary_audio(-1)
Mixer::Mixer(Uint32 __buffer_size,Uint32 ringsize,Uint8 silence_value) : audio_streams(),stream_mutex(SDL_CreateMutex())
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


#if 0
void	Mixer::set_auxilliary_audio(int fh)
{
	auxilliary_audio=fh;
#if DEBUG
	cout << "Auxilliary audio stream: " << auxilliary_audio << endl;
#endif
	SDL::PauseAudio(0);
	SDL::UnlockAudio();
}
#endif

ProducerConsumerBuf	*Mixer::Create_Audio_Stream(void)
{
	ProducerConsumerBuf	*pcb=new ProducerConsumerBuf;

	SDL::PauseAudio(1);
	stream_lock();
#if DEBUG
	cerr << "Create_Audio_Stream()" << endl;
#endif
	audio_streams.push_back(pcb);
	stream_unlock();
	SDL::PauseAudio(0);
	SDL::UnlockAudio();
	return pcb;
}

void	Mixer::Destroy_Audio_Stream(Uint32 id)
{
	if(id==0)
		return;	// We don't honour id 0
	SDL::PauseAudio(1);
	stream_lock();
	for(list<ProducerConsumerBuf *>::iterator it=audio_streams.begin();
		it!=audio_streams.end();++it)
			{
			ProducerConsumerBuf *p=*it;
			if(p->id==id)
				{
				p->end_consumption();
				audio_streams.erase(it);
				break;
				}
			}
	stream_unlock();
	SDL::PauseAudio(0);
}

bool	Mixer::is_playing(Uint32 id)
{
	if(id==0)
		return false; // We don't honor id 0
	stream_lock();
	for(list<ProducerConsumerBuf *>::iterator it=audio_streams.begin();
		it!=audio_streams.end();++it)
			{
			ProducerConsumerBuf *p=*it;
			if(p->id==id)
				{
				stream_unlock();
				return true;
				}
			}
	stream_unlock();
	return false;
}
