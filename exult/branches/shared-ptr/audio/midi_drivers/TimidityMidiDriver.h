/*
Copyright (C) 2003  The Pentagram Team

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

#ifndef TIMIDITYMIDIDRIVER_H_INCLUDED
#define TIMIDITYMIDIDRIVER_H_INCLUDED

#ifdef USE_TIMIDITY_MIDI

#include "LowLevelMidiDriver.h"

class TimidityMidiDriver : public LowLevelMidiDriver
{

	bool	used_inst[128];
	bool	used_drums[128];

	const static MidiDriverDesc	desc;
	static MidiDriver *createInstance() {
		return new TimidityMidiDriver();
	}

public:
	static const MidiDriverDesc* getDesc() { return &desc; }
	TimidityMidiDriver();

protected:
	// LowLevelMidiDriver implementation
	virtual int			open();
	virtual void		close();
	virtual void		send(uint32 b);
	virtual void		lowLevelProduceSamples(sint16 *samples, uint32 num_samples);

	// MidiDriver overloads
	virtual bool		isSampleProducer() { return true; }
	virtual bool		noTimbreSupport() { return true; }
};

#endif //USE_TIMIDITY_MIDI

#endif //TIMIDITYMIDIDRIVER_H_INCLUDED
