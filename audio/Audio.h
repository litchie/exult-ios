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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef AUDIO_H
#define AUDIO_H

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma interface
#endif

#include <vector>
#include "SDL_mapping.h"
#include <SDL.h>
#include <SDL_audio.h>
#include "Mixer.h"
#include "Midi.h"
#include "exceptions.h"

class SFX_cached;
class Flex;

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
	static	int *bg2si_sfxs;	// Converts BG sfx's to SI sfx's.
	bool truthful_;
	bool audio_enabled, speech_enabled, music_enabled, effects_enabled;
	bool allow_music_looping;
	bool SDL_open;
	SFX_cached *sfxs;		// ->list of cached .wav snd. effects.
	Flex *sfx_file;			// Holds .wav sound effects.
	Mixer	*mixer;
	MyMidiPlayer	*midi;
	bool initialized;
	SDL_AudioSpec wanted;

public:
	SDL_AudioSpec actual;

private:
	// You never allocate an Audio object directly, you rather access it using get_ptr()
	Audio();
	~Audio();
	void	Init(int _samplerate,int _channels);

	uint8 *	convert_VOC(uint8 *,uint32 &);
	void	build_speech_vector(void);

	static void	fill_audio(void *udata, uint8 *stream, int len);

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
	void	play(uint8 *sound_data,uint32 len,bool);
	void	playfile(const char *,bool);
	void	playwave(const char *,bool);
	void	mix_audio(void);
	void	mix(uint8 *sound_data,uint32 len);
	void	mixfile(const char *fname);
	bool	playing(void);
	void	clear(uint8 *,int);
	void	start_music(int num,bool continuous,int bank=0);
	void	start_music(const char *fname,int num,bool continuous);
	void	start_music(XMIDIEventList *midfile,bool continuous);
	void	start_music_combat(Combat_song song,bool continuous,int bank=0);
	void	stop_music();
	AudioID	play_sound_effect (int num, int volume = SDL_MIX_MAXVOLUME,
					int dir = 0, bool repeat = false);
	AudioID	play_wave_sfx(int num, int volume = SDL_MIX_MAXVOLUME,
					int dir = 0, bool repeat = false);
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
	bool	can_sfx(const std::string &game) const;

	ProducerConsumerBuf	*Create_Audio_Stream(uint32 id) {
		return !mixer?0:mixer->Create_Audio_Stream(id);
	}
	void    Destroy_Audio_Stream(uint32 id) {
		if(mixer) mixer->Destroy_Audio_Stream(id);
	}
	bool	is_playing(uint32 id) {
		return !mixer?0:mixer->is_playing(id);
	}
	bool	is_track_playing(int num) { 
		return midi && midi->is_track_playing(num);
	}

//	static	const	unsigned int	ringsize=3000;
//	static	const	int	samplerate=11025;
//	static	const	int	persecond=2;
//	static	const	int	buffering_unit=1024;

	MyMidiPlayer *get_midi() {return midi;}
	Flex *get_sfx_file()
		{ return sfx_file; }
};

#endif
