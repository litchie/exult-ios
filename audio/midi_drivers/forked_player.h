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

#ifndef _MIDI_driver_forked_player_h_
#define _MIDI_driver_forked_player_h_

#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma interface
#endif

#include <vector>
#include <sys/types.h>
#include "Flex.h"
#include "Table.h"
#include "Midi.h"
#if HAVE_LIBKMIDI
#include <libkmid.h>
#endif


class	forked_player	:	virtual public MidiAbstract
{
	bool	repeat_;
public:
	virtual void	start_track(XMIDIEventList *, bool repeat);
//	virtual void	start_sfx(XMIDIEventList *);
	virtual void	stop_track(void);
	virtual	bool	is_playing(void);
	virtual const	char *copyright(void);

	forked_player();
	virtual ~forked_player();
private:
	pid_t	forked_job;
};

#endif
