/*
Copyright (C) 2000-2001 The Exult Team

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

#include <SDL_audio.h>
#include <SDL_timer.h>
#include "SDL_mapping.h"

#include "Audio.h"
#include "Configuration.h"
#include "fnames.h"
#include "game.h"
#include "U7file.h"
#include "utils.h"
extern	Configuration *config;

#ifndef ALPHA_LINUX_CXX
#  include <csignal>
#  include <cstdio>
#  include <cstdlib>
#  include <cstring>
#  include <iostream>
#endif
#include <fcntl.h>
#include <unistd.h>

#if defined(MACOS)
  #include <stat.h>
#else
  #include <sys/stat.h>
  #include <sys/types.h>
#endif

using std::cerr;
using std::cout;
using std::endl;
using std::exit;
using std::memcpy;
using std::memset;
using std::string;
using std::strncmp;
using std::vector;

#define	TRAILING_VOC_SLOP 32
#define	LEADING_VOC_SLOP 32


//---- Audio ---------------------------------------------------------

Audio::~Audio()
{
	stop_music();
	SDL::CloseAudio();		// 14-jan-2001 - jsf experiment. Gets
					//   rid of crash at end.
	if(mixer)
		{
		cancel_streams();
		cancel_raw();
		}
	if(mixer)
		{
		delete mixer;
		mixer=0;
		}
	if(midi)
		{
		delete midi;
		midi=0;
		}
	// Avoid closing SDL audio. This seems to trigger a segfault
	// SDL::CloseAudio();
	SDL_open=false;
	self=0;
}


static	void resample(uint8 *sourcedata,uint8 **destdata,size_t sourcelen,size_t *destlen,int current_rate,int wanted_rate)
{
	// I have no idea what I'm doing here - Dancer
	// This is really Breshenham's line-drawing algorithm in
	// a false nose, and clutching a crude smoothing loop.

	float	f=wanted_rate/current_rate;
	*destlen=(sourcelen*f)+1;
	if(!*destlen||current_rate==wanted_rate)
		{
		// Least work
		*destlen=sourcelen;
		*destdata=new uint8[sourcelen];
		memcpy(*destdata,sourcedata,sourcelen);
		return;
		}
	*destdata=new uint8[*destlen];
	size_t last=0;
	for(size_t i=0;i<sourcelen;i++)
		{
		size_t pos=i*f;
		assert(pos<=*destlen);
		(*destdata)[pos]=sourcedata[i];
		// Interpolate if need be
		if(last!=pos&&last!=pos-1)
			for(size_t j=last+1;j<=pos-1;j++)
				{
				unsigned int x=(unsigned char)sourcedata[i];
				unsigned int y=(unsigned char)sourcedata[i-1];
				x=(x+y)/2;
				(*destdata)[j]=(uint8) x;
				}
		last=pos;
		}
	cerr << "End resampling. Resampled " << sourcelen << " bytes to " << *destlen << " bytes" << endl;
}

void	Audio::mix_audio(void)
{
}

void	Audio::clear(uint8 *buf,int len)
{
	memset(buf,actual.silence,len);
}

extern void fill_audio(void *udata, uint8 *stream, int len);

struct	Chunk
	{
	size_t	length;
	uint8	*data;
	Chunk() : length(0),data(0) {}
	};

static	uint8 *chunks_to_block(vector<Chunk> &chunks)
{
	uint8 *unified_block;
	size_t	aggregate_length=0;
	size_t	working_offset=0;
	
	for(vector<Chunk>::iterator it=chunks.begin();
		it!=chunks.end(); ++it)
		{
		aggregate_length+=it->length;
		}
	unified_block=new uint8[aggregate_length];
	for(vector<Chunk>::iterator it=chunks.begin();
		it!=chunks.end(); ++it)
		{
		memcpy(unified_block+working_offset,it->data,it->length);
		working_offset+=it->length;
		delete [] it->data; it->data=0; it->length=0;
		}
	
	return unified_block;
}

uint8 *Audio::convert_VOC(uint8 *old_data,uint32 &visible_len)
{
	vector<Chunk> chunks;
	size_t	data_offset=0x1a;
	bool	last_chunk=false;
	uint16	sample_rate;
	size_t  l=0;
	size_t	chunk_length;
	

			

	while(!last_chunk)
		{
		switch(old_data[data_offset]&0xff)
			{
			case 0:
#if DEBUG
				cout << "Terminator" << endl;
#endif
				last_chunk=true;
				continue;
			case 1:
#if DEBUG
				cout << "Sound data" << endl;
#endif
				l=(old_data[3+data_offset]&0xff)<<16;
				l|=(old_data[2+data_offset]&0xff)<<8;
				l|=(old_data[1+data_offset]&0xff);
#if DEBUG
				cout << "Chunk length appears to be " << l << endl;
#endif
				sample_rate=1000000/(256-(old_data[4+data_offset]&0xff));

				if(!truthful_)
					{
					// BG and SI use different sample
					// rates from the ones the VOC headers
					// say they do. Why?
					if(sample_rate==9615)
						sample_rate=7380;	// Assume 9615 is a lie.
					if(sample_rate==11111)
						sample_rate=11025;	// Assume 11111 is a lie.
					}
#if DEBUG
				cout << "Sample rate ("<< sample_rate<<") = _real_rate"<<endl;
				cout << "compression type " << (old_data[5+data_offset]&0xff) << endl;
				cout << "Channels " << (old_data[6+data_offset]&0xff) << endl;
#endif
				chunk_length=l+4;
				break;
			case 2:
#if DEBUG
				cout << "Sound continues" << endl;
#endif
				l=(old_data[3+data_offset]&0xff)<<16;
				l|=(old_data[2+data_offset]&0xff)<<8;
				l|=(old_data[1+data_offset]&0xff);
				cout << "Chunk length appears to be " << l << endl;
				chunk_length=l+4;
				break;
			case 3:
#if DEBUG
				cout << "Silence" << endl;
#endif
				chunk_length=0;
				break;
			default:
				cout << "Unknown VOC chunk " << (*(old_data+data_offset)&0xff) << endl;
				exit(1);
			}

		if(chunk_length==0)
			break;
#if 0
		// Quick rendering to stereo
		// Halve the frequency while we're at it
		l-=(TRAILING_VOC_SLOP+LEADING_VOC_SLOP);
		uint8 *stereo_data=new uint8[l*4];
		for(size_t i=LEADING_VOC_SLOP,j=0;i<l+LEADING_VOC_SLOP;i++)
			{
			stereo_data[j++]=old_data[i];
			stereo_data[j++]=old_data[i];
			stereo_data[j++]=old_data[i];
			stereo_data[j++]=old_data[i];
			}
		l*=4;
#else
		// Resample to the current rate
		uint8 *new_data;
		size_t new_len;
		l-=(TRAILING_VOC_SLOP+LEADING_VOC_SLOP);
		resample(old_data+LEADING_VOC_SLOP,&new_data,l,&new_len,sample_rate,actual.freq);
		l=new_len;
		cerr << "Have " << l << " bytes of resampled data" << endl;

		// And convert to stereo
		uint8 *stereo_data=new uint8[l*2];
		for(size_t i=0,j=0;i<l;i++)
			{
			stereo_data[j++]=new_data[i];
			stereo_data[j++]=new_data[i];
			}
		l*=2;
		delete [] new_data;
#endif

		Chunk	c;
		c.data=stereo_data;
		c.length=l;
		chunks.push_back(c);
		data_offset+=chunk_length;
		}
	cerr << "Turn chunks to block" << endl;
	uint8 *single_buffer=chunks_to_block(chunks);
	visible_len=l;
	return single_buffer;
}

		
void	Audio::play(uint8 *sound_data,uint32 len,bool wait)
{
	string s;
	config->value("config/audio/speech/enabled",s,"yes");
	if(s=="no")
		return;
	bool	own_audio_data=false;
	if(!strncmp((const char *)sound_data,"Creative Voice File",19))
		{
		sound_data=convert_VOC(sound_data,len);
		own_audio_data=true;
		}

	if(mixer)
		mixer->play(sound_data,len);
	if(own_audio_data)
		delete [] sound_data;
}

void	Audio::cancel_raw(void)
{
	if(mixer)
		mixer->cancel_raw();
}

void	Audio::cancel_streams(void)
{
	if(mixer)
		mixer->cancel_streams();
}

void	Audio::mix(uint8 *sound_data,uint32 len)
{
	if(mixer)
		mixer->play(sound_data,len);
}

static	size_t calc_sample_buffer(uint16 _samplerate)
{
	uint32 _buffering_unit=1;
	while(_buffering_unit<_samplerate/10U)
		_buffering_unit<<=1;
	// _buffering_unit=128;
	return _buffering_unit;
}
	
Audio *Audio::self=0;

Audio::Audio() : truthful_(false),speech_enabled(true), music_enabled(true),
			effects_enabled(true), SDL_open(false),mixer(0),midi(0)
{
	self=this;
	string s;
	config->value("config/audio/speech/enabled",s,"yes");
	speech_enabled = (s!="no");
	config->value("config/audio/midi/enabled",s,"---");
	music_enabled = (s!="no");
	config->value("config/audio/effects/enabled",s,"---");
	effects_enabled = (s!="no");
}

void Audio::Init(int _samplerate,int _channels)	
{
	// Initialise the speech vectors
	uint32 _buffering_unit=calc_sample_buffer(_samplerate);
	build_speech_vector();
	if(midi)
		{
		delete midi;
		midi=0;
		}
	if(mixer)
		{
		delete mixer;
		mixer=0;
		}
         

         /* Set the audio format */
         wanted.freq = _samplerate;
         wanted.format = AUDIO_U8;
         wanted.channels = _channels;    /* 1 = mono, 2 = stereo */
         wanted.samples = _buffering_unit;  /* Good low-latency value for callback */
         wanted.callback = fill_audio;
         wanted.userdata = NULL;

	// Avoid closing SDL audio. This seems to trigger a segfault
	// if(SDL_open)
		// SDL::CloseAudio();

         /* Open the audio device, forcing the desired format */
         if ( SDL::OpenAudio(&wanted, &actual) < 0 ) {
                 fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
         }

#if 0
         cout << "We think SDL will call-back for " << actual.samples <<" bytes at a time." << endl;
#endif

	wanted=actual;
	_buffering_unit=actual.size;
#if 0
         cout << "Each buffer in the mixing ring is " << _buffering_unit <<" bytes." << endl;
#endif
	SDL_open=true;
#if DEBUG
	cout << "Audio system assembled. Audio buffer at "<<_buffering_unit<<endl;
#endif
	midi=new MyMidiPlayer();
	mixer=new Mixer(_buffering_unit,_channels,actual.silence);
#if DEBUG
	cout << "Audio initialisation OK" << endl;
#endif
}

void	Audio::playfile(const char *fname,bool wait)
{
	FILE	*fp;
	
	fp=U7open(fname,"r");
	if(!fp)
		{
		perror(fname);
		return;
		}
	fseek(fp,0L,SEEK_END);
	size_t	len=ftell(fp);
	fseek(fp,0L,SEEK_SET);
	if(len<=0)
		{
		perror("seek");
		fclose(fp);
		return;
		}
	uint8 *buf=new uint8[len];
	fread(buf,len,1,fp);
	fclose(fp);
	play(buf,len,wait);
	delete [] buf;
}


void	Audio::mixfile(const char *fname)
{
}


bool	Audio::playing(void)
{
	return false;
}


bool	Audio::start_music(int num,bool repetition, int bank)
{
	if(music_enabled && midi != 0) {
		midi->start_music(num,repetition,bank);
		return true;
	} else
		return false;
}

void	Audio::start_music(const char *fname,int num,bool repetition)
{
	if(music_enabled && midi != 0) {
		midi->start_music(fname,num,repetition);
	}
}

void Audio::start_music(XMIDI *mid_file,bool repetition)
{
	if(music_enabled && midi != 0) {
		midi->start_track(mid_file,repetition);
	}
}

bool	Audio::start_music_combat (Combat_song song, bool repetition, int bank)
{
	int num = -1;
	
	if (Game::get_game_type()!=SERPENT_ISLE) switch (song)
	{
		case CSBattle_Over:
		num = 9;
		break;
		
		case CSAttacked1:
		num = 11;
		break;
		
		case CSAttacked2:
		num = 12;
		break;
		
		case CSVictory:
		num = 15;
		break;
		
		case CSRun_Away:
		num = 16;
		break;
		
		case CSDanger:
		num = 10;
		break;
		
		case CSHidden_Danger:
		num = 18;
		break;
		
		default:
		cerr << "Error: Unable to Find combat track for song " << song << "." << endl;
		break;
	}
	else switch (song)
	{
		case CSBattle_Over:
		num = 0;
		break;
		
		case CSAttacked1:
		num = 2;
		break;
		
		case CSAttacked2:
		num = 3;
		break;
		
		case CSVictory:
		num = 6;
		break;
		
		case CSRun_Away:
		num = 7;
		break;
		
		case CSDanger:
		num = 1;
		break;
		
		case CSHidden_Danger:
		num = 9;
		break;
		
		default:
		cerr << "Error: Unable to Find combat track for song " << song << "." << endl;
		break;
	}

	return start_music (num, repetition, bank);
}

void	Audio::stop_music()
{
	if(midi)
		midi->stop_music();
}
#if 0	// Unused
static void	load_buffer(char *buffer,const char *filename,size_t start,size_t len)
{
	FILE	*fp=U7open(filename,"rb");
	if(!fp)
		{
		memset(buffer,0,len);
		return;
		}
	fseek(fp,start,SEEK_SET);
	fread(buffer,len,1,fp);
	fclose(fp);
}
#endif

bool	Audio::start_speech(int num,bool wait)
{
	if (!speech_enabled)
		return false;
	char	*buf=0;
	size_t	len;
	const char	*filename;
	
	if (Game::get_game_type() == SERPENT_ISLE)
		filename = SISPEECH;
	else
		filename = U7SPEECH;
	
	U7object	sample(filename,num);
	try
	{
		buf = sample.retrieve(len);
	}
	catch( const std::exception & err )
	{
		return false;
	}
	play((uint8*)buf,len,wait);
	delete [] buf;
	return true;
}

void	Audio::build_speech_vector(void)
{
}

void	Audio::set_external_signal(int fh)
{
	// mixer->set_auxilliary_audio(fh);
}

void	Audio::terminate_external_signal(void)
{
	// mixer->set_auxilliary_audio(-1);
}

Audio	*Audio::get_ptr(void)
{
	if(!self)
		{
		new Audio();
		self->Init(44100,2);
		}

	return self;
}

void	Audio::play_sound_effect (int num)
{
	// Where sort of sfx are we using????
	
	if(effects_enabled && midi != 0) midi->start_sound_effect(num);

}

