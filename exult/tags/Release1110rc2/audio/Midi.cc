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


#ifndef ALPHA_LINUX_CXX
#  include <csignal>
#  include <iostream>
#endif
#include <unistd.h>

#include "fnames.h"
#include "exult.h"
#include "game.h"
#include "Audio.h"

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

	if (output_driver_type == MIDI_DRIVER_OGG)
	{
	    char filename[255] ;
		string s;

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

			s = midi_bank[bank];
			to_uppercase(s);
			if((num == 30 || num == 32) && s.find("MAINSHP") != std::string::npos)
			{
				//Convert credit/quote tracks
				if(num == 30)
					s = "endcr01.ogg";
				else
					s = "endcr02.ogg";
			}
			else
			{
				s = bgconvmusic[num];
				s = s + ".ogg";
			}

			s = get_system_path("<MUSIC>/" + s);
			strcpy(filename, s.c_str());
		}
		else
		{
			char outputstr[255];

			s = midi_bank[bank];
			to_uppercase(s);
			if((num == 4 || num == 5) && s.find("INTRORDM") != std::string::npos)
			{
				//Convert credit/quotes tracks
				if(num == 4)
					s = "endcr01.ogg";
				else
					s = "endcr02.ogg";
			}
			else
				s = "%02dbg.ogg";

			s = get_system_path("<MUSIC>/" + s);
			strcpy(outputstr, s.c_str());
			sprintf(filename, outputstr, num);
		}

		if(num == 99)		//Play the Exult theme tune
		{
			s = get_system_path("<MUSIC>/exult.ogg");
			strcpy(filename, s.c_str());
		}

		if(repeat)
			repeat = 2;		//Convert repeats to repeat 2 times only

	    oggmusic = Mix_LoadMUS(filename);
		if (!oggmusic) {
			cout << "Error playing " << filename << ": " << Mix_GetError()
				 << std::endl;
		}

		Mix_PlayMusic(oggmusic, repeat);
		Mix_VolumeMusic(MIX_MAX_VOLUME);

#ifdef DEBUG
       	cout << "OGG audio: Music track " << filename << endl;
#endif

		return;		//We don't want to continue with Midi conversions!!
	}

	// Bank 0 is mainmus, but it's for mt32. If our hardware is an fm synth,
	// use the fmsynth music, which is bank 3
	if (bank == 0 && midi_device->is_fm_synth()) bank = 3;
	// Bank 1 is BG menu/intro. We need Bank 4
	else if (bank == 1 && midi_device->is_fm_synth()) bank = 4;
	// Bank 2 is SI menu/intro. We need to offset -1
	else if (bank == 2 && midi_device->is_fm_synth()) num--;

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

	// Play with the conversion type for the FMSynth or MT32EMU drivers
	int conv = music_conversion;

	if (midi_device->use_gs127())
	//if (midi_device->is_fmsynth())
		conv = XMIDI_CONVERT_MT32_TO_GS127;
	//else if (midi_device->is_mt32())
		//conv = XMIDI_CONVERT_NOCONVERSION;

	XMIDI		midfile(mid_data, conv);
	
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
	if (output_driver_type == MIDI_DRIVER_OGG)
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
		s2 = get_system_path("<MUSIC>/" + s2);
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

	// Play with the conversion type for the FMSynth or MT32EMU drivers
	int conv = music_conversion;

	if (midi_device->use_gs127())
	//if (midi_device->is_fmsynth())
		conv = XMIDI_CONVERT_MT32_TO_GS127;
	//else if (midi_device->is_mt32())
		//conv = XMIDI_CONVERT_NOCONVERSION;

	XMIDI		midfile(mid_data, conv);
	
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

	if(num == 0 && bank == 0 && Game::get_game_type() == BLACK_GATE)
		return;		//Gets around Usecode bug where track 0 is played at Intro Earthquake

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
#ifdef USE_FMOPL_MIDI
  #include "midi_drivers/fmopl_midi.h"
#endif
#ifdef WIN32
  #include "midi_drivers/win_midiout.h"
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

#define	TRY_MIDI_DRIVER(CLASS_NAME)	do { \
	if(no_device) {	\
		try {	\
			midi_device=new CLASS_NAME();	\
			no_device=false;	\
			cout << "Music player: " << midi_device->copyright() << endl;	\
		} catch(...) {	\
			no_device=true;	\
		}	\
	}	\
} while(0)
	
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

void MyMidiPlayer::set_output_driver_type(int type)
{
	// Don't kill the device if we don't need to
	if (type != output_driver_type) {
		stop_music();
		delete midi_device;
		midi_device = 0;
		initialized = false;
	}

	output_driver_type = type;

	switch(output_driver_type) {
	case MIDI_DRIVER_OGG:
		config->set("config/audio/midi/driver","digital",true);
		break;
#ifdef USE_FMOPL_MIDI
	case MIDI_DRIVER_FMSYNTH:
		config->set("config/audio/midi/driver","fmsynth",true);
		break;
#endif
#ifdef USE_MT32EMU_MIDI
	case MIDI_DRIVER_MT32EMU:
		config->set("config/audio/midi/driver","mt32emu",true);
		break;
#endif
	default:
		config->set("config/audio/midi/driver","normal",true);
		break;
	};
}


bool MyMidiPlayer::init_device(void)
{
	// already initialized? Do this first
	if (initialized) return (midi_device != 0);

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

	// Global Midi Enable/Disable
	config->set("config/audio/midi/enabled",s,true);

	// String for default value for driver type
	std::string driver_default = "normal";

	// Music conversion
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
		driver_default = s;
	}

	// Effects conversion
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

	// Midi driver type.
	config->value("config/audio/midi/driver",s,driver_default.c_str());

	if (s == "digital")
	{
		output_driver_type = MIDI_DRIVER_OGG;
		config->set("config/audio/effects/driver","digital",true);
	}
#ifdef USE_FMOPL_MIDI
	else if (s == "fmsynth")
	{
		output_driver_type = MIDI_DRIVER_FMSYNTH;
		config->set("config/audio/effects/driver","fmsynth",true);
	}
#endif
#ifdef USE_MT32EMU_MIDI
	else if (s == "mt32emu")
	{
		output_driver_type = MIDI_DRIVER_MT32EMU;
	}
#endif
	else
	{
		output_driver_type = MIDI_DRIVER_NORMAL;
		config->set("config/audio/effects/driver","normal",true);
	}

	// no need for a MIDI device (for now)
	if (!sfx && !music)
	{
		midi_device = 0;
		return false;
	}


	// OGG is handled differently to the other MIDI devices, due to it
	// not actually being a midi device. There are hacks all over the place 
	// for it. The OGG_MIDI driver does do some 'stuff', such as init
	if (output_driver_type == MIDI_DRIVER_OGG)
		TRY_MIDI_DRIVER(OGG_MIDI);
#ifdef USE_FMOPL_MIDI
	else if (output_driver_type == MIDI_DRIVER_FMSYNTH)
		TRY_MIDI_DRIVER(FMOpl_Midi);
#endif

#ifdef WIN32
	TRY_MIDI_DRIVER(Windows_MidiOut);
#endif
#ifdef BEOS
	TRY_MIDI_DRIVER(Be_midi);
#endif
#if defined(HAVE_TIMIDITY_BIN) && (defined(XWIN) || defined(WIN32))
	TRY_MIDI_DRIVER(Timidity_binary);
#endif
#if HAVE_LIBKMIDI
	TRY_MIDI_DRIVER(KMIDI);
#endif
#if (defined(XWIN) && !defined(OPENBSD) && !defined(__zaurus__))
	TRY_MIDI_DRIVER(forked_player);
#endif
#if defined(MACOS) || defined(MACOSX)
	TRY_MIDI_DRIVER(Mac_QT_midi);
#endif
#if defined(__MORPHOS__) || defined(AMIGA)
	TRY_MIDI_DRIVER(AmigaMIDI);
#endif
#ifdef USE_FMOPL_MIDI
	TRY_MIDI_DRIVER(FMOpl_Midi);
#endif

	initialized = true;

	if(no_device)
	{
		midi_device=0;
		cerr << "Unable to create a MIDI device. No music will be played." << endl;
		return false;
	}

	load_patches(false);
		
	return true;
}


MyMidiPlayer::MyMidiPlayer()	: current_track(-1),repeating(false),
				  midi_device(0), initialized(false),
				  music_conversion(XMIDI_CONVERT_MT32_TO_GM),
				  effects_conversion(XMIDI_CONVERT_GS127_TO_GS),
				  output_driver_type(MIDI_DRIVER_NORMAL)
{
	add_midi_bank(MAINMUS);
	add_midi_bank(INTROMUS);
	add_midi_bank("<STATIC>/mainshp.flx");
	add_midi_bank(MAINMUS_AD);
	add_midi_bank(INTROMUS_AD);
	
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

	if (Game::get_game_type() == BLACK_GATE) real_num = bgconv[num];

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

OGG_MIDI::OGG_MIDI()
{
	oggmusic = NULL;
	Mix_HookMusicFinished(music_complete_callback);
	Mix_VolumeMusic(MIX_MAX_VOLUME);
}

OGG_MIDI::~OGG_MIDI()
{
	Mix_HaltMusic();
	if(oggmusic)
	{
		Mix_FreeMusic(oggmusic);
		oggmusic = NULL;
	}
}

//Clean up last track played, freeing memory each time
void OGG_MIDI::music_complete_callback(void)
{
	if(oggmusic)
	{
		Mix_FreeMusic(oggmusic);
		oggmusic = NULL;
	}
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

bool OGG_MIDI::is_playing(void)
{
	return Mix_PlayingMusic()!=0;
}

const char * OGG_MIDI::copyright(void)
{
	return "Internal OGG digital music player";
}


