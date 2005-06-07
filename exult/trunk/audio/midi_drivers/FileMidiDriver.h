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

#ifndef FILE_MIDI_DRIVER_H_INCLUDED
#define FILE_MIDI_DRIVER_H_INCLUDED

#include "MidiDriver.h"

class	FileMidiDriver : public MidiDriver
{
	int					global_volume;
	int					seq_volume;

public:
	// MidiDriver Implementation
	virtual int			initMidiDriver(uint32 sample_rate, bool stereo);
	virtual void		destroyMidiDriver();
	virtual int			maxSequences();
	virtual void		setGlobalVolume(int vol);

	virtual void		startSequence(int seq_num, XMidiEventList *list, bool repeat, int vol, int branch = -1);
	virtual void		finishSequence(int seq_num);
	virtual void		pauseSequence(int seq_num);
	virtual void		unpauseSequence(int seq_num);
	virtual void		setSequenceVolume(int seq_num, int vol);
	virtual void		setSequenceSpeed(int seq_num, int speed);
	virtual bool		isSequencePlaying(int seq_num);

	virtual bool		noTimbreSupport() { return true; }

	FileMidiDriver();
	virtual ~FileMidiDriver();

protected:

	//! Open the Midi Device
	//! \return 0 on sucess. Non zero on failure.
	virtual int			open()=0;

	//! Close the Midi Device
	virtual void		close()=0;

	//! Start playing the track
	virtual void		start_track(const char *filename, bool repeat, int vol)=0;

	//! Stop playing the track
	virtual void		stop_track()=0;

	//! Is it playing?
	virtual bool		is_playing()=0;

	//! Set the volume
	virtual void		set_volume(int vol) { };

	//! Get the temporary filename
	virtual const char	*get_temp_name();
};

#endif //FILE_MIDI_DRIVER_H_INCLUDED
