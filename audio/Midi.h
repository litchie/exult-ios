//-*-Mode: C++;-*-
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

#ifndef _MIDI_H_
#define _MIDI_H_

#if (_GNUG__ >= 2) && (!defined WIN32)
#  pragma interface
#endif

#include <vector>
#include "Mixer.h"
#include "xmidi.h"

#include <string>

//---- MidiAbstract -----------------------------------------------------------

class	MidiAbstract
{
public:
	// Do we accept events, default no
	virtual bool	accepts_events(void) { return false; }

	// Non Event based methods
	virtual void	start_track(const char *,bool repeat) { };
	virtual void	start_sfx(const char *) { };
	// Event based methods
	virtual void	start_track(midi_event *evntlist, int ppqn, bool repeat) { };
	virtual void	start_sfx(midi_event *evntlist, int ppqn) { };
	
	virtual void	stop_track(void)=0;
	virtual void	stop_sfx(void) { };
	virtual	bool	is_playing(void)=0;
	virtual	const	char *copyright(void)=0;

	MidiAbstract() {};
	virtual	~MidiAbstract() {};
};


//---- MyMidiPlayer -----------------------------------------------------------

class	MyMidiPlayer
{
public:
	MyMidiPlayer();
	~MyMidiPlayer();
	MyMidiPlayer(const char *flexfile);
	void	start_music(int num,bool continuous=false,int bank=0);
	void	start_music(const char *fname,int num,bool continuous=false);
	void	start_track(int num,bool continuous=false,int bank=0);
	void	start_track(const char *fname,int num,bool continuous=false);
	void	start_track(XMIDI *midfile, bool continuous=false);
	void	start_sound_effect(int num);

	void	stop_music();
	
	bool	add_midi_bank(const char *s);
	void	set_music_conversion(int conv);
	int	get_music_conversion() { return music_conversion; }
	void	set_effects_conversion(int conv);
	int	get_effects_conversion() { return effects_conversion; }

private:
	MyMidiPlayer(const MyMidiPlayer &m) ; // Cannot call
	MyMidiPlayer &operator=(const MyMidiPlayer &); // Cannot call
	void    kmidi_start_track(int num,bool continuous=false);
	std::vector<std::string>	midi_bank;
	int	current_track;
	MidiAbstract	*midi_device;

	bool	init_device(void);

	int		music_conversion;
	int		effects_conversion;
};

#endif
