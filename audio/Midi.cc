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


#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

#include "Midi.h"

#include <unistd.h>
#include <csignal>
#include <iostream>
#include "../fnames.h"
#include "../files/U7file.h"
#include "utils.h"
#include "xmidi.h"
#include "gamewin.h"
#include "game.h"
#include "conv.h"

#include "Configuration.h"
extern	Configuration	*config;


void    MyMidiPlayer::start_track(int num,bool repeat,int bank)
{
  #if DEBUG
        cout << "Audio subsystem request: Music track # " << num << endl;
  #endif
	U7object	track(midi_bank[bank].c_str(),num);

	if (!midi_device)
	        return;

//Not needed anymore
//#ifdef WIN32
//	//stop track before writing to temp. file
//	midi_device->stop_track();
//#endif
	
	char		*buffer;
	size_t		size;
	DataSource 	*mid_data;
	
	if(!track.retrieve(&buffer, size))
	        return;

	// Read the data into the XMIDI class
	mid_data = new BufferDataSource(buffer, size);

	XMIDI		midfile(mid_data);
	
	delete mid_data;
	delete [] buffer;


	// Now get the data out of the XMIDI class and play it
	
#ifndef WIN32
	
	FILE* mid_file = U7open(MIDITMPFILE, "wb");
	mid_data = new FileDataSource(mid_file);

	int can_play = midfile.retrieve(0, mid_data);
	
	delete mid_data;
	fclose(mid_file);

	if (can_play) midi_device->start_track(MIDITMPFILE,repeat);
	
#else
	midi_event	*eventlist;
	int		ppqn;
	
	if (midfile.retrieve(0, &eventlist, ppqn))
		midi_device->start_track(eventlist, ppqn, repeat);
#endif
}

void    MyMidiPlayer::start_track(const char *fname,int num,bool repeat)
{
  #if DEBUG
        cout << "Audio subsystem request: Music track # " << num << " in file "<< fname << endl;
  #endif

	if (!midi_device || !fname)
	        return;

// Not Needed anymore
//#ifdef WIN32
//	//stop track before writing to temp. file
//	midi_device->stop_track();
//#endif
	        
	FILE		*mid_file;
	DataSource	*mid_data;
	

	// Read the data into the XMIDI class

	mid_file = U7open(fname, "rb");
	mid_data = new FileDataSource(mid_file);

	XMIDI		midfile(mid_data);
	
	delete mid_data;
	fclose(mid_file);
	
	
	// Now get the data out of the XMIDI class and play it
#ifndef WIN32
	mid_file = U7open(MIDITMPFILE, "wb");
	mid_data = new FileDataSource(mid_file);

	int can_play = midfile.retrieve(num, mid_data);
	
	delete mid_data;
	fclose(mid_file);

	if (can_play) midi_device->start_track(MIDITMPFILE,repeat);
#else
	midi_event	*eventlist;
	int		ppqn;
	
	if (midfile.retrieve(num, &eventlist, ppqn))
		midi_device->start_track(eventlist, ppqn, repeat);
#endif
}

void	MyMidiPlayer::start_music(int num,bool repeat,int bank)
{
	if(!midi_device)
		return;
	if(current_track==num&&midi_device->is_playing())
		return;	// Already playing it
	current_track=num;
	start_track(num,repeat,bank);
}

void	MyMidiPlayer::start_music(const char *fname,int num,bool repeat)
{
	if(!midi_device || !fname)
		return;
	current_track=-1;
	start_track(fname,num,repeat);
}

void	MyMidiPlayer::stop_music()
{
	if(!midi_device)
		return;
	
	midi_device->stop_track();
	current_track=-1;
}

bool	MyMidiPlayer::add_midi_bank(const char *bankname)
{
	string	bank(bankname);
	midi_bank.push_back(bank);
	return true;
}


#if HAVE_TIMIDITY_BIN
  #include "midi_drivers/Timidity_binary.h"
#endif
#if HAVE_LIBKMIDI
  #include "midi_drivers/KMIDI.h"
#endif
#ifndef MACOS
  #include "midi_drivers/forked_player.h"
#endif
#ifdef WIN32
//  #include "midi_drivers/win_MCI.h"
  #include "midi_drivers/win_midiout.h"
#endif
#ifdef BEOS
  #include "midi_drivers/be_midi.h"
#endif
#ifdef MACOS
  #include "midi_drivers/mac_midi.h"
#endif

#define	TRY_MIDI_DRIVER(CLASS_NAME)	\
	if(no_device) {	\
		try {	\
			midi_device=new CLASS_NAME();	\
			no_device=false;	\
			cerr << midi_device->copyright() << endl;	\
		} catch(...) {	\
			no_device=true;	\
		}	\
	}

MyMidiPlayer::MyMidiPlayer()	: current_track(-1),midi_device(0)
{
	bool	no_device=true;

	add_midi_bank(MAINMUS);
	add_midi_bank(INTROMUS);
	add_midi_bank("static/mainshp.flx");

	// instrument_patches=AccessTableFile(XMIDI_MT);
	string	s;
	config->value("config/audio/midi/enabled",s,"---");
	if(s=="---")
		{
		cout << "Config does not specify MIDI. Assuming yes" << endl;
		s="yes";
		}
	if(s=="no")
		{
		cout << "Config says no midi. MIDI disabled" << endl;
		no_device=false;
		}
	config->set("config/audio/midi/enabled",s,true);

#ifdef WIN32
//	TRY_MIDI_DRIVER(Windows_MCI)
	TRY_MIDI_DRIVER(Windows_MidiOut)
#endif
#ifdef BEOS
	TRY_MIDI_DRIVER(Be_midi)
#endif
#if HAVE_TIMIDITY_BIN && defined(XWIN)
	TRY_MIDI_DRIVER(Timidity_binary)
#endif
#if HAVE_LIBKMIDI
	TRY_MIDI_DRIVER(KMIDI)
#endif
#ifdef XWIN
	TRY_MIDI_DRIVER(forked_player)
#endif
#ifdef MACOS
	TRY_MIDI_DRIVER(Mac_QT_midi)
#endif

	if(no_device)
		{
		midi_device=0;
		cerr << "Unable to create a music device. No music will be played" << endl;
		}
}

MyMidiPlayer::~MyMidiPlayer()
{
	if(midi_device&&midi_device->is_playing())
		midi_device->stop_track();
}


void    MyMidiPlayer::start_sound_effect(int num)
{
  #if DEBUG
        cout << "Audio subsystem request: sound effect # " << num << endl;
  #endif
        int real_num = num;

        if (Game::get_game_type() == BLACK_GATE)
        	real_num = bgconv[num];
        
        cout << "Real num " << real_num << endl;

	U7object	track("sfx.dat",real_num);

	if (!midi_device)
	        return;

//Not needed anymore
//#ifdef WIN32
//	//stop track before writing to temp. file
//	midi_device->stop_track();
//#endif
	
	char		*buffer;
	size_t		size;
	DataSource 	*mid_data;
	
	if(!track.retrieve(&buffer, size))
	        return;

	// Read the data into the XMIDI class
	mid_data = new BufferDataSource(buffer, size);

	// It's already GM, so dont convert
	XMIDI		midfile(mid_data, false);
	
	delete mid_data;
	delete [] buffer;

	// Now get the data out of the XMIDI class and play it
	
#ifdef WIN32
	midi_event	*eventlist;
	int		ppqn;
	
	if (midfile.retrieve(0, &eventlist, ppqn))
		midi_device->start_sfx(eventlist, ppqn);
#endif
}

