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

#include <cstdlib>

#include "pent_include.h"
#include "XMidiEvent.h"
#include "XMidiEventList.h"

#ifdef PENTAGRAM_IN_EXULT
#include "databuf.h"
#else
#include "ODataSource.h"
#endif

#ifndef UNDER_EMBEDDED_CE
using std::atof;
using std::atoi;
using std::memcmp;
using std::memcpy;
using std::memset;
using std::size_t;
#endif
using std::string;
using std::endl;

//#include "gamma.h"


//
// XMidiEventList stuff
//
int XMidiEventList::write (ODataSource *dest)
{
	int len = 0;
	
	if (!events)
	{
		perr << "No midi data loaded." << endl;
		return 0;
	}

	// This is so if using buffer ODataSource, the caller can know how big to make the buffer
	if (!dest)
	{
		// Header is 14 bytes long and add the rest as well
		len = convertListToMTrk (NULL);
		return 14 + len;
	}
		
	dest->write1 ('M');
	dest->write1 ('T');
	dest->write1 ('h');
	dest->write1 ('d');
	
	dest->write4high (6);

	dest->write2high (0);
	dest->write2high (1);
	dest->write2high (60);	// The PPQN
		
	len = convertListToMTrk (dest);

	return len + 14;
}

//
// PutVLQ
//
// Write a Conventional Variable Length Quantity
//
int XMidiEventList::putVLQ(ODataSource *dest, uint32 value)
{
	int buffer;
	int i = 1;
	buffer = value & 0x7F;
	while (value >>= 7)
	{
		buffer <<= 8;
		buffer |= ((value & 0x7F) | 0x80);
		i++;
	}
	if (!dest) return i;
	for (int j = 0; j < i; j++)
	{
		dest->write1(buffer & 0xFF);
		buffer >>= 8;
	}
	
	return i;
}

// Converts and event list to a MTrk
// Returns bytes of the array
// buf can be NULL
uint32 XMidiEventList::convertListToMTrk (ODataSource *dest)
{
	int time = 0;
	int lasttime = 0;
	XMidiEvent	*event;
	uint32	delta;
	unsigned char	last_status = 0;
	uint32 	i = 8;
	uint32 	j;
	uint32	size_pos=0;

	if (dest)
	{
		dest->write1('M');
		dest->write1('T');
		dest->write1('r');
		dest->write1('k');

		size_pos = dest->getPos();
		dest->skip(4);
	}

	for (event = events; event; event=event->next)
	{
		// We don't write the end of stream marker here, we'll do it later
		if (event->status == 0xFF && event->data[0] == 0x2f) {
			lasttime = event->time;
			continue;
		}

		delta = (event->time - time);
		time = event->time;

		i += putVLQ (dest, delta);

		if ((event->status != last_status) || (event->status >= 0xF0))
		{
			if (dest) dest->write1(event->status);
			i++;
		}
		
		last_status = event->status;
		
		switch (event->status >> 4)
		{
			// 2 bytes data
			// Note off, Note on, Aftertouch, Controller and Pitch Wheel
			case 0x8: case 0x9: case 0xA: case 0xB: case 0xE:
			if (dest)
			{
				dest->write1(event->data[0]);
				dest->write1(event->data[1]);
			}
			i += 2;
			break;
			

			// 1 bytes data
			// Program Change and Channel Pressure
			case 0xC: case 0xD:
			if (dest) dest->write1(event->data[0]);
			i++;
			break;
			

			// Variable length
			// SysEx
			case 0xF:
			if (event->status == 0xFF)
			{
				if (dest) dest->write1(event->data[0]);
				i++;
			}
	
			i += putVLQ (dest, event->ex.sysex_data.len);
			
			if (event->ex.sysex_data.len)
			{
				for (j = 0; j < event->ex.sysex_data.len; j++)
				{
					if (dest) dest->write1(event->ex.sysex_data.buffer[j]); 
					i++;
				}
			}

			break;
			

			// Never occur
			default:
			perr << "Not supposed to see this" << endl;
			break;
		}
	}

	// Write out end of stream marker
	if (lasttime > time) i += putVLQ (dest, lasttime-time);
	else i += putVLQ (dest, 0);
	if (dest) {
		dest->write1(0xFF);
		dest->write1(0x2F);
	}
	i += 2+putVLQ (dest, 0);

	if (dest)
	{
		int cur_pos = dest->getPos();
		dest->seek (size_pos);
		dest->write4high (i-8);
		dest->seek (cur_pos);
	}
	return i;
}

void XMidiEventList::decerementCounter()
{
	if (--counter < 0) {
		events->FreeThis();
		events = 0;
		XMidiEvent::Free(this);
	}
}
