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

#include <string>

#include "mac_midi.h"


Mac_QT_midi::Mac_QT_midi()
	: MidiAbstract(), mTunePlayer(0)
{
// FIX ME - check for QuickTime!!!
	
//	EnterMovies();		// Do we need this?

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

/*
		// Kill the old music's header and data as well
		if(tuneHeaderPtr) {
			DisposePtr(tuneHeaderPtr);
			tuneHeaderPtr = nil;
			if (kMusicDebug) ReportDialog("\pShutDownMusic(): Tune Header disposed");
		}
		if(tuneDataPtr) {
			DisposePtr(tuneDataPtr);
			tuneDataPtr = nil;
			
			if (kMusicDebug) ReportDialog("\pShutDownMusic(): Tune Data disposed");
		}
*/
}

void	Mac_QT_midi::start_track(const char *name, bool repeat)
{
//	mRepeat = repeat;
	
//	TuneSetHeaderWithSize()

	idle();
}

void	Mac_QT_midi::start_track(midi_event *evntlist, int ppqn, bool repeat)
{
	mRepeat = repeat;
	
//	TuneSetHeaderWithSize()

#if 0
	theError = TuneQueue();
#endif

	idle();
}

void	Mac_QT_midi::stop_track(void)
{
#if 0
	OSErr	err;

	err = TuneStop(mTunePlayer, 0);
#endif

	idle();
}

void	Mac_QT_midi::stop_sfx(void)
{
#if 0
	OSErr	err;

	err = TuneStop(mTunePlayer, 0);
#endif

	idle();
}

bool	Mac_QT_midi::is_playing(void)
{
	return	true;
}

const	char *Mac_QT_midi::copyright(void)
{
	return "Internal QuickTime MIDI player";
}

void	Mac_QT_midi::idle(void)
{
	if( mTunePlayer )
		TuneTask(mTunePlayer);
}

#endif




// excerpt from QuickTimeMusic.h

/*    QuickTime Music Track Event Formats:

    At this time, QuickTime music tracks support 5 different event types -- REST events,
    short NOTE events, short CONTROL events, short GENERAL events, Long NOTE events, 
    long CONTROL events, and variable GENERAL events.
 
        ¥ REST Event (4 bytes/event):
    
            (0 0 0) (5-bit UNUSED) (24-bit Rest Duration)
        
        ¥ÊShort NOTE Events (4 bytes/event):
    
            (0 0 1) (5-bit Part) (6-bit Pitch) (7-bit Volume) (11-bit Duration)
        
            where:  Pitch is offset by 32 (Actual pitch = pitch field + 32)

        ¥ÊShort CONTROL Events (4 bytes/event):
    
            (0 1 0) (5-bit Part) (8-bit Controller) (1-bit UNUSED) (1-bit Sign) (7-bit MSB) (7-bit LSB)
                                                                         ( or 15-bit Signed Value)
        ¥ Short GENERAL Event (4 bytes/event):
    
            (0 1 1) (1-bit UNUSED) (12-bit Sub-Type) (16-bit Value)
    
        ¥ Long NOTE Events (8 bytes/event):
    
            (1 0 0 1) (12-bit Part) (1-bit UNUSED) (7-bit Pitch) (1-bit UNUSED) (7-bit Volume)
            (1 0) (8-bit UNUSED) (22-bit Duration)
        
        ¥ÊLong CONTROL Event (8 bytes/event):
        
            (1 0 1 0) (12-bit Part) (16-bit Value MSB) 
            (1 0) (14-bit Controller) (16-bit Value LSB)
    
        ¥ÊLong KNOB Event (8 bytes/event):
    
            (1 0 1 1) (12-bit Sub-Type) (16-bit Value MSB)
            (1 0) (14-bit KNOB) (16-bit Value LSB)
    
        ¥ÊVariable GENERAL Length Events (N bytes/event):
    
            (1 1 1 1) (12-bit Sub-Type) (16-bit Length)
                :
            (32-bit Data values)
                :
            (1 1) (14-bit UNUSED) (16-bit Length)
    
            where:  Length field is the number of LONG words in the record.
                    Lengths include the first and last long words (Minimum length = 2)
                
    The following event type values have not been used yet and are reserved for 
    future expansion:
        
        ¥ (1 0 0 0)     (8 bytes/event)
        ¥ (1 1 0 0)     (N bytes/event)
        ¥ (1 1 0 1)     (N bytes/event)
        ¥ (1 1 1 0)     (N bytes/event)
        
    For all events, the following generalizations apply:
    
        -   All duration values are specified in Millisecond units.
        -   Pitch values are intended to map directly to the MIDI key numbers.
        -   Controllers from 0 to 127 correspond to the standard MIDI controllers.
            Controllers greater than 127 correspond to other controls (i.e., Pitch Bend, 
            Key Pressure, and Channel Pressure).    
*/

