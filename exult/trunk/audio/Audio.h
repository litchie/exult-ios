/*
 *  Copyright (C) 2000-2013  The Exult Team
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

#ifndef AUDIO_H
#define AUDIO_H

#include <vector>
#include <SDL.h>
#include <SDL_audio.h>
#include "Midi.h"
#include "exceptions.h"
#include "AudioMixer.h"
#include "exult_constants.h"

class SFX_cached;
class SFX_cache_manager;
class Flex;
class MyMidiPlayer;
class Tile_coord;
class Game_object;

#define MAX_SOUND_FALLOFF	24
/*
 *	Music:
 */
enum Combat_song
{
	CSBattle_Over,
	CSAttacked1,
	CSAttacked2,
	CSVictory,
	CSRun_Away,
	CSDanger,
	CSHidden_Danger
};

//---- Audio -----------------------------------------------------------

class Audio 
{
private:
	UNREPLICATABLE_CLASS(Audio);
	static	Audio	*self;
	static	const int *bg2si_sfxs;	// Converts BG sfx's to SI sfx's.
	bool truthful_;
	bool speech_enabled, music_enabled, effects_enabled;
	bool allow_music_looping;
	SFX_cache_manager *sfxs;		// SFX and voice cache manager
	bool initialized;
	SDL_AudioSpec wanted;
	Pentagram::AudioMixer *mixer;

public:
	bool audio_enabled;
	Flex *sfx_file;			// Holds .wav sound effects.

private:
	// You never allocate an Audio object directly, you rather access it using get_ptr()
	Audio();
	~Audio();
	void	Init(int _samplerate,int _channels);

public:
	friend class Tired_of_compiler_warnings;
	static void		Init(void);
	static void		Destroy(void);
	static Audio*	get_ptr(void);

	// Given BG sfx, get SI if playing SI.
	static	int game_sfx(int sfx)
		{ return bg2si_sfxs ? bg2si_sfxs[sfx] : sfx; }

	void	Init_sfx(void);

	void	honest_sample_rates(void) { truthful_=true; }
	void	cancel_streams(void);	// Dump any audio streams

	void	pause_audio(void);
	void    resume_audio(void);

	void	copy_and_play(const uint8 *sound_data,uint32 len,bool);
	void	play(uint8 *sound_data,uint32 len,bool);
	void	playfile(const char *,const char *,bool);
	bool	playing(void);
	void	start_music(int num,bool continuous=false,std::string flex=MAINMUS);
	void	start_music(std::string fname,int num,bool continuous=false);
	void	start_music_combat(Combat_song song,bool continuous);
	void	stop_music();
	int		play_sound_effect (int num, int volume = AUDIO_MAX_VOLUME,
					int balance = 0, int repeat = 0, int distance=0);
	int		play_wave_sfx(int num, int volume = AUDIO_MAX_VOLUME,
					int balance = 0, int repeat = 0, int distance=0);

	static void get_2d_position_for_tile(const Tile_coord &tile, int &distance, int &balance);

	int		play_sound_effect (int num, const Game_object *obj, int volume = AUDIO_MAX_VOLUME, int repeat = 0);
	int		play_sound_effect (int num, const Tile_coord &tile, int volume = AUDIO_MAX_VOLUME, int repeat = 0);

	int		update_sound_effect(int chan, const Game_object *obj);
	int		update_sound_effect(int chan, const Tile_coord &tile);

	void	stop_sound_effect(int chan);

	void	stop_sound_effects();
	bool	start_speech(int num,bool wait=false);
	bool	is_speech_enabled() const { return speech_enabled; }
	void	set_speech_enabled(bool ena) { speech_enabled = ena; }
	bool	is_music_enabled() const { return music_enabled; }
	void	set_music_enabled(bool ena) { music_enabled = ena; }
	bool	are_effects_enabled() const { return effects_enabled; }
	void	set_effects_enabled(bool ena) { effects_enabled = ena; }
	bool	is_audio_enabled() const { return audio_enabled; }
	void	set_audio_enabled(bool ena);
	bool	is_music_looping_allowed() const { return allow_music_looping; }
	void	set_allow_music_looping(bool ena) { allow_music_looping = ena; }
	static bool	can_sfx(const std::string &file, std::string *out = 0);
	static bool have_roland_sfx(Exult_Game game, std::string *out = 0);
	static bool have_sblaster_sfx(Exult_Game game, std::string *out = 0);
	static bool have_midi_sfx(std::string *out = 0);
	static bool have_config_sfx(const std::string &game, std::string *out = 0);
	static void	channel_complete_callback(int chan);

	bool	is_track_playing(int num);

	Flex *get_sfx_file()                   
		{ return sfx_file; }
	SFX_cache_manager *get_sfx_cache() const
		{ return sfxs; }

	MyMidiPlayer *get_midi();
};

#endif
