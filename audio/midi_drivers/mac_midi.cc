/*
 *	mac_midi.cc - QuickTime based midi player for MacOS.
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

//MacOS-specific code
#if defined(MACOS) || defined(MACOSX)

#include <iomanip>
#include <cstdlib>
#include <string>

#include "mac_midi.h"

#ifdef MACOSX
#include <QuickTime/QuickTimeMusic.h>
#endif

using std::cout;
using std::endl;
using std::malloc;
using std::free;
using std::realloc;
using std::memset;

enum
{
	// number of (32-bit) long words in a note request event
	kNoteRequestEventLength = ((sizeof(NoteRequest)/sizeof(long)) + 2),

	// number of (32-bit) long words in a marker event
	kMarkerEventLength	= 1,

	// number of (32-bit) long words in a general event, minus its data
	kGeneralEventLength	= 2
};

#define	BUFFER_INCREMENT		5000

#define REST_IF_NECESSARY()	do {\
			int timeDiff = eventPos->time - lastEventTime;	\
			if(timeDiff)	\
			{	\
				timeDiff = (int)(timeDiff*tick);	\
				qtma_StuffRestEvent(*tunePos, timeDiff);	\
				tunePos++;	\
				lastEventTime = eventPos->time;	\
			}	\
		} while(0)


static uint32 *BuildTuneSequence(midi_event *evntlist, int ppqn, int part_poly_max[32], int part_to_inst[32], int &numParts);
static uint32 *BuildTuneHeader(int part_poly_max[32], int part_to_inst[32], int numParts);


Mac_QT_midi::Mac_QT_midi()
	: MidiAbstract(), mTunePlayer(0), mTuneSequence(0), mTuneHeader(0)
{
// FIX ME - check for QuickTime!!!
	
	mTunePlayer = OpenDefaultComponent(kTunePlayerComponentType, 0);
	if( 0 == mTunePlayer )
		throw exult_exception("Couldn't get tune place component");
}

Mac_QT_midi::~Mac_QT_midi(void)
{
	if(mTunePlayer)
	{
		if(mTuneSequence)
			TuneStop(mTunePlayer, 0);
		CloseComponent(mTunePlayer);
		mTunePlayer = 0;
	}
	if(mTuneSequence)
	{
		free(mTuneSequence);
		mTuneSequence = 0;
	}
	if(mTuneHeader)
	{
		DisposePtr((Ptr)mTuneHeader);
		mTuneHeader = 0;
	}
}


void	Mac_QT_midi::start_track(XMIDIEventList *elist, bool repeat)
{
	int			part_to_inst[32];
	int			part_poly_max[32];
	int			numParts = 0;

	// Max, this is just a work around that should allow things to compile
	// Note that now the ppqn is always 60, and the tempo is always 500000 mcs
	// This means each tick is now always 120th of a second
	int ppqn = 60;
	midi_event *evntlist = elist->events;

	UInt32		queueFlags = 0;
	ComponentResult tpError;

	memset(part_poly_max,0,sizeof(part_poly_max));
	memset(part_to_inst,-1,sizeof(part_to_inst));

	// First thing we do - stop any already playing music
	stop_track();
	
	// Build a tune sequence from the event list
	mTuneSequence = BuildTuneSequence(evntlist, ppqn, part_poly_max, part_to_inst, numParts);
	if(!mTuneSequence)
		goto bail;

	// Now build a tune header from the data we collect above, create
	// all parts as needed and assign them the correct instrument.
	mTuneHeader = BuildTuneHeader(part_poly_max, part_to_inst, numParts);
	if(!mTuneHeader)
		goto bail;

	// Set up the queue flags
	queueFlags = kTuneStartNow;
	if(repeat)
		queueFlags |= kTuneLoopUntil;

	// Set the time scale (units per second), we want milliseconds
	tpError = TuneSetTimeScale(mTunePlayer, 1000);
	if (tpError != noErr)
		DebugStr("\pMIDI error during TuneSetTimeScale");

	// Set the header, to tell what instruments are used
	tpError = TuneSetHeader(mTunePlayer, (UInt32 *)mTuneHeader);
	if (tpError != noErr)
		DebugStr("\pMIDI error during TuneSetHeader");
	
	// Have it allocate whatever resources are needed
	tpError = TunePreroll(mTunePlayer);
	if (tpError != noErr)
		DebugStr("\pMIDI error during TunePreroll");

	// We want to play at normal volume
	tpError = TuneSetVolume(mTunePlayer, 0x00010000);
	if (tpError != noErr)
		DebugStr("\pMIDI error during TuneSetVolume");
	
	// Finally, start playing the full song
	tpError = TuneQueue(mTunePlayer, (UInt32 *)mTuneSequence, 0x00010000, 0, 0xFFFFFFFF, queueFlags, NULL, 0);
	if (tpError != noErr)
		DebugStr("\pMIDI error during TuneQueue");


#if defined(DEBUG) && 0
	// Loop until music is finished
	while(is_playing() && !Button())
		;
#endif
	
	// Free the event list
	XMIDI::DeleteEventList (elist);

	return;
	
bail:
	// Free the event list
	XMIDI::DeleteEventList (elist);

	// This disposes of the allocated tune header/sequence
	stop_track();
}

void	Mac_QT_midi::stop_track(void)
{
	if(0 == mTuneSequence)
        return;

// For some resons, using a non-zero stopflag doesn't stop the music at all:/
//	TuneStop(mTunePlayer, kTuneStopFade | kTuneStopInstant | kTuneStopReleaseChannels);

	// Stop music
	TuneStop(mTunePlayer, 0);
	
	// Deallocate all instruments
	TuneUnroll(mTunePlayer);
	
	// Finally, free the data storage
	free(mTuneSequence);
	mTuneSequence = 0;

	if(mTuneHeader)
	{
		DisposePtr((Ptr)mTuneHeader);
		mTuneHeader = 0;
	}
}

bool	Mac_QT_midi::is_playing(void)
{
	TuneStatus	ts;

	TuneGetStatus(mTunePlayer,&ts);
	return ts.queueTime != 0;
}

const	char *Mac_QT_midi::copyright(void)
{
	return "Internal QuickTime MIDI player";
}

uint32 *BuildTuneSequence(midi_event *evntlist, int ppqn, int part_poly_max[32], int part_to_inst[32], int &numParts)
{
	int			part_poly[32];
	int			channel_to_part[16];
	
	int			channel_pan[16];
	int			channel_vol[16];
	int			channel_pitch_bend[16];
	
	int			lastEventTime = 0;
	int			tempo = 500000;
	double		Ippqn = 1.0 / (1000*ppqn);
	double		tick = tempo * Ippqn;
	midi_event	*eventPos = evntlist;
	uint32 		*tunePos, *endPos;
	uint32		*tuneSequence;
	size_t		tuneSize;
#ifdef DEBUG
	int			numEventsHandled = 0;
#endif
	
	// allocate space for the tune header
	tuneSize = 5000;
	tuneSequence = (uint32 *)malloc(tuneSize * sizeof(uint32));
	if (tuneSequence == NULL)
		return NULL;
	
	// Set starting position in our tune memory
	tunePos = tuneSequence;
	endPos = tuneSequence + tuneSize;

	// Initialise the arrays
	memset(part_poly,0,sizeof(part_poly));
	
	memset(channel_to_part,-1,sizeof(channel_to_part));
	memset(channel_pan,-1,sizeof(channel_pan));
	memset(channel_vol,-1,sizeof(channel_vol));
	memset(channel_pitch_bend,-1,sizeof(channel_pitch_bend));

	/*
	 * Now the major work - iterate over all GM events,
	 * and turn them into QuickTime Music format.
	 * At the same time, calculate the max. polyphony for each part,
	 * and also the part->instrument mapping.
	 */
	while(eventPos)
	{
		int status = (eventPos->status&0xF0)>>4;
		int channel = eventPos->status&0x0F;
		int part = channel_to_part[channel];
        int velocity, pitch;
        int value, controller;
        int bend;
        int newInst;
		
		// Check if we are running low on space...
		if((tunePos+16) > endPos)
		{
			// Resize our data storage.
			uint32 		*oldTuneSequence = tuneSequence;

			tuneSize += BUFFER_INCREMENT;
			tuneSequence = (uint32 *)realloc(tuneSequence, tuneSize * sizeof(uint32));
			if(oldTuneSequence != tuneSequence)
				tunePos += tuneSequence - oldTuneSequence;
			endPos = tuneSequence + tuneSize;
		}
		
#if defined(DEBUG) && 0
		numEventsHandled++;

		cout << std::setw(4) << numEventsHandled << ". ";
		cout << "Time: " << std::setw(5) << eventPos->time << " ";
		cout << std::hex << std::uppercase;
		cout << "Status 0x" << status << " Channel: 0x" << channel << " ";
		cout << "Data 0x" << std::setw(4) <<*(unsigned short *)(eventPos->data) << " ";
		cout << std::dec;
		cout << "Length " << eventPos->len << endl;
#endif

		switch (status)
		{
		case MIDI_STATUS_NOTE_OFF:
			assert(part>=0 && part<=31);

			// Keep track of the polyphony of the current part
			part_poly[part]--;
			break;
		case MIDI_STATUS_NOTE_ON:
			if (part < 0)
			{
				// If no part is specified yet, we default to the first instrument, which
				// is piano (or the first drum kit if we are on the drum channel)
				int newInst;
				
				if (channel == 9)
					newInst = kFirstDrumkit + 1;		// the first drum kit is the "no drum" kit!
				else
					newInst = kFirstGMInstrument;
				part = channel_to_part[channel] = numParts;
				part_to_inst[numParts++] = newInst;
			}
			// TODO - add support for more than 32 parts using eXtended QTMA events
			assert(part<=31);
			
			// Decode pitch & velocity
			pitch = eventPos->data[0];
			velocity = eventPos->data[1];
			
			if (velocity == 0)
			{
				// was a NOTE OFF in disguise, so we decrement the polyphony
				part_poly[part]--;
			}
			else
			{
				// Keep track of the polyphony of the current part
				int foo = ++part_poly[part];
				if (part_poly_max[part] < foo)
					part_poly_max[part] = foo;

				// Now scan forward to find the matching NOTE OFF event
				midi_event *noteOffPos;
				for(noteOffPos = eventPos; noteOffPos; noteOffPos = noteOffPos->next)
				{
					if ((noteOffPos->status&0xF0)>>4 == MIDI_STATUS_NOTE_OFF
						&& channel == (eventPos->status&0x0F)
						&& pitch == noteOffPos->data[0])
						break;
					// NOTE ON with velocity == 0 is the same as a NOTE OFF
					if ((noteOffPos->status&0xF0)>>4 == MIDI_STATUS_NOTE_ON
						&& channel == (eventPos->status&0x0F)
						&& pitch == noteOffPos->data[0]
						&& 0 == noteOffPos->data[1])
						break;
				}
				
				// Did we find a note off? Should always be the case, but who knows...
				if (noteOffPos)
				{
					// We found a NOTE OFF, now calculate the note duration
					int duration = (int)((noteOffPos->time - eventPos->time)*tick);
					
					REST_IF_NECESSARY();
					// Now we need to check if we get along with a normal Note Even, or if we need an extended one...
					if (duration < 2048 && pitch>=32 && pitch<=95 && velocity>=0 && velocity<=127)
					{
						qtma_StuffNoteEvent(*tunePos, part, pitch, velocity, duration);
						tunePos++;
					}
					else
					{
						qtma_StuffXNoteEvent(*tunePos, *(tunePos+1), part, pitch, velocity, duration);
						tunePos+=2;
					}
					
#if defined(DEBUG) && 0
					cout << std::setw(4) << numEventsHandled << ". ";
					cout << "NOTE ON duration " << std::setw(4) << duration << " ";
					cout << "Channel " << std::setw(2) << channel << " ";
					cout << "Part " << std::setw(2) << part << " ";
					cout << "Pitch " << std::setw(2) << pitch << " ";
					cout << "Velocity " << std::setw(2) << velocity << endl;
#endif
				}
			}
			break;
		case MIDI_STATUS_AFTERTOUCH:
			COUT("MIDI_STATUS_AFTERTOUCH");
			break;
		case MIDI_STATUS_CONTROLLER:
			controller = eventPos->data[0];
			value = eventPos->data[1];

			switch(controller)
			{
			case 0:	// bank change - igore for now
				break;
			case kControllerVolume:
				if(channel_vol[channel] != value<<8)
				{
					channel_vol[channel] = value<<8;
					if(part>=0 && part<=31)
					{
						REST_IF_NECESSARY();
						qtma_StuffControlEvent(*tunePos, part, kControllerVolume, channel_vol[channel]);
						tunePos++;
					}
				}
				break;
			case kControllerPan:
				if(channel_pan[channel] != ((value-64)<<8))
				{
					channel_pan[channel] = (value-64)<<8;
					if(part>=0 && part<=31)
					{
						REST_IF_NECESSARY();
						qtma_StuffControlEvent(*tunePos, part, kControllerPan, channel_pan[channel]);
						tunePos++;
					}
				}
				break;
			default:
				COUT("CONTROLLER not handled: "<< controller);
				break;
			}
			
			break;
		case MIDI_STATUS_PROG_CHANGE:
			// Instrument changed
			newInst = eventPos->data[0];
			// Channel 9 (the 10th channel) is different, it indicates a drum kit
			if (channel == 9)
				newInst += kFirstDrumkit;
			else
				newInst += kFirstGMInstrument;
			// Only if the instrument for this channel *really* changed, add a new part.
			if(newInst != part_to_inst[part])
			{
				// TODO maybe make use of kGeneralEventPartChange here,
				// to help QT reuse note channels?
				part = channel_to_part[channel] = numParts;
				part_to_inst[numParts++] = newInst;

				if(channel_vol[channel] >= 0)
				{
					REST_IF_NECESSARY();
					qtma_StuffControlEvent(*tunePos, part, kControllerVolume, channel_vol[channel]);
					tunePos++;
				}
				if(channel_pan[channel] >= 0)
				{
					REST_IF_NECESSARY();
					qtma_StuffControlEvent(*tunePos, part, kControllerPan, channel_pan[channel]);
					tunePos++;
				}
				if(channel_pitch_bend[channel] >= 0)
				{
					REST_IF_NECESSARY();
					qtma_StuffControlEvent(*tunePos, part, kControllerPitchBend, channel_pitch_bend[channel]);
					tunePos++;
				}			
			}
			break;
		case MIDI_STATUS_PRESSURE:
			COUT("MIDI_STATUS_PRESSURE");
			break;
		case MIDI_STATUS_PITCH_WHEEL:
			// In the midi spec, 0x2000 = center, 0x0000 = - 2 semitones, 0x3FFF = +2 semitones
			// but for QTMA, we specify it as a 8.8 fixed point of semitones
			// TODO: detect "pitch bend range changes"
			bend = eventPos->data[0] & 0x7f | (eventPos->data[1] & 0x7f) << 7;
			
			// "Center" the bend
			bend -= 0x2000;
			
			// Move it to our format:
			bend <<= 4;
			
			// If it turns out the pitch bend didn't change, stop here
			if(channel_pitch_bend[channel] == bend)
				break;
			
			channel_pitch_bend[channel] = bend;
			if(part>=0 && part<=31)
			{
				// Stuff a control event
				REST_IF_NECESSARY();
				qtma_StuffControlEvent(*tunePos, part, kControllerPitchBend, bend);
				tunePos++;
			}			
			break;
		case MIDI_STATUS_SYSEX:
			if (eventPos->status == 0xFF && eventPos->data[0] == 0x51) // Tempo change
			{
				tempo = (eventPos->buffer[0] << 16) +
					(eventPos->buffer[1] << 8) +
					eventPos->buffer[2];
				
				tick = tempo * Ippqn;
			}
			break;
		}
		
		// on to the next event
		eventPos = eventPos->next;
	} 
	
	// Finally, place an end marker
	*tunePos = kEndMarkerValue;
	
	return tuneSequence;
}


uint32 *BuildTuneHeader(int part_poly_max[32], int part_to_inst[32], int numParts)
{
	uint32			*myHeader;
	uint32			*myPos1, *myPos2;		// pointers to the head and tail long words of a music event
	NoteRequest		*myNoteRequest;
	NoteAllocator	myNoteAllocator;		// for the NAStuffToneDescription call
	ComponentResult	myErr = noErr;

	myHeader = NULL;
	myNoteAllocator = NULL;

	/*
	 * Open up the Note Allocator
	 */
	myNoteAllocator = OpenDefaultComponent(kNoteAllocatorComponentType,0);
	if (myNoteAllocator == NULL)
		goto bail;
	
	/*
	 * Allocate space for the tune header
	 */
	myHeader = (uint32 *)
			NewPtrClear((numParts * kNoteRequestEventLength + kMarkerEventLength) * sizeof(uint32));
	if (myHeader == NULL)
		goto bail;
	
	myPos1 = myHeader;
	
	/*
	 * Loop over all parts
	 */
	for(int part = 0; part < numParts; ++part)
	{
		/*
		 * Stuff request for the instrument with the given polyphony
		 */
		myPos2 = myPos1 + (kNoteRequestEventLength - 1); // last longword of general event
		qtma_StuffGeneralEvent(*myPos1, *myPos2, part, kGeneralEventNoteRequest, kNoteRequestEventLength);
		myNoteRequest = (NoteRequest *)(myPos1 + 1);
		myNoteRequest->info.flags = 0;
		myNoteRequest->info.polyphony = part_poly_max[part];
		myNoteRequest->info.typicalPolyphony = 0x00010000;
		myErr = NAStuffToneDescription(myNoteAllocator,part_to_inst[part],&myNoteRequest->tone);
		if (myErr != noErr)
			goto bail;
		
		// move pointer to beginning of next event
		myPos1 += kNoteRequestEventLength;
	}

	*myPos1 = kEndMarkerValue;		/* end of sequence marker */


bail:
	if(myNoteAllocator)
		CloseComponent(myNoteAllocator);

	// if we encountered an error, dispose of the storage we allocated and return NULL
	if (myErr != noErr) {
		DisposePtr((Ptr)myHeader);
		myHeader = NULL;
	}

	return myHeader;
}

#endif
