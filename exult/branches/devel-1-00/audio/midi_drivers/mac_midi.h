/*
 *	mac_midi.h - QuickTime based midi player for MacOS.
 *
 *  Copyright (C) 2001  Max Horn
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

#ifndef _MIDI_driver_macos_midi_h_
#define _MIDI_driver_macos_midi_h_

#if defined(MACOS) || defined(MACOSX)

#ifdef MACOS
#include <QuickTimeMusic.h>
#else
// Work around a conflict between exult's Palette class
// and the Carbon headers
typedef struct ComponentInstanceRecord *       TunePlayer;
#endif

#include "Midi.h"
#include "exceptions.h"


class	Mac_QT_midi : virtual public MidiAbstract
{
public:
	// Do we accept events, YES!
	virtual bool	accepts_events(void) { return true; }

	// Event based methods
	virtual void	start_track(XMIDIEventList *, bool repeat);
	virtual void	stop_track(void);
	virtual bool	is_playing(void);
	virtual const char *copyright(void);

	Mac_QT_midi();
	virtual ~Mac_QT_midi();

private:
	UNREPLICATABLE_CLASS(Mac_QT_midi);
	
	TunePlayer	mTunePlayer;
	uint32		*mTuneSequence;
	uint32		*mTuneHeader;
};

#endif

#endif
