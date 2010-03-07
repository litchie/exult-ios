/*
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "pent_include.h"

#include <SDL_audio.h>
#include <SDL_timer.h>

#include <fstream>
//#include "SDL_mapping.h"

#include "Audio.h"
#include "Configuration.h"
#include "Flex.h"
#include "conv.h"
#include "exult.h"
#include "fnames.h"
#include "game.h"
#include "utils.h"

#include "AudioMixer.h"
#include "VocAudioSample.h"
#include "databuf.h"

#if !defined(ALPHA_LINUX_CXX)
#ifndef UNDER_CE
#  include <csignal>
#endif
#  include <cstdio>
#  include <cstdlib>
#  include <cstring>
#  include <iostream>
#endif
#ifndef UNDER_CE
#include <fcntl.h>
#include <unistd.h>
#endif

#if defined(MACOS)
#  include <stat.h>
#elif !defined(UNDER_CE)
#  include <sys/stat.h>
#  include <sys/types.h>
#endif

//#include <crtdbg.h>


#ifndef UNDER_EMBEDDED_CE
using std::cerr;
using std::cout;
using std::endl;
using std::exit;
using std::memcpy;
using std::memset;
using std::string;
using std::strncmp;
using std::vector;
using std::ifstream;
using std::ios;
#endif

// These MIGHT be macros!
#ifndef min
using std::min;
#endif
#ifndef max
using std::max;
#endif

#define	TRAILING_VOC_SLOP 32
#define	LEADING_VOC_SLOP 32
#define MIXER_CHANNELS 32

struct	Chunk
{
	size_t	length;
	uint8	*data;
	Chunk(size_t l, uint8 *d) : length(l),data(d) {}
};


Audio *Audio::self = 0;
int const *Audio::bg2si_sfxs = 0;

//----- Utilities ----------------------------------------------------

//----- SFX ----------------------------------------------------------

/*
 *	For caching sound effects:
 */
class SFX_cached
	{
	int num;			// Sound-effects #.
	Pentagram::AudioSample *data;	// The data.
	SFX_cached *next;	// Next in chain.
	int ref_cnt;		// Reference count of the sfx/voice.
public:
	friend class Audio;
	friend class SFX_cache_manager;
	SFX_cached(int sn, uint8 *b, uint32 l, SFX_cached *oldhead)
		: num(sn), next(oldhead), ref_cnt(1)
		{
			/*
			if (num >= 0 || !strncmp((const char *)b, "OggS", 4))
				{
				SDL_RWops *rwsrc = SDL_RWFromMem(b, l);
				data = Mix_LoadWAV_RW(rwsrc, 1);
				}
			else
				data = Mix_QuickLoad_RAW(b, l);
				*/
		}
	~SFX_cached()
		{
		Uint8 *chunkbuf=NULL;
		//if(data->allocated == 0)
		//	chunkbuf = data->abuf;
		//Mix_FreeChunk(data);

		//Must be freed after the Mix_FreeChunk
		if(chunkbuf)
			delete[] chunkbuf;
		}
	void add_ref(void)
		{ ++ref_cnt; }
	void release(void)
		{
		// Free when refcount is 0
		if (!--ref_cnt)
			delete this;
		}
	int get_num_refs(void)
		{ return(ref_cnt); }
	};

/*
 *	This is a resource-management class for SFX. Maybe make it a
 *	template class and use for other resources also?
 *	Based on code by Sam Lantinga et al on:
 *	http://www.ibm.com/developerworks/library/l-pirates2/
 */
class SFX_cache_manager
	{
protected:
	SFX_cached *cache;

	// Tries to locate a sfx in the cache based on sfx num.
	SFX_cached *find_sfx(int id)
		{
		SFX_cached *prev = 0;
		for (SFX_cached *each = cache; each; each = each->next)
			{
			if (each->num == id)
				{
				// Move to head of chain.
				if (prev)
					{
					prev->next = each->next;
					each->next = cache;
					cache = each;
					}
				return each;
				}
			prev = each;
			}
		return 0;
		}
	
	// Loads a wave SFX from its digital file and num.
	SFX_cached *load_sfx(Flex *sfx_file, int id)
		{
		size_t wavlen;			// Read .wav file.
		unsigned char *wavbuf =
				(unsigned char *) sfx_file->retrieve(id, wavlen);
		if (wavlen)
			cache = new SFX_cached(id, wavbuf, wavlen, cache);
		delete[] wavbuf;

		// Perform garbage collection here.
		garbage_collect();
		return wavlen ? cache : (SFX_cached *)0;
		}
	
	// Unloads a given SFX, based on data pointer.
	void unload(Pentagram::AudioSample *data)
		{
			/*
		SFX_cached *prev = 0, *each;
		for (each = cache; each; each = each->next)
			{
			if (data == each->data)
				{
				// Free the object if not in use
				if (each->ref_cnt != 1)
					{
					CERR("Unloading cached SFX " << each->num
						<< " while it was still in use! This better be happening because Exult is closing!");
					while (each->ref_cnt > 1)
						each->release();
					}

				// Unlink
				if (prev)
					prev->next = each->next;
				else
					cache = each->next;

				each->release();
				break;
				}
			prev = each;
			}
			*/
		}
public:
	SFX_cache_manager()
		{ cache = 0; }
	~SFX_cache_manager()
		{ flush(); }
	
	// For sounds played through 'play', which include voice and some SFX (?)
	// in the intro sequences.
	Pentagram::AudioSample *add_from_data(unsigned char *wavbuf, size_t wavlen)
		{
			/*
		cache = new SFX_cached(-1, wavbuf, wavlen, cache);
		cache->add_ref();

		// Perform garbage collection here.
		garbage_collect();
		return cache->data;
		*/
			return 0;
		}

	// For SFX played through 'play_wave_sfx'. Searched cache for
	// the sfx first, then loads from the sfx file if needed.
	Pentagram::AudioSample *request(Flex *sfx_file, int id)
		{
			/*
		SFX_cached *sfx = 0;
		if (id > -1 && sfx_file)
			{
			sfx = find_sfx(id);
			if (!sfx)
				sfx = load_sfx(sfx_file, id);
			if (!sfx)
				return (Mix_Chunk *)0;
			sfx->add_ref();
			return sfx->data;
			}
		return (Mix_Chunk *)0;
		*/
			return 0;
		}
	
	// Reduces the ref-count of the data, to a minimum of 1.
	// Does *not* remove from cache, ever.
	void release(Pentagram::AudioSample *data)
		{
			/*
		if (!data)
			return;
		SFX_cached *prev = 0, *each;
		for (each = cache; each; each = each->next)
			{
			if (data == each->data)
				{
				// Free the object if not in use
				if (each->ref_cnt == 1)
					CERR("Tried to release cached but unused SFX");
				else
					each->release();
				break;
				}
			prev = each;
			}
			*/
		}
	
	// Empties the cache.
	void flush(void)
		{
		while (cache)
			unload(cache->data);
		cache = 0;
		}

	// Remove unused sounds from the cache.
	void garbage_collect()
		{
		// Maximum 'stable' number of sounds we will cache (actual
		// count may be higher if all of the cached sounds are
		// being played).
		const int max_fixed = 6;
		SFX_cached *each = cache;
		SFX_cached *prev = 0;
		int cnt = 0;
		while (each)
			{
			if (each->ref_cnt == 1 &&
				(each->num < 0 || cnt >= max_fixed))
				{	// Remove from cache and unlink.
				prev->next = each->next;
				each->release();
				}
			else
				prev = each;
			cnt++;
			// Causes occasional segfault:
			//each = each->next;
			each = prev->next;
			}
		}
	};

//---- Audio ---------------------------------------------------------
void Audio::Init(void)
{
	// Crate the Audio singleton object
	if (!self)
	{
		self = new Audio();
#ifdef UNDER_CE
		self->Init(SAMPLERATE/2,1);
#else
		self->Init(SAMPLERATE,2);
#endif
	}
}

void Audio::Destroy(void)
{
	delete self;
	self = 0;
}

Audio	*Audio::get_ptr(void)
{
	// The following assert here might be too harsh, maybe we should leave
	// it to the caller to handle non-inited audio-system?
	assert(self != NULL);

	return self;
}


Audio::Audio() :
	truthful_(false),speech_enabled(true), music_enabled(true),
	effects_enabled(true), mixer(0),
	initialized(false), sfx_file(0)
{
	assert(self == NULL);

	string s;

	config->value("config/audio/enabled",s,"yes");
	audio_enabled = (s!="no");
	config->set("config/audio/enabled", audio_enabled?"yes":"no",true);

	config->value("config/audio/speech/enabled",s,"yes");
	speech_enabled = (s!="no");
	config->value("config/audio/midi/enabled",s,"---");
	music_enabled = (s!="no");
	config->value("config/audio/effects/enabled",s,"---");
	effects_enabled = (s!="no");
	config->value("config/audio/midi/looping",s,"yes");
	allow_music_looping = (s!="no");

	mixer = 0;
	sfxs = new SFX_cache_manager();
}

void Audio::Init(int _samplerate,int _channels)	
{
	if (!audio_enabled) return;

	FORGET_OBJECT(mixer);

	mixer = new Pentagram::AudioMixer(_samplerate,_channels==2,MIXER_CHANNELS);

	COUT("Audio initialisation OK");

	mixer->openMidiOutput();
	initialized = true;
}

void Audio::channel_complete_callback(int chan)
{
	/*
	Mix_Chunk *done_chunk = Mix_GetChunk(chan);
	SFX_cache_manager *sfxs = Audio::get_ptr()->get_sfx_cache();
	sfxs->release(done_chunk);
	*/
	/*
	Mix_Chunk *done_chunk = Mix_GetChunk(chan);
	Uint8 *chunkbuf=NULL;

	//We need to free these chunks as they were allocated by us and not SDL_Mixer
	//This happens when Mix_QuickLoadRAW is used.
	if(done_chunk->allocated == 0)
		chunkbuf = done_chunk->abuf;
		
	Mix_FreeChunk(done_chunk);

	//Must be freed after the Mix_FreeChunk
	if(chunkbuf)
		delete[] chunkbuf;
	*/
}

bool	Audio::can_sfx(const std::string &game) const
{
	string s;
	string d = "config/disk/game/" + game + "/waves";
	config->value(d.c_str(), s, "---");
	if (s != "---" && U7exists(s.c_str()))
		return true;

	// Also just check in the actual data dir
	d = "<DATA>/" + s;
	if (U7exists(d.c_str()))
		return true;

#ifdef ENABLE_MIDISFX
	if (U7exists("<DATA>/midisfx.flx"))
		return true;
#endif
	
	return false;
}

void	Audio::Init_sfx()
{
	delete sfx_file;

	if (Game::get_game_type() == SERPENT_ISLE)
		bg2si_sfxs = bgconv;
	else
		bg2si_sfxs = 0;
					// Collection of .wav's?
	string s;
	string d = "config/disk/game/" + Game::get_gametitle() + "/waves";
	config->value(d.c_str(), s, "---");
	if (s != "---")
	{
		if (!U7exists(s.c_str()))
		{
			d = "<DATA>/" + s;
			if (!U7exists(d.c_str()))
			{
				cerr << "Digital SFX's file specified: " << s << "... but file not found" << endl;
				return;
			}
		}
		else
			d = s;

		COUT("Opening digital SFX's file: \"" << s << "\"");
		sfx_file = new FlexFile(d.c_str());
	}
}

Audio::~Audio()
{ 
	if (!initialized)
	{
		self = 0;
		//SDL_open = false;
		return;
	}

	CERR("~Audio:  about to stop_music()");
	stop_music();

	FORGET_OBJECT(mixer);

	delete sfxs;
	delete sfx_file;

	CERR("~Audio:  about to quit subsystem");
	self = 0;
}


void	Audio::play(uint8 *sound_data,uint32 len, bool wait)
	{
	if (!audio_enabled || !speech_enabled || !len) return;

	IBufferDataSource bds(sound_data,len);
	if (Pentagram::VocAudioSample::isVoc(&bds))
	{
		Pentagram::VocAudioSample *audio_sample = new Pentagram::VocAudioSample(sound_data,len);

		mixer->playSample(audio_sample,0,128);
		audio_sample->Release();
	}
	else
	{
		delete [] sound_data;
	}


	}

void	Audio::cancel_streams(void)
{
	if (!audio_enabled) return;

	//Mix_HaltChannel(-1);
	mixer->reset();

}

void	Audio::pause_audio(void)
{
	if (!audio_enabled) return;

	mixer->setPausedAll(true);
}

void 	Audio::resume_audio(void)
{
	if (!audio_enabled) return;

	mixer->setPausedAll(false);
}


void Audio::playfile(const char *fname, const char *fpatch, bool wait)
{
	if (!audio_enabled)
		return;

	U7multiobject sample(fname, fpatch, 1);

	size_t len;
	uint8 *buf = (uint8 *)sample.retrieve(len);
	if (!buf || len <= 0)
		{
		// Failed to find file in patch or static dirs.
		CERR("Audio::playfile: Error reading file '" << fname << "'");
		delete [] buf;
		return;
		}

	play(buf, len, wait);
}


bool	Audio::playing(void)
{
	return false;
}


void	Audio::start_music(int num, bool continuous,std::string flex)
{
	if(audio_enabled && music_enabled && mixer && mixer->getMidiPlayer())
		mixer->getMidiPlayer()->start_music(num,continuous && allow_music_looping,flex);
}

void	Audio::start_music(std::string fname, int num, bool continuous)
{
	if(audio_enabled && music_enabled && mixer && mixer->getMidiPlayer())
		mixer->getMidiPlayer()->start_music(fname,num,continuous && allow_music_looping);
}

void	Audio::start_music_combat (Combat_song song, bool continuous)
{
	if(!audio_enabled || !music_enabled || !mixer || !mixer->getMidiPlayer())
		return;

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
		CERR("Error: Unable to Find combat track for song " << song << ".");
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
		CERR("Error: Unable to Find combat track for song " << song << ".");
		break;
	}
	
	mixer->getMidiPlayer()->start_music(num,continuous && allow_music_looping);
}

void	Audio::stop_music()
{
	if (!audio_enabled) return;

	if(mixer && mixer->getMidiPlayer())
		mixer->getMidiPlayer()->stop_music();
}

bool Audio::start_speech(int num, bool wait)
{
	if (!audio_enabled || !speech_enabled)
		return false;

	const char *filename;
	const char *patchfile;

	if (Game::get_game_type() == SERPENT_ISLE)
	{
		filename = SISPEECH;
		patchfile = PATCH_SISPEECH;
	}
	else
	{
		filename = U7SPEECH;
		patchfile = PATCH_U7SPEECH;
	}
	
	U7multiobject sample(filename, patchfile, num);

	size_t len;
	uint8 *buf = (uint8 *)sample.retrieve(len);
	if (!buf || len <= 0)
	{
		delete [] buf;
		return false;
	}

	play(buf,len,wait);
	return true;
}

/*
 *	This returns a 'unique' ID, but only for .wav SFX's (for now).
 */
int	Audio::play_sound_effect (int num, int volume, int dir, int repeat)
{
	if (!audio_enabled || !effects_enabled) return -1;

	// Where sort of sfx are we using????
	if (sfx_file != 0)		// Digital .wav's?
		return play_wave_sfx(num, volume, dir, repeat);
#ifdef ENABLE_MIDISFX
	else if (mixer && mixer->getMidiPlayer()) 
		mixer->getMidiPlayer()->start_sound_effect(num);
#endif
	return -1;
}

/*
 *	Play a .wav format sound effect, 
 *  return the channel number playing on or -1 if not playing, (0 is a valid channel in SDL_Mixer!)
 */
int Audio::play_wave_sfx
	(
	int num,
	int volume,			// 0-128.
	int dir,			// 0-15, from North, clockwise.
	int repeat			// Keep playing.
	)
{
	if (!effects_enabled || !sfx_file /*|| !mixer*/) 
		return -1;  // no .wav sfx available
/*
#if 0
	if (Game::get_game_type() == BLACK_GATE)
		num = bgconv[num];
	CERR("; after bgconv:  " << num);
#endif
	if (num < 0 || (unsigned)num >= sfx_file->number_of_objects())
	{
		cerr << "SFX " << num << " is out of range" << endl;
		return -1;
	}
	wave = sfxs->request(sfx_file, num);
	if (!wave)
	{
		cerr << "Couldn't play sfx '" << num << "'" << endl;
		return -1;
	}

	int sfxchannel;
	sfxchannel = Mix_PlayChannel(-1, wave, repeat);
	if (sfxchannel < 0)
		{
		sfxs->release(wave);
		CERR("No channel was available to play sfx '" << num << "'");
		return -1;
		}

	CERR("Playing SFX: " << num);
	Mix_Volume(sfxchannel, volume);
	Mix_SetPosition(sfxchannel, (dir * 22), 0);
	return sfxchannel;
	*/
	return -1;
}

/*
 *	Halt sound effects.
 */

void Audio::stop_sound_effects()
{
	if (sfx_file != 0)		// .Wav's?
	{
	}
		
#ifdef ENABLE_MIDISFX
	else if (mixer && mixer->getMidiPlayer())
		mixer->getMidiPlayer()->stop_sound_effects();
#endif
	}


void Audio::set_audio_enabled(bool ena)
{
	if (ena && audio_enabled && initialized)
	{

	}
	else if (!ena && audio_enabled && initialized)
	{
		stop_sound_effects();
		stop_music();
		audio_enabled = false;
	}
	else if (ena && !audio_enabled && initialized)
	{
		audio_enabled = true;
	}
	else if (!ena && !audio_enabled && initialized)
	{

	}
	else if (ena && !audio_enabled && !initialized)
	{
		audio_enabled = true;

		Init(SAMPLERATE,2);
	}
	else if (!ena && !audio_enabled && !initialized)
	{

	}
}

bool Audio::is_track_playing(int num)
{
	MyMidiPlayer *midi = mixer?mixer->getMidiPlayer():0;
	return midi && midi->is_track_playing(num);
}

MyMidiPlayer *Audio::get_midi()
{
	return mixer?mixer->getMidiPlayer():0;
}
