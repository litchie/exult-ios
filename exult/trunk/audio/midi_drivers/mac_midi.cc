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
	
	int			channel_pan[16];
	int			channel_vol[16];
	
	int			numParts = 0;
	int			lastEventTime = 0;
	int			tempo = 500000;
	double		Ippqn = 1.0 / ppqn;
	double		tick = tempo * Ippqn;
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
	memset(channel_pan,-1,sizeof(channel_pan));
	memset(channel_vol,-1,sizeof(channel_vol));

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
			if(part<0 || part>31)
//			if (part == -1)
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
			
			// Decode pitch & velocity
			int pitch = eventPos->data[0];
			int velocity = eventPos->data[1];
			
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
					int oldDuration = duration;
					
					duration *= tick / 1000;
			ASSERT((duration & 0xFFC00000) == 0);
					double foo = oldDuration;
					foo *= tick;
					
					// Now we need to check if we get along with a normal Note Even, or if we need an extended one...
					if (duration < 2048 && pitch>=32 && part<=95 && velocity>=0 && velocity<=127)
						qtma_StuffNoteEvent(*tunePos, part, pitch, velocity, duration);
					else
						qtma_StuffXNoteEvent(*tunePos, *(tunePos+1), part, pitch, velocity, duration);
					tunePos++;
				}
			}
			break;
		case MIDI_STATUS_AFTERTOUCH:
			break;
		case MIDI_STATUS_CONTROLLER:
/*
enum {
  kControllerModulationWheel    = 1,
  kControllerBreath             = 2,
  kControllerFoot               = 4,
  kControllerPortamentoTime     = 5,    // time in 8.8 seconds, portamento on/off is omitted, 0 time = 'off'
  kControllerVolume             = 7,    // main volume control
  kControllerBalance            = 8,
  kControllerPan                = 10,   // 0 - "default", 1 - n: positioned in output 1-n (incl fractions)
  kControllerExpression         = 11,   // secondary volume control
  kControllerPitchBend          = 32,   // positive & negative semitones, with 8 bits fraction, same units as transpose controllers
  kControllerAfterTouch         = 33,   // aka channel pressure
  kControllerPartTranspose      = 40,   // identical to pitchbend, for overall part xpose
  kControllerTuneTranspose      = 41,   // another pitchbend, for "song global" pitch offset
  kControllerPartVolume         = 42,   // another volume control, passed right down from note allocator part volume
  kControllerTuneVolume         = 43,   // another volume control, used for "song global" volume - since we share one synthesizer across multiple tuneplayers
  kControllerSustain            = 64,   // boolean - positive for on, 0 or negative off
  kControllerPortamento         = 65,   // boolean
  kControllerSostenuto          = 66,   // boolean
  kControllerSoftPedal          = 67,   // boolean
  kControllerReverb             = 91,
  kControllerTremolo            = 92,
  kControllerChorus             = 93,
  kControllerCeleste            = 94,
  kControllerPhaser             = 95,
  kControllerEditPart           = 113,  // last 16 controllers 113-128 and above are global controllers which respond on part zero
  kControllerMasterTune         = 114,
  kControllerMasterTranspose    = 114,  // preferred
  kControllerMasterVolume       = 115,
  kControllerMasterCPULoad      = 116,
  kControllerMasterPolyphony    = 117,
  kControllerMasterFeatures     = 118
};
*/
/*	FIX ME - we might not yet have a Part for this channel!!! so what we want to do:
	save controller settings in an array for each channel, then whenever a new part
	is defined for that channel, apply these settings to that part.
	If a part *is* already specified, then also apply the setting immediatly to the current channel
			if(part < 0 || part > 31)
				DebugStr("\ppart out of range");
			if (controller == 7)
				qtma_StuffControlEvent(*tunePos, part, controller, value);
			tunePos++;
*/
			int controller = eventPos->data[0];
			int value = eventPos->data[1];

			switch(controller)
			{
			case kControllerVolume:
				if(channel_vol[channel] != value<<8)
				{
					channel_vol[channel] = value<<8;
					if(part>=0 && part<=31)
					{
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
						qtma_StuffControlEvent(*tunePos, part, kControllerPan, channel_pan[channel]);
						tunePos++;
					}
				}
				break;
			}
			
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
				part = channel_to_part[channel] = numParts;
				part_to_inst[numParts++] = newInst;
			}
#if 1
			if(channel_vol[channel] > 0)
			{
				qtma_StuffControlEvent(*tunePos, part, kControllerVolume, channel_vol[channel]);
				tunePos++;
			}
			if(channel_pan[channel] > 0)
			{
				qtma_StuffControlEvent(*tunePos, part, kControllerPan, channel_pan[channel]);
				tunePos++;
			}
#endif
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
					
				tick = tempo * Ippqn;
			}	
			break;
		}
		
		// Check if the timer advanced since the last NOTE ON, if yes,
		// insert a REST event
		if(eventPos->time != lastEventTime)
		{
			int restDuration = (eventPos->time - lastEventTime) * tick / 1000;
			ASSERT((restDuration & 0xFF000000) == 0);
			qtma_StuffRestEvent(*tunePos, restDuration);
			tunePos++;
			lastEventTime = eventPos->time;
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

	// Set the time scale (units per second)
//	thisError = TuneSetTimeScale(mTunePlayer, ppqn * 500 / 384);
	thisError = TuneSetTimeScale(mTunePlayer, 1000);
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
	
	// Free the event list
	XMIDI::DeleteEventList (evntlist_head);

	return;
	
bail:

	// Free the event list
	XMIDI::DeleteEventList (evntlist_head);

	// This disposes of the allocated tune header/sequence
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

#endif
