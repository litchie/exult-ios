/*
Copyright (C) 2000  Max Horn

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


//MacOS-specific code
#ifdef MACOS

#include <iomanip>
#include <string>

#include "mac_midi.h"

#include <TextUtils.h>


#ifdef DEBUG
#define ASSERT(condition)	((condition) ? ((void) 0) : DebugStr("\pAssertion (" ## #condition ## ") " ## __FILE__ ## ", " ## "__LINE__"))
#else
#define ASSERT(x)
#endif

using std::cout;
using std::endl;
using std::memset;

static Handle BuildTuneSequence(midi_event *evntlist);
static UInt32 *BuildTuneHeader(int part_poly_max[32], int part_to_inst[32], int numParts);


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
		//TuneStop(mTunePlayer, kTuneStopFade);
		TuneStop(mTunePlayer, 0);
		CloseComponent(mTunePlayer);
		mTunePlayer = 0;
	}
	if(mTuneSequence)
	{
		DisposePtr((Ptr)mTuneSequence);
		mTuneSequence = 0;
	}
	if(mTuneHeader)
	{
		DisposePtr((Ptr)mTuneHeader);
		mTuneHeader = 0;
	}
}

void	Mac_QT_midi::start_track(midi_event *evntlist_head, int ppqn, bool repeat)
{
// FIXME !!! TODO !!! Currently, I use a static sized buffer of 10000 events;
//  this is very bad of course - will change it to a proper dynamic buffer later
//
// one way of doing this might be to revert back using a Handle for the tune memory,
// and then unlock/resize/lock the handle, as required... then we also would need to
// adjust tunePos, of course.
//
#define	MAX_NOTES		10000

	int			part_poly[32];
	int			part_poly_max[32];
	int			part_to_inst[32];
	int			channel_to_part[16];
	int			numParts = 0;
	int			last_time = 0;
	int			tempo = 500000;
	midi_event	*eventPos = evntlist_head;
	uint32 		*tunePos, *endPos;
	UInt32		queueFlags = 0;
	ComponentResult thisError;
	
	// First thing we do - stop any already playing music
	stop_track();

	// allocate space for the tune header
	mTuneSequence = (uint32 *)NewPtrClear(MAX_NOTES * sizeof(long));
	if (mTuneSequence == NULL)
		goto bail;
	
	// Set starting position in our tune memory
	tunePos = mTuneSequence;
	endPos = tunePos + MAX_NOTES;

	// Initialise the arrays
	memset(part_poly,0,sizeof(part_poly));
	memset(part_poly_max,0,sizeof(part_poly_max));
	
	memset(part_to_inst,-1,sizeof(part_to_inst));
	memset(channel_to_part,-1,sizeof(channel_to_part));

	/*
	 * Now the major work - iterate over all GM events,
	 * and turn them into QuickTime Music format.
	 * At the same time, calculate the max. polyphony for each part,
	 * and also the part->instrument mapping.
	 */
	COUT("NEW MIDI SONG, ppqn = " << ppqn << ", repat = " << repeat);
	while(eventPos && (tunePos < endPos))
	{
		int status = (eventPos->status&0xF0)>>4;
		int channel = eventPos->status&0x0F;
		int part = channel_to_part[channel];

#ifdef DEBUG
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
			ASSERT(part>=0 && part<=31);

			// Keep track of the polyphony of the current part
			part_poly[part]--;
			break;
		case MIDI_STATUS_NOTE_ON:
			ASSERT(part>=0 && part<=31);
TODO
if no part is yet specified, we should default to the first instrument - piano/drum kit 1


			// Remember the maximum polyphony for the current part
			int foo = ++part_poly[part];
			if (part_poly_max[part] < foo)
				part_poly_max[part] = foo;
			
			// Decode pitch & velocity
			int pitch = eventPos->data[0];
			int velocity = eventPos->data[1];
			
			// Now scan forward to find the matching NOTE OFF event
			midi_event *noteOffPos;
			for(noteOffPos = eventPos; noteOffPos; noteOffPos = noteOffPos->next)
			{
				if ((noteOffPos->status&0xF0)>>4 == MIDI_STATUS_NOTE_OFF
					&& pitch == noteOffPos->data[0])
					break;
				// NOTE ON with velocity == 0 is the same as a NOTE OFF
				if ((noteOffPos->status&0xF0)>>4 == MIDI_STATUS_NOTE_ON
					&& pitch == noteOffPos->data[0]
					&& 0 == noteOffPos->data[1])
					break;
			}
			
			// Did we find a note off? Should always be the case, but who knows...
			if (noteOffPos)
			{
				// We found a NOTE OFF, now calculate the note duration
				int duration = noteOffPos->time - eventPos->time;

				// Now we need to check if we get along with a normal Note Even, or if we need an extended one...
				if (duration < 2048 && pitch>=32 && part<=95 && velocity>=0 && velocity<=127)
					qtma_StuffNoteEvent(*tunePos, part, pitch, velocity, duration);
				else
					qtma_StuffXNoteEvent(*tunePos, *(tunePos+1), part, pitch, velocity, duration);
				tunePos++;
			}
			
			// Check if the timer advanced since the last NOTE ON, if yes,
			// insert a REST event
			if(eventPos->time != last_time)
			{
				qtma_StuffRestEvent(*tunePos, eventPos->time - last_time);
				tunePos++;
				last_time = eventPos->time;
			}
			break;
		case MIDI_STATUS_AFTERTOUCH:
			break;
		case MIDI_STATUS_CONTROLLER:
/*	FIX ME - we might not yet have a Part for this channel!!! so what we want to do:
	save controller settings in an array for each channel, then whenever a new part
	is defined for that channel, apply these settings to that part.
	If a part *is* already specified, then also apply the setting immediatly to the current channel
			int controller = evntlist->data[0];
			int value = evntlist->data[1] << 8;
			if(part < 0 || part > 31)
				DebugStr("\ppart out of range");
			if (controller == 7)
				qtma_StuffControlEvent(*tunePos, part, controller, value);
			tunePos++;
*/
			break;
		case MIDI_STATUS_PROG_CHANGE:
			// Instrument changed
			int newInst = eventPos->data[0];
			// Channel 9 (the 10th channel) is different, it indicates a drum kit
			if (channel == 9)
				newInst += kFirstDrumkit;
			else
				newInst += kFirstGMInstrument;
			// Only if the instrument for this channel *really* changed, add a new part.
			if(newInst != part_to_inst[part])
			{
				channel_to_part[channel] = numParts;
				part_to_inst[numParts++] = newInst;
			}
			break;
		case MIDI_STATUS_PRESSURE:
			break;
		case MIDI_STATUS_PITCH_WHEEL:
			ASSERT(part>=0 && part<=31);

			// In the midi spec, 0x2000 = center, 0x0000 = - 2 semitones, 0x3FFF = +2 semitones
			// but for QTMA, we specify it as a 8.8 fixed point of semitones
			int bend = eventPos->data[0] & 0x7f | (eventPos->data[1] & 0x7f) << 7;
			
			// "Center" the bend
			bend -= 0x2000;
			
			// Move it to our format:
			bend <<= 4;
			
			// Mask it to 16 bit
			bend &= 0xFFFF;
			
			qtma_StuffControlEvent(*tunePos, part, kControllerPitchBend, bend);
			tunePos++;
			
			
			break;
		case MIDI_STATUS_SYSEX:
			if (eventPos->status == 0xFF && eventPos->data[0] == 0x51) // Tempo change
			{
				tempo = (eventPos->buffer[0] << 16) +
					(eventPos->buffer[1] << 8) +
					eventPos->buffer[2];
					
				int tick = tempo/ppqn;
			}	
			break;
		}
		
		eventPos = eventPos->next;
	}
	
	// Finally, place an end marker
	*tunePos = kEndMarkerValue;
	
	// Now build a tune header from the data we collect above, create
	// all parts as needed and assign them the correct instrument.
	mTuneHeader = BuildTuneHeader(part_poly_max, part_to_inst, numParts);
	if(!mTuneHeader)
		goto bail;

	// Set up the queue flags
	queueFlags = kTuneStartNow;
	if(repeat)
		queueFlags |= kTuneLoopUntil;

	// tell that we have 600 units per second
	thisError = TuneSetTimeScale(mTunePlayer, ppqn * 500 / 384);
//	thisError = TuneSetTimeScale(mTunePlayer, 500);
//	thisError = TuneSetTimeScale(mTunePlayer, 600);
	if (thisError != noErr)
		DebugStr("\pMIDI error during TuneSetTimeScale");

	// Set the header, to tell what instruments are used
	thisError = TuneSetHeader(mTunePlayer, mTuneHeader);
	if (thisError != noErr)
		DebugStr("\pMIDI error during TuneSetHeader");
	
	// Have it allocate whatever resources are needed
	thisError = TunePreroll(mTunePlayer);
	if (thisError != noErr)
		DebugStr("\pMIDI error during TunePreroll");

	// We want to play at normal volume
	thisError = TuneSetVolume(mTunePlayer, 0x00010000);
	if (thisError != noErr)
		DebugStr("\pMIDI error during TuneSetVolume");
	
	thisError = TuneQueue(mTunePlayer,mTuneSequence,0x00010000,0,0x7FFFFFFF,queueFlags,NULL,0);
	if (thisError != noErr)
		DebugStr("\pMIDI error during TuneQueue");


//	while(is_playing() && !Button())
//		;
	
	return;
	
bail:

	stop_track();

}

void	Mac_QT_midi::stop_track(void)
{
// For some resons, using a non-zero stopflag doesn't stop the music :/
//	TuneStop(mTunePlayer, kTuneStopFade | kTuneStopInstant | kTuneStopReleaseChannels);
	TuneStop(mTunePlayer, 0);
	if(mTuneSequence)
	{
		DisposePtr((Ptr)mTuneSequence);
		mTuneSequence = 0;
	}
	if(mTuneHeader)
	{
		DisposePtr((Ptr)mTuneHeader);
		mTuneHeader = 0;
	}
}

void	Mac_QT_midi::stop_sfx(void)
{
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


#define kNoteRequestEventLength				((sizeof(NoteRequest)/sizeof(long)) + 2) 	// number of (32-bit) long words in a note request event
#define kMarkerEventLength					(1) 										// number of (32-bit) long words in a marker event
#define kGeneralEventLength					(2) 										// number of (32-bit) long words in a general event, minus its data
#define kNoteDuration						240											// in 600ths of a second
#define kRestDuration						300											// in 600ths of a second; tempo will be 120 bpm

UInt32 *BuildTuneHeader(int part_poly_max[32], int part_to_inst[32], int numParts)
{
	unsigned long			*myHeader;
	unsigned long			*myPos1, *myPos2;		// pointers to the head and tail long words of a music event
	NoteRequest				*myNoteRequest;
	NoteAllocator			myNoteAllocator;		// for the NAStuffToneDescription call
	long					myLongsInInst = 0L;		// the number of long words occupied by the atomic instrument
	ComponentResult			myErr = noErr;

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
	myHeader = (unsigned long *)
			NewPtrClear((numParts * kNoteRequestEventLength + kMarkerEventLength) * sizeof(long));
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
		myNoteRequest->info.reserved = 0;
		myNoteRequest->info.polyphony = part_poly_max[part];
		myNoteRequest->info.typicalPolyphony = 0x00010000;
		myErr = NAStuffToneDescription(myNoteAllocator,part_to_inst[part],&myNoteRequest->tone);
		if (myErr != noErr)
			goto bail;
		
#if DEBUG
		char instrumentName[256];
		CopyPascalStringToC(myNoteRequest->tone.instrumentName,instrumentName);
		COUT("Part " << part << ": max polyphony " << part_poly_max[part] << ", instrument '" << instrumentName << "'");
#endif

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


#if 0
Handle BuildTuneSequence(midi_event *evntlist)
{
	unsigned long 			*mySequence;
	unsigned long 			*myPos;
	Handle					myHandle;

	// allocate space for the tune header
	myHandle = NewHandleClear(22 * sizeof(long));
	if (myHandle == NULL)
		goto bail;
		
	HLock(myHandle);
	mySequence = (unsigned long *)*myHandle;

	myPos = mySequence;

	// *** do NOT attempt to do the following:
	// ***
	// ***	qtma_StuffNoteEvent(*myPos++, 1, 60, 100, kNoteDuration);
	// ***
	// *** your application will die a horrible death if you do....
	
	qtma_StuffNoteEvent(*myPos, 1, 60, 100, kNoteDuration);						// piano C
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;
	qtma_StuffNoteEvent(*myPos, 2, 60, 100, kNoteDuration);						// violin C
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;

	qtma_StuffNoteEvent(*myPos, 1, 63, 100, kNoteDuration);						// piano
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;
	qtma_StuffNoteEvent(*myPos, 2, 64, 100, kNoteDuration);						// violin
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;

	// make the 5th and 6th notes much softer, just for fun
	qtma_StuffNoteEvent(*myPos, 1, 67, 60, kNoteDuration);						// piano
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;
	qtma_StuffNoteEvent(*myPos, 2, 66, 60, kNoteDuration);						// violin
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;

	qtma_StuffNoteEvent(*myPos, 1, 72, 100, kNoteDuration);						// piano
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;
	qtma_StuffNoteEvent(*myPos, 2, 73, 100, kNoteDuration);						// violin
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;

	qtma_StuffNoteEvent(*myPos, 1, 60, 100, kNoteDuration);						// piano
	myPos++;
	qtma_StuffNoteEvent(*myPos, 1, 67, 100, kNoteDuration);						// piano
	myPos++;
	qtma_StuffNoteEvent(*myPos, 2, 63, 100, kNoteDuration);						// violin
	myPos++;
	qtma_StuffNoteEvent(*myPos, 2, 72, 100, kNoteDuration);						// violin
	myPos++;
	qtma_StuffRestEvent(*myPos, kRestDuration);
	myPos++;

	*myPos = kEndMarkerValue;													// end marker

bail:
	return(myHandle);
}

#else
Handle BuildTuneSequence(midi_event *evntlist_head)
{
#define	MAX_NOTES		10000

	unsigned long 		*mySequence;
	unsigned long 		*myPos, *endPos;
	Handle				myHandle;

	// allocate space for the tune header
	myHandle = NewHandleClear(MAX_NOTES * sizeof(long));
	if (myHandle == NULL)
		goto bail;
		
	HLock(myHandle);
	mySequence = (unsigned long *)*myHandle;

	myPos = mySequence;
	endPos = myPos + MAX_NOTES;

	int part_poly[32];
	int part_poly_max[32];
	int part_to_inst[32];
	int channel_to_part[16];
	int numParts = 0;
	int last_time = 0;
	midi_event *pos = evntlist_head;

	// Initialise the arrays
	memset(part_poly,0,sizeof(part_poly));
	memset(part_poly_max,0,sizeof(part_poly_max));
	memset(part_to_inst,-1,sizeof(part_to_inst));
	memset(channel_to_part,-1,sizeof(channel_to_part));

	// Loop over all sounds
	while(pos && (myPos < endPos))
	{
		int status = (pos->status&0xF0)>>4;
		int channel = pos->status&0x0F;
		int part = channel_to_part[channel];

		switch (status)
		{
		case MIDI_STATUS_NOTE_OFF:
			// Keep track of the polyphony of the current part
			if (part > 0)
				part_poly[part]--;
			break;
		case MIDI_STATUS_NOTE_ON:
			// Remember the maximum polyphony for the current part
			int foo = ++part_poly[part];
			if (part_poly_max[part] < foo)
				part_poly_max[part] = foo;
			
			// Decode pitch & velocity
			int pitch = pos->data[0];
			int velocity = pos->data[1];

			if(part<0 || part>31 || pitch<32 || pitch > 95 || velocity<0 || velocity>127)
				DebugStr("\ppart/pitch/velocity out of range");

			// Now scan forward to find the matching NOTE OFF event
			midi_event *noteOffPos;
			for(noteOffPos = pos; noteOffPos; noteOffPos = noteOffPos->next)
			{
				if ((noteOffPos->status&0xF0)>>4 == MIDI_STATUS_NOTE_OFF
					&& pitch == noteOffPos->data[0])
					break;
				// NOTE ON with velocity == 0 is also NOTE OFF
				if ((noteOffPos->status&0xF0)>>4 == MIDI_STATUS_NOTE_ON
					&& pitch == noteOffPos->data[0]
					&& 0 == noteOffPos->data[1])
					break;
			}
			
			if (noteOffPos)
			{
				// We found a NOTE OFF, now calculate the note duration
				int duration = noteOffPos->time - pos->time;
				// If the duration needs more than 11 bits, we need to stuff the note
				// into an extended note event.
				if (duration < 2048)
					qtma_StuffNoteEvent(*myPos, part, pitch, velocity, duration);
				else
					qtma_StuffXNoteEvent(*myPos, *(myPos+1), part, pitch, velocity, duration);
				myPos++;
			}
			
			// Check if the timer advanced since the last NOTE ON, if yes,
			// insert a REST event
			if(pos->time != last_time)
			{
				qtma_StuffRestEvent(*myPos, pos->time - last_time);
				myPos++;
				
				last_time = pos->time;
			}
			break;
		case MIDI_STATUS_AFTERTOUCH:
			break;
		case MIDI_STATUS_CONTROLLER:
/*	FIX ME - we might not yet have a Part for this channel!!! so what we want to do:
	save controller settings in an array for each channel, then whenever a new part
	is defined for that channel, apply these settings to that part.
	If a part *is* already specified, then also apply the setting immediatly to the current channel
			int controller = evntlist->data[0];
			int value = evntlist->data[1] << 8;
			if(part < 0 || part > 31)
				DebugStr("\ppart out of range");
			if (controller == 7)
				qtma_StuffControlEvent(*myPos, part, controller, value);
			myPos++;
*/
			break;
		case MIDI_STATUS_PROG_CHANGE:
			int newInst = pos->data[0];
			if (channel == 9)	// Drum channel
				newInst += kFirstDrumkit;
			else
				newInst += kFirstGMInstrument;
			if(newInst != part_to_inst[part])	// Was the instrument actually changed?
			{
				channel_to_part[channel] = numParts;
				part_to_inst[numParts++] = newInst;
			}
			break;
		case MIDI_STATUS_PRESSURE:
			break;
		case MIDI_STATUS_PITCH_WHEEL:
			break;
		//TODO - recognize "mute all"
		}
		

		pos = pos->next;
	}

	*myPos = kEndMarkerValue;													// end marker

bail:
	return(myHandle);
}
#endif

#endif
