/*
Copyright (C) 2001 Ruediger Hanke

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if defined( AMIGA ) || defined( __MORPHOS__ )

#define NO_PPCINLINE_STDARG
#include <proto/exec.h>
#include "amiga_midi.h"
#include "Configuration.h"
extern	Configuration	*config;

AmigaMIDI::AmigaMIDI()
{
	int amUnit = 0;

	FileOpen = false;
	amMsgPort = NULL;
	amMidiRequest = NULL;

	config->value( "config/audio/midi/unit", amUnit, 0 );

	/* Open amidi.device */
	amMsgPort = CreateMsgPort();
	if( amMsgPort )
	{
		amMidiRequest = (struct IOMidiRequest *)CreateIORequest( amMsgPort, sizeof( struct IOMidiRequest ) );
		if( amMidiRequest )
		{
			amMidiRequest->amr_Version = 1;
			if( !OpenDevice( "amidi.device", amUnit, (struct IORequest *)amMidiRequest, 0 ) )
				return;
			DeleteIORequest( amMidiRequest );
			amMidiRequest = NULL;
		}
		DeleteMsgPort( amMsgPort );
		amMsgPort = NULL;
	}
}

AmigaMIDI::~AmigaMIDI(void)
{
	stop_track();
	if( amMidiRequest )
	{
		CloseDevice( (struct IORequest *)amMidiRequest );
		DeleteIORequest( amMidiRequest );
		DeleteMsgPort( amMsgPort );
	}
}

void AmigaMIDI::stop_track(void)
{
	if( FileOpen )
	{
		if( amMidiRequest->amr_Std.io_Command == CMD_WRITE )
		{
			if( !CheckIO( (struct IORequest *)amMidiRequest ) )
				AbortIO( (struct IORequest *)amMidiRequest );
			WaitIO( (struct IORequest *)amMidiRequest );
		}

		FileOpen = false;
	}
}

bool AmigaMIDI::is_playing(void)
{
	bool open;

	open = FileOpen && amMidiRequest->amr_Std.io_Command == CMD_WRITE && CheckIO( (struct IORequest *)amMidiRequest ) == NULL;

	return open;
}


void AmigaMIDI::start_track(XMIDIEventList *event_list,bool repeat)
{
	const char *name = MIDITMPFILE;
	event_list->Write(name);

	if( amMidiRequest )
	{
		stop_track();

		FileOpen = true;
		amMidiRequest->amr_Std.io_Command = CMD_WRITE;
		amMidiRequest->amr_Std.io_Data = (APTR)name;
		amMidiRequest->amr_Std.io_Length = 0;
		amMidiRequest->amr_Loop = repeat ? -1 : 0;
		SendIO( (struct IORequest *)amMidiRequest );
	}
}

const	char *AmigaMIDI::copyright(void)
{
#ifdef __MORPHOS__
  return "MorphOS AMIDI MIDI player";
#else
  return "AmigaOS AMIDI MIDI player";
#endif
}

#endif
