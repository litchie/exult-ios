/*
Code originally written by Max Horn for ScummVM,
minor tweaks by various other people of the ScummVM, and Exult teams.

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

#ifndef COREMIDIDRIVER_H_INCLUDED
#define COREMIDIDRIVER_H_INCLUDED

#ifdef MACOSX
#define USE_CORE_MIDI

#include "LowLevelMidiDriver.h"

#include <CoreMIDI/CoreMIDI.h>

class CoreMidiDriver : public LowLevelMidiDriver
{

	MIDIClientRef	mClient;
	MIDIPortRef		mOutPort;
	MIDIEndpointRef	mDest;

	static const MidiDriverDesc	desc;
	static MidiDriver *createInstance() {
		return new CoreMidiDriver();
	}

public:
	static const MidiDriverDesc* getDesc() { return &desc; }

	CoreMidiDriver();
	~CoreMidiDriver();

protected:
	virtual int			open();
	virtual void		close();
	virtual void		send(uint32 message);
	virtual void		send_sysex(uint8 status, const uint8 *msg, uint16 length);
	virtual void		increaseThreadPriority();
	virtual void		yield();
};

#endif //MACOSX

#endif //COREMIDIDRIVER_H_INCLUDED
