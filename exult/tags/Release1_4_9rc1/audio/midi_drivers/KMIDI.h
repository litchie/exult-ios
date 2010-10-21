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

#ifndef _MIDI_driver_KMIDI_h_
#define _MIDI_driver_KMIDI_h_

#ifdef XWIN
#if HAVE_LIBKMIDI
#define USE_LIBK_MIDI

#include <vector>
#include "Flex.h"
#include "Table.h"
#include "FileMidiDriver.h"

#include <libkmid.h>


class	KMIDI	: public FileMidiDriver
{
	bool	repeat_;

	const static MidiDriverDesc	desc;
	static MidiDriver *createInstance() {
		return new KMIDI();
	}
public:
	const static MidiDriverDesc* getDesc() { return &desc; }

	virtual void	start_track(const char *name,bool repeat,int vol);
	virtual void	stop_track(void);
	virtual	bool	is_playing(void);

	KMIDI();
	virtual ~KMIDI();
};
#endif
#endif

#endif	// Multiple inclusion protector
