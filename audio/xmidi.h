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

// Tab Size = 4

#ifndef __IFF_h_
#define __IFF_h_

#if !AUTOCONFIGURED
#include "../autoconfig.h"
#endif

#include <string>
#include "common_types.h"
#include "U7file.h"


class   XMIDI
{
public:
	struct  midi_descriptor
	{
		uint16	type;
		uint16	tracks;
	};

	struct  midi_event
	{
		int		time;
		unsigned char	status;

		unsigned char	data[2];

		uint32	len;
		unsigned char	*stream;		;

		midi_event	*next;
	};

protected:
	midi_descriptor	info;

private:
	midi_event		**events;
	signed short	*timing;

	midi_event		*list;
	midi_event		*current;
	
	static unsigned char	mt32asgm[128];
	bool			convert_from_mt32;

public:
	// Reading from a file
	XMIDI(const char *fname);
	XMIDI(const std::string &fname);
	~XMIDI();

	// Reading from a byte stream
	XMIDI(const unsigned char *stream, std::size_t len);

	int number_of_tracks()
	{
		if (info.type != 1)
			return info.tracks;
		else
			return 1;
	};

	int retrieve (uint32 track, unsigned char **bufffer, std::size_t *len);	// Extract to a memory block
	int retrieve (uint32 track, const char *fname);					// Extract to a file
	
	// Not yet implimented
	// int apply_patch (int track, const char *fname);
	// int apply_patch (int track, const std::string &fname);

private:
	XMIDI(); // No default constructor
        
	// List manipulation
	void DeleteEventList (midi_event *mlist);
	void CreateNewEvent (int time);

	// Variable length quantity
	int GetVLQ (const unsigned char *stream, uint32 *quant);
	int GetVLQ2 (const unsigned char *stream, uint32 *quant);
	int PutVLQ(unsigned char *stream, uint32 value);

	int ConvertEvent (const int time, const unsigned char status, const unsigned char *stream, const int size);
	int ConvertSystemMessage (const int time, const unsigned char *stream);

	int ConvertEVNTtoList (const unsigned char *stream);
	int ConvertMTrktoList (const unsigned char *stream);
	uint32 ConvertListToMTrk (unsigned char *buf, midi_event *mlist);

	int ExtractTracksFromXmi (const unsigned char *stream, const uint32 size);
	int ExtractTracksFromMid (const unsigned char *stream, const uint32 size);
	
	int ExtractTracks (const unsigned char *stream);
	int ExtractTracksFromFile (const char *fname);

};

#endif
