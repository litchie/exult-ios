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

#ifndef _Midi_h_
#define _Midi_h_

#if __GNUG__ >= 2
#  pragma interface
#endif

#if !AUTOCONFIGURED
#include "../autoconfig.h"
#endif

#include <vector>
#include "SDL_mapping.h"
#include <SDL_audio.h>
#include "Flex.h"
#include "Table.h"
#include "Mixer.h"


//---- MidiAbstract -----------------------------------------------------------

class	MidiAbstract
{
public:
	virtual void	start_track(const char *,int repeats)=0;
	virtual void	stop_track(void)=0;
	virtual	bool	is_playing(void)=0;
	virtual	const	char *copyright(void)=0;

	MidiAbstract() {};
	virtual	~MidiAbstract() {};
};

//---- MidiAbstract -----------------------------------------------------------

#undef HAVE_TIMIDITY_BIN	// Disabled for now
#if HAVE_TIMIDITY_BIN
class	Timidity_binary : virtual public MidiAbstract
{
public:
	virtual void	start_track(const char *,int repeats);
	virtual void	stop_track(void);
	virtual	bool	is_playing(void);
	virtual const	char *copyright(void);

	Timidity_binary();
	virtual ~Timidity_binary();
private:
	pid_t	forked_job;
};
#endif


//---- MyMidiPlayer -----------------------------------------------------------

class	MyMidiPlayer
{
public:
	MyMidiPlayer();
	~MyMidiPlayer();
	MyMidiPlayer(const char *flexfile);
	void	start_music(int num,int repeats=0);
	void	start_track(int num,int repeats=0);

private:
	void    kmidi_start_track(int num,int repeats=0);
	Flex midi_tracks;
	Table instrument_patches;
	int	current_track;
	MidiAbstract	*midi_device;

};

#endif
