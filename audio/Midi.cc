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
#include <unistd.h>

#ifndef PENTAGRAM
#include "fnames.h"
#include "exult.h"
#include "game.h"
#include "Audio.h"
#endif

#include "../files/U7file.h"
#include "../files/utils.h"
#include "Midi.h"
#include "xmidi.h"
#include "conv.h"
#include "convmusic.h"

#include "../conf/Configuration.h"
extern	Configuration	*config;

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::strcpy;


static Mix_Music *oggmusic;


void    MyMidiPlayer::start_track(int num,bool repeat,int bank)
{
	if (!midi_device && !init_device())
	        return;


  #ifdef DEBUG
        cout << "Audio subsystem request: Music track # " << num << endl;
  #endif
	// -1 and 255 are stop tracks
	if (num == -1 || num == 255)
	{
		stop_music();
		return;
	}

	current_track = num;
	repeating = repeat;

	if (music_conversion == XMIDI_CONVERT_OGG)
	{
	    char filename[255] ;

		//Free previous music, may not have been properly stopped
		if(oggmusic)
		{
			Mix_FreeMusic(oggmusic);
			oggmusic = NULL;
		}


		if(Game::get_game_type()==SERPENT_ISLE)
		{
			if(bank == 2 && num == 28)	//Convert title track number only in SI
				num = 70;

			string s = midi_bank[bank];
			to_uppercase(s);
			if((num == 30 || num == 32) && s.find("MAINSHP") != std::string::npos)
			{
				//Convert credit/quote tracks
				if(num == 30)
					s = get_system_path("<DATA>/mp3/") + "endcr01.ogg";
				else
					s = get_system_path("<DATA>/mp3/") + "endcr02.ogg";
			}
			else
				s = get_system_path("<DATA>/mp3/") + bgconvmusic[num] + ".ogg";

			strcpy(filename, s.c_str());
		}
		else
		{
			char outputstr[255];

			string s = midi_bank[bank];
			to_uppercase(s);
			if((num == 4 || num == 5) && s.find("INTRORDM") != std::string::npos)
			{
				//Convert credit/quotes tracks
				if(num == 4)
					s = get_system_path("<DATA>/mp3/") + "endcr01.ogg";
				else
					s = get_system_path("<DATA>/mp3/") + "endcr02.ogg";
			}
			else
				s = get_system_path("<DATA>/mp3/%02dbg.ogg");

			strcpy(outputstr, s.c_str());
			sprintf(filename, outputstr, num);
		}

		if(repeat)
			repeat = 2;		//Convert repeats to repeat 2 times only
	    oggmusic = Mix_LoadMUS(filename);
	    Mix_PlayMusic(oggmusic, repeat);
		Mix_VolumeMusic(MIX_MAX_VOLUME);

#ifdef DEBUG
       	cout << "Audio OGG: Music track " << filename << endl;
#endif

		return;		//We don't want to continue with Midi conversions!!
	}

	U7object	track(midi_bank[bank].c_str(),num);

	char		*buffer;
	size_t		size;
	DataSource 	*mid_data;
	
	try
	{
		buffer = track.retrieve(size);
	}
	catch( const std::exception & /*err*/ )
	{
		return;
	}

	// Read the data into the XMIDI class
	mid_data = new BufferDataSource(buffer, size);

	XMIDI		midfile(mid_data, music_conversion);
	
	delete mid_data;
	delete [] buffer;

	// Now give the xmidi object to the midi device

	XMIDIEventList *eventlist = midfile.GetEventList(0);
	if (eventlist) midi_device->start_track(eventlist, repeat);

}

void    MyMidiPlayer::start_track(const char *fname,int num,bool repeat)
{
	if (!fname || (!midi_device && !init_device()))
	        return;

	current_track = -1;
	repeating = repeat;

  #ifdef DEBUG
        cout << "Audio subsystem request: Music track # " << num << " in file "<< fname << endl;
  #endif

	// -1 and 255 are stop tracks
	if (num == -1 || num == 255)
	{
		midi_device->stop_track();
		return;
	}

	//Only called from the endgame sequences  
	if (music_conversion == XMIDI_CONVERT_OGG)
	{
	    char filename[255] ;

		//Free previous music, may not have been properly stopped
		if(oggmusic)
		{
			Mix_FreeMusic(oggmusic);
			oggmusic = NULL;
		}

		string s = fname;
		to_uppercase(s);

		if(s.find("ENDSCORE") != std::string::npos && Game::get_game_type()!=SERPENT_ISLE)
		{
		    sprintf(filename, "end%02dbg.ogg", num);
		}
		else if(s.find("R_SINTRO") != std::string::npos && Game::get_game_type()==SERPENT_ISLE)
			strcpy(filename, "si01.ogg");		//SI introduction sequence
		else if(s.find("R_SEND") != std::string::npos && Game::get_game_type()==SERPENT_ISLE)
			strcpy(filename, "si13.ogg");		//SI end sequence
		else
			return;

		string s2 = filename;
		s2 = get_system_path("<DATA>/mp3/" + s2);
		strcpy(filename, s2.c_str());

	    oggmusic = Mix_LoadMUS(filename);
	    Mix_PlayMusic(oggmusic, repeat);
		Mix_VolumeMusic(MIX_MAX_VOLUME);

		return;		//We don't want to continue with Midi conversions!!
	}


	FILE		*mid_file;
	DataSource	*mid_data;
	

	// Read the data into the XMIDI class

	mid_file = U7open(fname, "rb");  //DARKE FIXME
	mid_data = new FileDataSource(mid_file);

	XMIDI		midfile(mid_data, music_conversion);
	
	delete mid_data;
	fclose(mid_file);
	
	// Now give the xmidi object to the midi device
	XMIDIEventList *eventlist = midfile.GetEventList(num);
	if (eventlist) midi_device->start_track(eventlist, repeat);
}

void    MyMidiPlayer::start_track(XMIDIEventList *midfile, bool repeat)
{

  #ifdef DEBUG
        cout << "Audio subsystem request: Custom Music track" << endl;
  #endif
	if (!midi_device && !init_device())
	        return;
	
	// Now give the xmidi object to the midi device
	midi_device->start_track(midfile, repeat);
}

void	MyMidiPlayer::start_music(int num,bool repeat,int bank)
{
	if(!midi_device && !init_device())
		return;
	if(current_track==num&&midi_device->is_playing())
		return;	// Already playing it
	start_track(num,repeat,bank);
}

void	MyMidiPlayer::start_music(const char *fname,int num,bool repeat)
{
	if(!fname || (!midi_device && !init_device()))
		return;
	start_track(fname,num,repeat);
}

void	MyMidiPlayer::stop_music()
{
	if(!midi_device && !init_device())
		return;

	current_track = -1;
	repeating = false;
	
	midi_device->stop_track();
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
#if !defined(MACOS) && !defined(WIN32)
  #include "midi_drivers/forked_player.h"
#endif
#ifdef WIN32
  #include "midi_drivers/mixer_midiout.h"
#endif
#ifdef BEOS
  #include "midi_drivers/be_midi.h"
#endif
#if defined(MACOS) || defined(MACOSX)
  #include "midi_drivers/mac_midi.h"
#endif
#if defined( __MORPHOS__ ) || defined( AMIGA )
	#include "midi_drivers/amiga_midi.h"
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
	case XMIDI_CONVERT_OGG:
		config->set("config/audio/midi/convert","digital",true);
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

#ifndef PENTAGRAM
	bool sfx = Audio::get_ptr()->are_effects_enabled();
#else
	config->value("config/audio/effects/enabled",s,"no");
	bool sfx = s!="yes";
#endif

	if (!sfx) s = "no";
	else s = "yes";

	config->set("config/audio/effects/enabled",s,true);

#ifndef PENTAGRAM
	bool music = Audio::get_ptr()->is_music_enabled();
#else
	config->value("config/audio/midi/enabled",s,"yes");
	bool music = s!="no";
#endif

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
	else if (s == "digital")
		music_conversion = XMIDI_CONVERT_OGG;
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


	//OGG is initialised differently to the other MIDI devices, due to it
	//not actually being a midi device. Just set the midi_device to something
	//to stop the other code breaking, much is dependant on this class existing
	if (music_conversion == XMIDI_CONVERT_OGG)
	{
		midi_device=new OGG_MIDI();
		no_device=false;       
		oggmusic = NULL;
		Mix_HookMusicFinished(music_complete_callback);
		Mix_VolumeMusic(MIX_MAX_VOLUME);
	}
	else
	{
#ifdef WIN32
	TRY_MIDI_DRIVER(Mixer_MidiOut)
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
#if defined(MACOS) || defined(MACOSX)
	TRY_MIDI_DRIVER(Mac_QT_midi)
#endif
#if defined(__MORPHOS__) || defined(AMIGA)
  TRY_MIDI_DRIVER(AmigaMIDI)
#endif
	}

	initialized = true;

	if(no_device)
	{
		midi_device=0;
		cerr << "Unable to create a MIDI device. No music will be played." << endl;
		return false;
	}
		
	return true;
}


//Clean up last track played, freeing memory each time
void MyMidiPlayer::music_complete_callback(void)
{
	if(oggmusic)
	{
		Mix_FreeMusic(oggmusic);
		oggmusic = NULL;
	}
}


MyMidiPlayer::MyMidiPlayer()	: current_track(-1),repeating(false),
				  midi_device(0), initialized(false),
				  music_conversion(XMIDI_CONVERT_MT32_TO_GM),
				  effects_conversion(XMIDI_CONVERT_GS127_TO_GS)
{
#ifndef PENTAGRAM
	add_midi_bank(MAINMUS);
	add_midi_bank(INTROMUS);
	add_midi_bank("<STATIC>/mainshp.flx");
#endif
	
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
  #ifdef DEBUG
        cout << "Audio subsystem request: sound effect # " << num << endl;
  #endif
        int real_num = num;

#ifndef PENTAGRAM
        if (Game::get_game_type() == BLACK_GATE)
        	real_num = bgconv[num];
#endif        
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
	catch( const std::exception & /*err*/ )
	{
		return;
	}

	// Read the data into the XMIDI class
	mid_data = new BufferDataSource(buffer, size);

	// It's already GM, so dont convert
	XMIDI		midfile(mid_data, effects_conversion);
	
	delete mid_data;
	delete [] buffer;

	// Now give the xmidi object to the midi device
	XMIDIEventList *eventlist = midfile.GetEventList(0);
	if (eventlist) midi_device->start_sfx(eventlist);
	
}

void    MyMidiPlayer::stop_sound_effects()
{
	if (midi_device)
		midi_device->stop_sfx();
}


void OGG_MIDI::start_track(XMIDIEventList *, bool repeat)
{
}

void OGG_MIDI::stop_track(void)
{
	Mix_HaltMusic();
	if(oggmusic)
	{
		Mix_FreeMusic(oggmusic);
		oggmusic = NULL;
	}
}

void OGG_MIDI::start_sfx(XMIDIEventList *)
{
}

void OGG_MIDI::stop_sfx(void)
{
}

bool OGG_MIDI::is_playing(void)
{
	return Mix_PlayingMusic();
}

const char * OGG_MIDI::copyright(void)
{
  return "Internal OGG NULL device";
}


