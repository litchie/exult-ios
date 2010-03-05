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

#ifndef _MIDI_H_
#define _MIDI_H_

#include <vector>
#include <string>

#include "SDL_mixer.h"

#include "common_types.h"

#include "fnames.h"

class MidiDriver;

//---- MyMidiPlayer -----------------------------------------------------------

class	MyMidiPlayer
{
public:

	enum TimberLibrary {
		TIMBRE_LIB_GM		= 0,	// General Midi/GS output mode
		TIMBRE_LIB_INTRO	= 1,	// Intro
		TIMBRE_LIB_MAINMENU	= 2,	// Main Menu
		TIMBRE_LIB_GAME		= 3,	// In Game
		TIMBRE_LIB_ENDGAME	= 4		// Endgame
	};


	MyMidiPlayer();
	~MyMidiPlayer();

	void			start_music(int num,bool continuous=false,std::string flex=MAINMUS);
	void			start_music(std::string fname,int num,bool continuous=false);
	void			stop_music();

	bool			is_track_playing(int num);
	int				get_current_track();
	int				is_repeating() { return repeating; }
	
	void			set_timbre_lib(TimberLibrary lib);
	TimberLibrary	get_timbre_lib() { return timbre_lib; }
	
	void			set_midi_driver(std::string desired_driver, bool use_oggs);
	std::string		get_midi_driver() { return midi_driver_name; }
	bool			get_ogg_enabled() { return ogg_enabled; }

	void			set_music_conversion(int conv);
	int				get_music_conversion() { return music_conversion; }

#ifdef ENABLE_MIDISFX
	void			start_sound_effect(int num);
	void			stop_sound_effects();

	void			set_effects_conversion(int conv);
	int				get_effects_conversion() { return effects_conversion; }
#endif

private:

	MyMidiPlayer(const MyMidiPlayer &m) ; // Cannot call
	MyMidiPlayer &operator=(const MyMidiPlayer &); // Cannot call

	bool			repeating;
	int				current_track;

	std::string		midi_driver_name;
	MidiDriver *	midi_driver;
	bool			initialized;
	bool			init_device(void);
	static void		sdl_music_hook(void *udata, uint8 *stream, int len);


	TimberLibrary	timbre_lib;
	std::string		timbre_lib_filename;
	int				timbre_lib_index;
	int				timbre_lib_game;
	int				music_conversion;
	int				effects_conversion;
	void			load_timbres();
	int				setup_timbre_for_track(std::string &str);
	
	// Ogg Stuff
	bool			ogg_enabled;
	Mix_Music		*oggmusic;

	bool			ogg_play_track(std::string filename, int num, bool repeat);
	bool			ogg_is_playing();
	void			ogg_stop_track();
};

#endif
