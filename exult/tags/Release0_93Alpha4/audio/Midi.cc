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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif



#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

#ifndef ALPHA_LINUX_CXX
#  include <csignal>
#  include <iostream>
#endif

#include "Midi.h"

#include <unistd.h>
#include "../fnames.h"
#include "../files/U7file.h"
#include "utils.h"
#include "xmidi.h"
#include "gamewin.h"
#include "game.h"
#include "conv.h"
#include "Audio.h"

#include "Configuration.h"
extern	Configuration	*config;

using std::cerr;
using std::cout;
using std::endl;
using std::string;


void    MyMidiPlayer::start_track(int num,bool repeat,int bank)
{
	if (!midi_device && !init_device())
	        return;


  #if DEBUG
        cout << "Audio subsystem request: Music track # " << num << endl;
  #endif
	// -1 and 255 are stop tracks
	if (num == -1 || num == 255)
	{
		midi_device->stop_track();
		return;
	}

	U7object	track(midi_bank[bank].c_str(),num);

//Not needed anymore
//#ifdef WIN32
//	//stop track before writing to temp. file
//	midi_device->stop_track();
//#endif
	
	char		*buffer;
	size_t		size;
	DataSource 	*mid_data;
	
	try
	{
		buffer = track.retrieve(size);
	}
	catch( const std::exception & err )
	{
		return;
	}

	// Read the data into the XMIDI class
	mid_data = new BufferDataSource(buffer, size);

	XMIDI		midfile(mid_data, music_conversion);
	
	delete mid_data;
	delete [] buffer;


	// Now get the data out of the XMIDI class and play it
		
	if (!midi_device->accepts_events())
		{
		FILE* mid_file = U7open(MIDITMPFILE, "wb");
		mid_data = new FileDataSource(mid_file);

		int can_play = midfile.retrieve(0, mid_data);
	
		delete mid_data;
		fclose(mid_file);

		if (can_play) midi_device->start_track(MIDITMPFILE,repeat);
		}
	else
		{
		midi_event	*eventlist;
		int		ppqn;
	
		if (midfile.retrieve(0, &eventlist, ppqn))
			midi_device->start_track(eventlist, ppqn, repeat);
		}

}

void    MyMidiPlayer::start_track(const char *fname,int num,bool repeat)
{
	if (!fname || (!midi_device && !init_device()))
	        return;

  #if DEBUG
        cout << "Audio subsystem request: Music track # " << num << " in file "<< fname << endl;
  #endif

	// -1 and 255 are stop tracks
	if (num == -1 || num == 255)
	{
		midi_device->stop_track();
		return;
	}

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

	XMIDI		midfile(mid_data, music_conversion);
	
	delete mid_data;
	fclose(mid_file);
	
	
	// Now get the data out of the XMIDI class and play it
	if (!midi_device->accepts_events())
		{
		mid_file = U7open(MIDITMPFILE, "wb");
		mid_data = new FileDataSource(mid_file);

		int can_play = midfile.retrieve(num, mid_data);
	
		delete mid_data;
		fclose(mid_file);

		if (can_play) midi_device->start_track(MIDITMPFILE,repeat);
		}
	else
		{
		midi_event	*eventlist;
		int		ppqn;
	
		if (midfile.retrieve(num, &eventlist, ppqn))
			midi_device->start_track(eventlist, ppqn, repeat);
		}
}

void    MyMidiPlayer::start_track(XMIDI *midfile, bool repeat)
{

  #if DEBUG
        cout << "Audio subsystem request: Custom Music track" << endl;
  #endif
	if (!midi_device && !init_device())
	        return;

	// Now get the data out of the XMIDI class and play it
	
	if (!midi_device->accepts_events())
		{	
		FILE* mid_file = U7open(MIDITMPFILE, "wb");
		DataSource *mid_data = new FileDataSource(mid_file);

		int can_play = midfile->retrieve(0, mid_data);
	
		delete mid_data;
		fclose(mid_file);

		if (can_play) midi_device->start_track(MIDITMPFILE,repeat);
	
		}
	else
		{
		midi_event	*eventlist;
		int		ppqn;
	
		if (midfile->retrieve(0, &eventlist, ppqn))
			midi_device->start_track(eventlist, ppqn, repeat);
		}
}

void	MyMidiPlayer::start_music(int num,bool repeat,int bank)
{
	if(!midi_device && !init_device())
		return;
	if(current_track==num&&midi_device->is_playing())
		return;	// Already playing it
	current_track=num;
	start_track(num,repeat,bank);
}

void	MyMidiPlayer::start_music(const char *fname,int num,bool repeat)
{
	if(!fname || (!midi_device && !init_device()))
		return;
	current_track=-1;
	start_track(fname,num,repeat);
}

void	MyMidiPlayer::stop_music()
{
	if(!midi_device && !init_device())
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
			cout << midi_device->copyright() << endl;	\
		} catch(...) {	\
			no_device=true;	\
		}	\
	}
	
void MyMidiPlayer::set_music_conversion(int conv)
{
	music_conversion = conv;
	
	switch(music_conversion) {
	case XMIDI_CONVERT_MT32_TO_GS:
		config->set("config/audio/midi/convert","gs",true);
		break;
	case XMIDI_CONVERT_NOCONVERSION:
		config->set("config/audio/midi/convert","none",true);
		break;
	case XMIDI_CONVERT_MT32_TO_GS127:
		config->set("config/audio/midi/convert","gs127",true);
		break;
	default:
		config->set("config/audio/midi/convert","gm",true);
		break;
	}
}

void MyMidiPlayer::set_effects_conversion(int conv)
{
	effects_conversion = conv;
	
	switch(effects_conversion) {
	case XMIDI_CONVERT_NOCONVERSION:
		config->set("config/audio/effects/convert","none",true);
		break;
	default:
		config->set("config/audio/effects/convert","gs",true);
		break;
	}
}

bool MyMidiPlayer::init_device(void)
{
	bool	no_device=true;

	// instrument_patches=AccessTableFile(XMIDI_MT);
	string	s;

	bool sfx = Audio::get_ptr()->are_effects_enabled();

	if (!sfx) s = "no";
	else s = "yes";

	config->set("config/audio/effects/enabled",s,true);

	bool music = Audio::get_ptr()->is_music_enabled();

	if (!music) s = "no";
	else s = "yes";

	config->set("config/audio/midi/enabled",s,true);


	config->value("config/audio/midi/convert",s,"gm");

	if (s == "gs")
		music_conversion = XMIDI_CONVERT_MT32_TO_GS;
	else if (s == "none")
		music_conversion = XMIDI_CONVERT_NOCONVERSION;
	else if (s == "gs127")
		music_conversion = XMIDI_CONVERT_MT32_TO_GS127;
	else if (s == "gs127drum")
	{
		music_conversion = XMIDI_CONVERT_MT32_TO_GS;
		config->set("config/audio/midi/convert","gs",true);
	}
	else
	{
		music_conversion = XMIDI_CONVERT_MT32_TO_GM;
		config->set("config/audio/midi/convert","gm",true);
	}

	config->value("config/audio/effects/convert",s,"gs");

	if (s == "none")
		effects_conversion = XMIDI_CONVERT_NOCONVERSION;
	else if (s == "gs127")
		effects_conversion = XMIDI_CONVERT_NOCONVERSION;
	else
	{
		effects_conversion = XMIDI_CONVERT_GS127_TO_GS;
		config->set("config/audio/effects/convert","gs",true);
	}

	// already initialized?
	if (initialized)
		return (midi_device != 0);

	// no need for a MIDI device (for now)
	if (!sfx && !music)
	{
		midi_device = 0;
		return false;
	}

#ifdef WIN32
//	TRY_MIDI_DRIVER(Windows_MCI)
	TRY_MIDI_DRIVER(Windows_MidiOut)
#endif
#ifdef BEOS
	TRY_MIDI_DRIVER(Be_midi)
#endif
#if defined(HAVE_TIMIDITY_BIN) && (defined(XWIN) || defined(WIN32))
	TRY_MIDI_DRIVER(Timidity_binary)
#endif
#if HAVE_LIBKMIDI
	TRY_MIDI_DRIVER(KMIDI)
#endif
#if (defined(XWIN) && !defined(OPENBSD))
	TRY_MIDI_DRIVER(forked_player)
#endif
#ifdef MACOS
	TRY_MIDI_DRIVER(Mac_QT_midi)
#endif

	initialized = true;

	if(no_device)
	{
		midi_device=0;
		cerr << "Unable to create a MIDI device. No music will be played." << endl;
		return false;
	}
		
	return true;
}

MyMidiPlayer::MyMidiPlayer()	: current_track(-1),midi_device(0),
				  initialized(false),
				  music_conversion(XMIDI_CONVERT_MT32_TO_GM),
				  effects_conversion(XMIDI_CONVERT_GS127_TO_GS)
{
	add_midi_bank(MAINMUS);
	add_midi_bank(INTROMUS);
	add_midi_bank("<STATIC>/mainshp.flx");
	
	init_device();
}

MyMidiPlayer::~MyMidiPlayer()
{
	if(midi_device)//&&midi_device->is_playing())
		//midi_device->stop_track();
		delete midi_device;
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

	U7object	track("<DATA>/midisfx.flx",real_num);

	if (!midi_device && !init_device())
	        return;
	
	char		*buffer;
	size_t		size;
	DataSource 	*mid_data;
	
	try
	{
		buffer = track.retrieve(size);
	}
	catch( const std::exception & err )
	{
		return;
	}

	// Read the data into the XMIDI class
	mid_data = new BufferDataSource(buffer, size);

	// It's already GM, so dont convert
	XMIDI		midfile(mid_data, effects_conversion);
	
	delete mid_data;
	delete [] buffer;

	// Now get the data out of the XMIDI class and play it
	if (!midi_device->accepts_events())
		{
		FILE *mid_file = U7open(MIDISFXFILE, "wb");
		mid_data = new FileDataSource(mid_file);

		int can_play = midfile.retrieve(0, mid_data);
	
		delete mid_data;
		fclose(mid_file);

		if (can_play) midi_device->start_sfx(MIDISFXFILE);
		}
	else
		{
		midi_event	*eventlist;
		int		ppqn;
	
		if (midfile.retrieve(0, &eventlist, ppqn))
			midi_device->start_sfx(eventlist, ppqn);
		}
}

void    MyMidiPlayer::stop_sound_effects()
{
	if (midi_device)
		midi_device->stop_sfx();
}
