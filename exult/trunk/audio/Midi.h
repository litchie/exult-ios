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
#ifndef WIN32
//#include "Mixer.h"
#endif
#include "xmidi.h"

#include "SDL_mixer.h"

#include <string>

//---- MidiAbstract -----------------------------------------------------------

class	MidiAbstract
{
public:
	virtual void start_track(XMIDIEventList *, bool repeat) = 0;
	virtual void start_sfx(XMIDIEventList *) { }
	
	virtual void	stop_track(void)=0;
	virtual void	stop_sfx(void) { };
	virtual	bool	is_playing(void)=0;
	virtual	const	char *copyright(void)=0;

	virtual void load_patches(bool force_xmidi) { }
	virtual bool is_fm_synth() { return false; }
	virtual bool use_gs127() { return false; }

	MidiAbstract() {};
	virtual	~MidiAbstract() {};
};

//Pseudo device, used purely to create the OGG_MIDI class with something
class	OGG_MIDI : virtual public MidiAbstract
{
public:
	virtual void	start_track(XMIDIEventList *, bool repeat);
	virtual void	start_sfx(XMIDIEventList *);
	virtual void	stop_track(void);
	virtual void	stop_sfx(void);
	virtual	bool	is_playing(void);
	virtual const	char *copyright(void);
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
	void	start_track(XMIDIEventList *eventlist, bool continuous=false);
	void	start_sound_effect(int num);
	void	stop_sound_effects();
	void	load_patches(bool force_xmidi=false) { if (midi_device) midi_device->load_patches(force_xmidi); }

	void	stop_music();
	
	bool	add_midi_bank(const char *s);
	void	set_music_conversion(int conv);
	int	get_music_conversion() { return music_conversion; }
	void	set_effects_conversion(int conv);
	int	get_effects_conversion() { return effects_conversion; }
	
	inline bool	is_track_playing(int num) { 
		return midi_device && current_track==num && midi_device->is_playing();
	}
	inline int	get_current_track() { return (midi_device!=0)&&midi_device->is_playing()?current_track:-1; }
	inline int	is_repeating() { return repeating; }
	int		music_conversion;		//was in private SQ

	bool is_fm_synth() { return midi_device?midi_device->is_fm_synth():false; }

	
private:
	static void music_complete_callback(void);

	MyMidiPlayer(const MyMidiPlayer &m) ; // Cannot call
	MyMidiPlayer &operator=(const MyMidiPlayer &); // Cannot call
	void    kmidi_start_track(int num,bool continuous=false);
	std::vector<std::string>	midi_bank;
	int	current_track;
	bool	repeating;
	MidiAbstract	*midi_device;
	bool	initialized;

	bool	init_device(void);

	int		effects_conversion;
};

#endif
