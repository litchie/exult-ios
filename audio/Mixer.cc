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

using std::cerr;
using std::endl;
using std::list;
using std::vector;
static const int Mixer_Sample_Magic_Number=0x55443322;

//---- Mixer ---------------------------------------------------------

Mixer::~Mixer()
{ 
	SDL_DestroyMutex(stream_mutex);
}




 /* The audio function callback takes the following parameters:
     stream:  A pointer to the audio buffer to be filled
     len:     The length (in bytes) of the audio buffer
 */
 
void fill_audio(void *udata, uint8 *stream, int len)
 {
	// cerr << "fill_audio: " << len << endl;
	 Audio::get_ptr()->mixer->fill_audio_func(udata,stream,len);
}

void	compress_audio_sample(uint8 *buf,int len)
{
	return;
	uint8	*dbuf=new uint8[len*2];
	uint8	*source=buf;
	uint8	*dest=dbuf;
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

void Mixer::cancel_raw(void)
{
	Audio::get_ptr()->Destroy_Audio_Stream(Mixer_Sample_Magic_Number);
}

void Mixer::fill_audio_func(void *udata,uint8 *stream,int len)
{
#if 0
	cout << "fill_audio_func: " << len << endl;
	// cout << "fill_audio_func(aux): " << auxilliary_audio << endl;
#endif
	stream_lock();
	if(audio_streams.size()==0)
		{
#if DEBUG
		cerr << "No more audio data" << endl;
#endif
		SDL::PauseAudio(1);
		stream_unlock();
		return;
		}
	else
		{
		int which=0;
		vector<list<ProducerConsumerBuf *>::iterator> close_list;
		for(list<ProducerConsumerBuf *>::iterator it=audio_streams.begin();
			it!=audio_streams.end();++it)
				{
				
#ifdef NOT_YET
				// The world is not ready for this yet
				if((*it)->size()<buffer_length)
					continue;
#endif
				uint8	*temp_buffer=new uint8[len];
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
#if 0
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
#if 0
	if(buffers.begin()->num_samples)
		{
#if 0
		cerr << "Mixing sample data" << endl;
#endif
		if((unsigned)len>buffers.front().length)
			len=buffers.front().length;
		SDL::MixAudio(stream, buffers.begin()->buffer, len, SDL_MIX_MAXVOLUME);
		}
#endif
}

void	Mixer::play(uint8 *sound_data,uint32 len)
{
	ProducerConsumerBuf *audiostream=Audio::get_ptr()->Create_Audio_Stream();
	audiostream->id=Mixer_Sample_Magic_Number;
	audiostream->produce(sound_data,len);
	audiostream->end_production();
}

Mixer::Mixer(uint32 __buffer_size,uint32 ringsize,uint8 silence_value) : audio_streams(),stream_mutex(SDL_CreateMutex())
{
	buffer_length=__buffer_size;
	silence=silence_value;
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

void	Mixer::Destroy_Audio_Stream(uint32 id)
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

bool	Mixer::is_playing(uint32 id)
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

void Mixer::cancel_streams(void)
{
	stream_lock();
	for(list<ProducerConsumerBuf *>::iterator it=audio_streams.begin();
		it!=audio_streams.end();++it)
			{
			ProducerConsumerBuf *p=*it;
			p->end_consumption();
			}
	audio_streams.clear();
	stream_unlock();
}

