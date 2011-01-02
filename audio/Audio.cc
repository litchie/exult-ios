/*
*  Copyright (C) 2000-2011  The Exult Team
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
#include <set>
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
#include "AudioSample.h"
#include "databuf.h"
#include "gamewin.h"
#include "actors.h"

#if !defined(ALPHA_LINUX_CXX)
#ifndef UNDER_CE
#  include <csignal>
#endif
#  include <cstdio>
#  include <cstdlib>
#  include <cstring>
#  include <iostream>
#  include <climits>
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

using namespace Pentagram;

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
*	This is a resource-management class for SFX. Maybe make it a
*	template class and use for other resources also?
*	Based on code by Sam Lantinga et al on:
*	http://www.ibm.com/developerworks/library/l-pirates2/
*/
class SFX_cache_manager
{
protected:
	typedef std::pair<int,AudioSample*> SFX_cached;
	typedef std::map<int, SFX_cached>::iterator cache_iterator;

	std::map<int, SFX_cached> cache;

	// Tries to locate a sfx in the cache based on sfx num.
	SFX_cached *find_sfx(int id)
	{
		cache_iterator found = cache.find(id);
		if (found  == cache.end()) return 0;
		return &(found->second);
	}

public:
	SFX_cache_manager()
	{ }
	~SFX_cache_manager()
	{ flush(); }

	// For SFX played through 'play_wave_sfx'. Searched cache for
	// the sfx first, then loads from the sfx file if needed.
	AudioSample *request(Flex *sfx_file, int id)
	{
		SFX_cached *loaded = find_sfx(id);
		if (!loaded) {
			SFX_cached new_sfx;
			new_sfx.first = 0;
			new_sfx.second = 0;
			loaded = &(cache[id] = new_sfx);
		}

		if (!loaded->second)
		{
			garbage_collect();

			size_t wavlen;			// Read .wav file.
			uint8 *wavbuf = (uint8*) sfx_file->retrieve(id, wavlen);
			loaded->second = AudioSample::createAudioSample(wavbuf,wavlen);
		}

		if (!loaded->second) return 0;

		// Increment counter
		++loaded->first;

		return loaded->second;
	}

	// Empties the cache.
	void flush(AudioMixer *mixer = 0)
	{
		for (cache_iterator it = cache.begin() ; it != cache.end(); it = cache.begin())
		{
			if (it->second.second) 
			{
				if (it->second.second->getRefCount() != 1 && mixer)
					mixer->stopSample(it->second.second);
				it->second.second->Release();
			}
			it->second.second = 0;
			cache.erase(it);
		}
	}

	// Remove unused sounds from the cache.
	void garbage_collect()
	{
		// Maximum 'stable' number of sounds we will cache (actual
		// count may be higher if all of the cached sounds are
		// being played).
		const int max_fixed = 6;

		std::multiset <int> sorted;

		for (cache_iterator it = cache.begin(); it != cache.end(); ++it)
		{
			if (it->second.second && it->second.second->getRefCount() == 1) 
				sorted.insert(it->second.first); 
		}

		if (sorted.empty()) return;

		int threshold = INT_MAX;
		int count = 0;

		for ( std::multiset <int>::reverse_iterator it = sorted.rbegin( ) ; it != sorted.rend( ); ++it )
		{
			if (count < max_fixed)
			{
				threshold = *it;
				count++;
			}
			else if (*it == threshold)
			{
				count++;
			}
			else
			{
				break;
			}
		}

		if (count <= max_fixed) 
			return;

		for (cache_iterator it = cache.begin(); it != cache.end(); ++it)
		{
			if (it->second.second)
			{
				if (it->second.first < threshold) 
				{
					it->second.second->Release();
					it->second.second = 0;
				}
				else if (it->second.first == threshold && count > max_fixed) 
				{
					it->second.second->Release();
					it->second.second = 0;
					count--;
				}
			}
		}

	}
};

//---- Audio ---------------------------------------------------------
void Audio::Init(void)
{
	// Crate the Audio singleton object
	if (!self)
	{
		int sample_rate = 22050;
		bool stereo = true;

#ifdef UNDER_CE
		sample_rate = 11025;
		stereo = false;
#endif

		config->value("config/audio/sample_rate", sample_rate, sample_rate);
		config->value("config/audio/stereo", stereo, stereo);

		self = new Audio();
		self->Init(sample_rate,stereo?2:1);
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
effects_enabled(true), initialized(false), mixer(0), sfx_file(0)
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

	mixer = new AudioMixer(_samplerate,_channels==2,MIXER_CHANNELS);

	COUT("Audio initialisation OK");

	mixer->openMidiOutput();
	initialized = true;
}

bool	Audio::can_sfx(const std::string &file, std::string *out)
{
	if (file == "")
		return false;
	string d = file;
	// Full path?
	if (U7exists(d.c_str()))
		{
		if (out)
			*out = d;
		return true;
		}

#ifdef MACOSX
	// Check in the app bundle:
	d = "<BUNDLE>/" + file;
	if (is_system_path_defined("<BUNDLE>") && U7exists(d.c_str()))
		{
		if (out)
			*out = d;
		return true;
		}
#endif

	// Also just check in the actual data dir
	d = "<DATA>/" + file;
	if (U7exists(d.c_str()))
		{
		if (out)
			*out = d;
		return true;
		}

	return false;
}

bool Audio::have_roland_sfx(Exult_Game game, std::string *out)
	{
	if (game == BLACK_GATE)
		return can_sfx(SFX_ROLAND_BG, out);
	else if (game == SERPENT_ISLE)
		return can_sfx(SFX_ROLAND_SI, out);
	return false;
	}
	
bool Audio::have_sblaster_sfx(Exult_Game game, std::string *out)
	{
	if (game == BLACK_GATE)
		return can_sfx(SFX_BLASTER_BG, out);
	else if (game == SERPENT_ISLE)
		return can_sfx(SFX_BLASTER_SI, out);
	return false;
	}
	
bool Audio::have_midi_sfx(std::string *out)
	{
#ifdef ENABLE_MIDISFX
	return can_sfx(SFX_MIDIFILE, out);
#else
	return false;
#endif
	}
	
bool Audio::have_config_sfx(const std::string &game, std::string *out)
	{
	string s;
	string d = "config/disk/game/" + game + "/waves";
	config->value(d.c_str(), s, "---");
	return (s != "---") && can_sfx(s, out);
	}
	
void	Audio::Init_sfx()
{
	FORGET_OBJECT(sfx_file);

	if (sfxs)
		{
		sfxs->flush(mixer);
		sfxs->garbage_collect();
		}

	Exult_Game game = Game::get_game_type();
	if (game == SERPENT_ISLE)
		bg2si_sfxs = bgconv;
	else
		bg2si_sfxs = 0;
	// Collection of .wav's?
	string flex;
#ifdef ENABLE_MIDISFX
	string v;
	config->value("config/audio/effects/midi", v, "no");
	if (have_midi_sfx(&flex) && v != "no")
		{
		cout << "Opening midi SFX's file: \"" << flex << "\"" << endl;
		sfx_file = new FlexFile(flex.c_str());
		return;
		}
#endif
	if (!have_config_sfx(Game::get_gametitle(), &flex))
		{
		if (have_roland_sfx(game, &flex) || have_sblaster_sfx(game, &flex))
			{
			string d = "config/disk/game/" + Game::get_gametitle() + "/waves";
			size_t sep = flex.rfind('/');
			std::string pflex;
			if (sep != string::npos)
				{
				sep++;
				pflex = flex.substr(sep);
				}
			else
				pflex = flex;
			config->set(d.c_str(), pflex, true);
			}
		else
			{
			cerr << "Digital SFX's file specified: " << flex
				<< "... but file not found, and fallbacks are missing" << endl;
			return;
			}
		}
	cout << "Opening digital SFX's file: \"" << flex << "\"" << endl;
	sfx_file = new FlexFile(flex.c_str());
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

void	Audio::copy_and_play(const uint8 *sound_data,uint32 len, bool wait)
{
	uint8 *new_sound_data = new uint8[len];
	std::memcpy(new_sound_data,sound_data,len);
	play(new_sound_data ,len,wait);
}

void	Audio::play(uint8 *sound_data,uint32 len, bool wait)
{
	if (!audio_enabled || !speech_enabled || !len) return;

	AudioSample *audio_sample = AudioSample::createAudioSample(sound_data,len);

	if (audio_sample)
	{
		mixer->playSample(audio_sample,0,128);
		audio_sample->Release();
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
int	Audio::play_sound_effect (int num, int volume, int balance, int repeat, int distance)
{
	if (!audio_enabled || !effects_enabled) return -1;

	// Where sort of sfx are we using????
	if (sfx_file != 0)		// Digital .wav's?
		return play_wave_sfx(num, volume, balance, repeat, distance);
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
 int volume,		// 0-256.
 int balance,		// balance, -256 (left) - +256 (right)
 int repeat,		// Keep playing.
 int distance
 )
{
	if (!effects_enabled || !sfx_file || !mixer) 
		return -1;  // no .wav sfx available

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
	AudioSample *wave = sfxs->request(sfx_file, num);
	if (!wave)
	{
		cerr << "Couldn't play sfx '" << num << "'" << endl;
		return -1;
	}

	int instance_id = mixer->playSample(wave,repeat,0,true,AUDIO_DEF_PITCH,volume,volume);
	if (instance_id < 0)
	{
		CERR("No channel was available to play sfx '" << num << "'");
		return -1;
	}

	CERR("Playing SFX: " << num);
	
	mixer->set2DPosition(instance_id,distance,balance);
	mixer->setPaused(instance_id,false);

	return instance_id;
}

/*
static int slow_sqrt(int i)
{
	for (int r = i/2; r != 0; r--)
	{
		if (r*r <= i) return r;
	}

	return 0;
}
*/

void Audio::get_2d_position_for_tile(const Tile_coord &tile, int &distance, int &balance)
{
	distance = 0;
	balance = 0;

	Game_window *gwin = Game_window::get_instance();
	Rectangle size = gwin->get_win_tile_rect();
	Tile_coord apos(size.x+size.w/2,size.y+size.h/2,gwin->get_camera_actor()->get_lift());

	int sqr_dist = apos.square_distance_screen_space(tile);
	if (sqr_dist > MAX_SOUND_FALLOFF*MAX_SOUND_FALLOFF) {
		distance = 257;
		return;
	}

	//distance = sqrt((double) sqr_dist) * 256 / MAX_SOUND_FALLOFF;
	//distance = slow_sqrt(sqr_dist) * 256 / MAX_SOUND_FALLOFF;
	distance = sqr_dist * 256 / (MAX_SOUND_FALLOFF*MAX_SOUND_FALLOFF);

	balance = (Tile_coord::delta(apos.tx,tile.tx)*2-tile.tz-apos.tz)*32 / 5;

}

int Audio::play_sound_effect (int num, const Game_object *obj, int volume, int repeat)
{
	Tile_coord tile = obj->get_center_tile();
	return play_sound_effect(num,tile, volume,repeat);
}

int Audio::play_sound_effect (int num, const Tile_coord &tile, int volume, int repeat)
{
	int distance;
	int balance;
	get_2d_position_for_tile(tile,distance,balance);
	if (distance > 256) distance = 256;
	return play_sound_effect(num,volume,balance,repeat,distance);
}

int Audio::update_sound_effect(int chan, const Game_object *obj)
{
	Tile_coord tile = obj->get_center_tile();
	return update_sound_effect(chan,tile);
}

int Audio::update_sound_effect(int chan, const Tile_coord &tile)
{
	if (!mixer) return -1;

	int distance;
	int balance;
	get_2d_position_for_tile(tile,distance,balance);
	if (distance > 256) {
		mixer->stopSample(chan);
		return -1;
	}
	else {
		mixer->set2DPosition(chan,distance,balance);
		return chan;
	}
}

void Audio::stop_sound_effect(int chan)
{
	if (!mixer) return;
	mixer->stopSample(chan);
}

/*
*	Halt sound effects.
*/

void Audio::stop_sound_effects()
{
	if (sfxs) sfxs->flush(mixer);

#ifdef ENABLE_MIDISFX
	if (mixer && mixer->getMidiPlayer())
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

		int sample_rate = 22050;
		bool stereo = true;

#ifdef UNDER_CE
		sample_rate = 11025;
		stereo = false;
#endif

		config->value("config/audio/sample_rate", sample_rate, sample_rate);
		config->value("config/audio/stereo", stereo, stereo);

		Init(sample_rate,stereo?2:1);
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
