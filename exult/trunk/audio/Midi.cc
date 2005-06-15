/*
Copyright (C) 2000-2005  The Exult Team

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

// Includes Pentagram headers so we must include pent_include.h 
#include "pent_include.h"

#ifndef ALPHA_LINUX_CXX
#ifndef UNDER_CE
#  include <csignal>
#endif
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
#include "XMidiFile.h"
#include "MidiDriver.h"
#include "conv.h"
#include "databuf.h"
#include "convmusic.h"

#include "data/exult_flx.h"

#include "../conf/Configuration.h"
extern	Configuration	*config;

#ifndef UNDER_CE
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::strcpy;
#endif

//
// Midi devices types and conversions
//
// The midi player supports the following types of devices:
// Hardware GM/GS assumed (i.e. Windows Midi, Core Audio Midi)
// Hardware MT32 forced (MT32 connected to external Hardware midi port)
// Software GM/GS (Timidty)
// Software MT32 (MT32Emu)
// Software FMSynth (FMOpl)
//
// Hardware Midi Device can be forced into MT32 mode by setting the Converstion
// type to None.
//
// Midi Conversion setting is ignored devices that return true to
// isMT32Device() or isFMSynth(). They are assumed to properly support
// loadTimbreLibrary();
//
// Additionally as an Augmentation, we support Playing of Tracks as
// OGG Vorbis files
//
// The player supports the following states and does the following:
//
// General Midi Timbre Library:
// GM/GS Device: Do nothing
// MT32/FMSynth: Load GM Timbre Set into the MT32.
//
// Intro/Game/Endgame Timbre Library:
// GM/GS Device: Convert notes using XMIDI Convert setting
// MT32/FMSynth: Load correct Timbre Library from Game Data
//
// 

#define SEQ_NUM_MUSIC	0
#define SEQ_NUM_SFX		1

void	MyMidiPlayer::start_music(int num,bool repeat,std::string flex)
{
	// No output device
	if(!ogg_enabled && !midi_driver && !init_device())
		return;

	// -1 and 255 are stop tracks
	if (num == -1 || num == 255) {
		stop_music();
		return;
	}

	// Already playing it??
	if(current_track==num) {
		// OGG is playing?
		if (ogg_enabled && ogg_is_playing())
			return;
		// Midi driver is playing?
		if (midi_driver && midi_driver->isSequencePlaying(SEQ_NUM_MUSIC))
			return;	
	}

	// Work around Usecode bug where track 0 is played at Intro Earthquake
	if(num == 0 && flex == MAINMUS && Game::get_game_type() == BLACK_GATE)
		return;		

  #ifdef DEBUG
        cout << "Audio subsystem request: Music track # " << num << " in "<< flex << endl;
  #endif

	stop_music();

	current_track = num;
	repeating = repeat;

	// OGG Handling
	if (ogg_enabled) {

		// Play ogg for this track
		if (ogg_play_track(flex,num,repeat))
			return;

		// If we failed to play the track, call stop to clean up and put us back into
		// midi synth mode
		ogg_stop_track();

		// No midi driver so don't fall through
		if (!midi_driver) return;
	}

	// Handle FM Synth
	if (midi_driver->isFMSynth())  {

		// use the fmsynth music, which is bank 3
		if (flex == MAINMUS) flex = MAINMUS_AD;
		// Bank 1 is BG menu/intro. We need Bank 4
		else if (flex == INTROMUS) flex = INTROMUS_AD;
		// Bank 2 is SI menu. We need to offset -1
		else if (flex == MAINSHP_FLX) num--;
	}

	DataSource 	*mid_data;
	
	try {
		mid_data = new ExultDataSource(flex.c_str(),num);
	}
	catch( const std::exception & /*err*/ ) {
		return;
	}

	XMidiFile midfile(mid_data, setup_timbre_for_track(flex));
	
	delete mid_data;

	// Now give the xmidi object to the midi device

	XMidiEventList *eventlist = midfile.GetEventList(0);
	if (eventlist)  midi_driver->startSequence(SEQ_NUM_MUSIC, eventlist, repeat, 255);
}

void	MyMidiPlayer::start_music(std::string fname,int num,bool repeat)
{
	// No output device
	if(!ogg_enabled && !midi_driver && !init_device())
		return;

	stop_music();

	// -1 and 255 are stop tracks
	if (num == -1 || num == 255) return;

	current_track = -1;
	repeating = repeat;

  #ifdef DEBUG
        cout << "Audio subsystem request: Music track # " << num << " in file "<< fname << endl;
  #endif

	// OGG Handling
	if (ogg_enabled) {

		// Play ogg for this track
		if (ogg_play_track(fname,num,repeat))
			return;

		// If we failed to play the track, call stop to clean up and put us back into
		// midi synth mode
		ogg_stop_track();

		// No midi driver so don't fall through
		if (!midi_driver) return;
	}

	// Handle FMSynth Stuff here
	if (midi_driver->isFMSynth()) {
		if (fname == ENDSCORE_XMI) 
			num += 2;
		else if (fname == R_SINTRO)
			fname = A_SINTRO;
		else if (fname == R_SEND)
			fname = A_SEND;
	}

	// Read the data into the XMIDI class

	FILE *mid_file = U7open(fname.c_str(), "rb");
	if (!mid_file) return;
	DataSource *mid_data = new FileDataSource(mid_file);

	XMidiFile midfile(mid_data, setup_timbre_for_track(fname));
	
	delete mid_data;
	fclose(mid_file);

	// Now give the xmidi object to the midi device

	XMidiEventList *eventlist = midfile.GetEventList(num);
	if (eventlist) {
		midi_driver->startSequence(SEQ_NUM_MUSIC, eventlist, repeat, 255);
	}

}

void MyMidiPlayer::set_timbre_lib(TimberLibrary lib)
{
	// Fixme?? - This can be VERY SLOW
	if (lib != timbre_lib)
	{
		timbre_lib = lib;
		load_timbres();
	}
}

int MyMidiPlayer::setup_timbre_for_track(std::string &str)
{
	// Default to GM
	TimberLibrary lib = TIMBRE_LIB_GM;

	// Both Games
	if (str == MAINMUS) lib = TIMBRE_LIB_GAME;
	else if (str == MAINMUS_AD) lib = TIMBRE_LIB_GAME;

	// BG
	else if (str == INTROMUS) lib = TIMBRE_LIB_MAINMENU;	// is both intro and menu
	else if (str == INTROMUS_AD) lib = TIMBRE_LIB_MAINMENU;	// is both intro and menu
	else if (str == ENDSCORE_XMI) lib = TIMBRE_LIB_ENDGAME;

	// SI
	else if (str == MAINSHP_FLX) lib = TIMBRE_LIB_MAINMENU;
	else if (str == R_SINTRO) lib = TIMBRE_LIB_INTRO;
	else if (str == A_SINTRO) lib = TIMBRE_LIB_INTRO;
	else if (str == R_SEND) lib = TIMBRE_LIB_ENDGAME;
	else if (str == A_SEND) lib = TIMBRE_LIB_ENDGAME;

	// Exult
	else if (str == EXULT_FLX) lib = TIMBRE_LIB_GM;

	set_timbre_lib(lib);

	// Nothing if the device is real
	if (midi_driver->isFMSynth() || midi_driver->isMT32())
		return XMIDIFILE_CONVERT_NOCONVERSION;

	// A 'Fake' MT32 Device ie Device with MT32 patchmaps but does not support SYSEX
	if (music_conversion == XMIDIFILE_CONVERT_GM_TO_MT32 || 
			(music_conversion == XMIDIFILE_CONVERT_NOCONVERSION && midi_driver->noTimbreSupport()))
	{
		if (timbre_lib == TIMBRE_LIB_GM)
			return XMIDIFILE_CONVERT_GM_TO_MT32;
		else
			return XMIDIFILE_CONVERT_NOCONVERSION;
	}
	// General Midi device
	else if (timbre_lib == TIMBRE_LIB_GM) 
	{
		return XMIDIFILE_CONVERT_NOCONVERSION;
	}

    return music_conversion;
}

void MyMidiPlayer::load_timbres()
{
	if (!midi_driver) return;

	if (ogg_enabled) ogg_stop_track();

	// Stop all playing sequences
	for (int i = 0; i < midi_driver->maxSequences(); i++)
		midi_driver->finishSequence(i);

	// No timbre Support!
	if (midi_driver->noTimbreSupport()) return;

	// Not in a mode that uses Timbres
	if (!midi_driver->isFMSynth() && !midi_driver->isMT32() && 
		music_conversion != XMIDIFILE_CONVERT_NOCONVERSION)
		return;

	const char *u7voice = 0;

	// Black Gate Settings
	if (Game::get_game_type() == BLACK_GATE && timbre_lib != TIMBRE_LIB_ENDGAME)
		u7voice = U7VOICE_FLX;
	// Serpent Isle
	else if (Game::get_game_type() == SERPENT_ISLE && timbre_lib == TIMBRE_LIB_MAINMENU)
		u7voice = MAINMENU_TIM;

	MidiDriver::TimbreLibraryType type;
	const char* filename;
	int index = -1;

	// General Midi Mode - AdLib
	if (timbre_lib == TIMBRE_LIB_GM && midi_driver->isFMSynth()) {
		type = MidiDriver::TIMBRE_LIBRARY_FMOPL_SETGM;
		filename = "FMOPL_SETGM";
		index = -2;
	}
	// General Midi Mode - MT32
	else if (timbre_lib == TIMBRE_LIB_GM) {
		type = MidiDriver::TIMBRE_LIBRARY_XMIDI_FILE;
		filename = EXULT_FLX;
		index = EXULT_FLX_MTGM_MID;
	}
	// U7VOICE
	else if (u7voice) {
		if (midi_driver->isFMSynth()) {
			type = MidiDriver::TIMBRE_LIBRARY_U7VOICE_AD;
			index = 1;
		}
		else {
			type = MidiDriver::TIMBRE_LIBRARY_U7VOICE_MT;
			index = 0;
		}
		filename = u7voice;
	}
	// XMIDI_MT and XMIDI_AD
	else {
		if (midi_driver->isFMSynth()) {
			type = MidiDriver::TIMBRE_LIBRARY_XMIDI_AD;
			filename = XMIDI_AD;
		}
		else {
			type = MidiDriver::TIMBRE_LIBRARY_XMIDI_MT;
			filename = XMIDI_MT;
		}
	}


	if (timbre_lib_filename == filename && timbre_lib_index == index &&
		timbre_lib_game == Game::get_game_type())
	{
		return;
	}

	timbre_lib_filename = filename;
	timbre_lib_index = index;
	timbre_lib_game = Game::get_game_type();

	FILE* file = 0;
	DataSource *ds = 0;

	if (index == -1) 
	{
		file = U7open(filename,"rb");
		if (!file) return;
		ds = new FileDataSource(file);
	}
	else if (index >= 0)
	{
		ds = new ExultDataSource(filename,index);
	}

	midi_driver->loadTimbreLibrary(ds,type);

	if (file) fclose (file);
	delete ds;
}

void	MyMidiPlayer::stop_music()
{
	if(!ogg_enabled && !midi_driver && !init_device())
		return;

	current_track = -1;
	repeating = false;
	
	if (ogg_enabled) ogg_stop_track();
	if (midi_driver) midi_driver->finishSequence(SEQ_NUM_MUSIC);
}

bool MyMidiPlayer::is_track_playing(int num)
{
	if (current_track == -1 || current_track != num) return false;

	if (ogg_enabled && ogg_is_playing()) return true;
	if (midi_driver && midi_driver->isSequencePlaying(0)) return true;

	return false;
}

int MyMidiPlayer::get_current_track()
{
	if (current_track == -1) return -1;

	if (ogg_enabled && ogg_is_playing()) return current_track;
	if (midi_driver && midi_driver->isSequencePlaying(0)) return current_track;

	return -1;
}

void MyMidiPlayer::set_music_conversion(int conv)
{
	// Same, do nothing
	if (music_conversion == conv) return;

	music_conversion = conv;
	
	switch(music_conversion) {
	case XMIDIFILE_CONVERT_MT32_TO_GS:
		config->set("config/audio/midi/convert","gs",true);
		break;
	case XMIDIFILE_CONVERT_NOCONVERSION:
		config->set("config/audio/midi/convert","mt32",true);
		if (!midi_driver->isFMSynth() && !midi_driver->isMT32()) load_timbres();
		break;
	case XMIDIFILE_CONVERT_MT32_TO_GS127:
		config->set("config/audio/midi/convert","gs127",true);
		break;
	case XMIDIFILE_CONVERT_GM_TO_MT32:
		config->set("config/audio/midi/convert","fakemt32",true);
		break;
	default:
		config->set("config/audio/midi/convert","gm",true);
		break;
	}
}

#ifdef ENABLE_MIDISFX
void MyMidiPlayer::set_effects_conversion(int conv)
{
	// Same, do nothing
	if (effects_conversion == conv) return;

	effects_conversion = conv;
	
	switch(effects_conversion) {
	case XMIDIFILE_CONVERT_NOCONVERSION:
		config->set("config/audio/effects/convert","mt32",true);
		break;
	default:
		config->set("config/audio/effects/convert","gs",true);
		break;
	}
}
#endif

void MyMidiPlayer::set_midi_driver(std::string desired_driver, bool use_oggs)
{
    // Don't kill the device if we don't need to
	if (midi_driver_name != desired_driver || ogg_enabled != use_oggs) {
		stop_music();
		if (midi_driver) midi_driver->destroyMidiDriver();
		Mix_HookMusic(NULL,NULL);
		delete midi_driver;
		midi_driver = 0;
		initialized = false;
	}

	ogg_enabled = use_oggs;
	midi_driver_name = desired_driver;

	config->set("config/audio/midi/driver",midi_driver_name,true);
	config->set("config/audio/midi/use_oggs",ogg_enabled?"yes":"no",true);

	init_device();
}


bool MyMidiPlayer::init_device(void)
{
	// already initialized? Do this first
	if (initialized) return (midi_driver != 0) || ogg_enabled;

	string	s;
	string	driver_default = "default";

	bool music = Audio::get_ptr()->is_music_enabled();

	if (!music) s = "no";
	else s = "yes";

	// Global Midi Enable/Disable
	config->set("config/audio/midi/enabled",s,true);

	// Music conversion
	config->value("config/audio/midi/convert",s,"gm");

	if (s == "gs")
		music_conversion = XMIDIFILE_CONVERT_MT32_TO_GS;
	else if (s == "mt32")
		music_conversion = XMIDIFILE_CONVERT_NOCONVERSION;
	else if (s == "none")
	{
		music_conversion = XMIDIFILE_CONVERT_NOCONVERSION;
		config->set("config/audio/midi/convert","mt32",true);
	}
	else if (s == "gs127")
		music_conversion = XMIDIFILE_CONVERT_MT32_TO_GS127;
	else if (s == "fakemt32")
		music_conversion = XMIDIFILE_CONVERT_GM_TO_MT32;
	else if (s == "gs127drum")
	{
		music_conversion = XMIDIFILE_CONVERT_MT32_TO_GS;
		config->set("config/audio/midi/convert","gs",true);
	}
	else
	{
		music_conversion = XMIDIFILE_CONVERT_MT32_TO_GM;
		config->set("config/audio/midi/convert","gm",true);
		driver_default = s;
	}

	bool sfx = false;
#ifdef ENABLE_MIDISFX
	sfx = Audio::get_ptr()->are_effects_enabled();

	// Effects conversion
	config->value("config/audio/effects/convert",s,"gs");

	if (s == "mt32")
		effects_conversion = XMIDIFILE_CONVERT_NOCONVERSION;
	else if (s == "none")
	{
		effects_conversion = XMIDIFILE_CONVERT_NOCONVERSION;
		config->set("config/audio/effects/convert","mt32",true);
	}
	else if (s == "gs127")
		effects_conversion = XMIDIFILE_CONVERT_NOCONVERSION;
	else
	{
		effects_conversion = XMIDIFILE_CONVERT_GS127_TO_GS;
		config->set("config/audio/effects/convert","gs",true);
	}
#endif

	// no need for a MIDI device (for now)
	if (!sfx && !music)
	{
		midi_driver = 0;
		return false;
	}
	
	// OGG Vorbis support
	config->value("config/audio/midi/use_oggs",s,"no");
	ogg_enabled = (s == "yes");
	config->set("config/audio/midi/use_oggs",ogg_enabled?"yes":"no",true);

	// Midi driver type.
	config->value("config/audio/midi/driver",s,driver_default.c_str());

	if (s == "normal")
	{
		config->set("config/audio/midi/driver","default",true);
		midi_driver_name = "default";
	}
	else if (s == "digital")
	{
		ogg_enabled = true;
		config->set("config/audio/midi/driver","default",true);
		config->set("config/audio/midi/use_oggs","yes",true);
		midi_driver_name = "default";
	}
	else
	{
		config->set("config/audio/midi/driver",s,true);
		midi_driver_name = s;
	}

	std::cout << "OGG Vorbis Digital Music: " << (ogg_enabled?"Enabled":"Disabled") << std::endl;

	Audio *audio = Audio::get_ptr();

	midi_driver = MidiDriver::createInstance(s,audio->get_sample_rate(),audio->is_stereo());

	initialized = true;

	if(!midi_driver) return ogg_enabled;

	if (midi_driver->isSampleProducer()) Mix_HookMusic(sdl_music_hook,this);

	timbre_lib_filename = "";
	load_timbres();

	return true;
}


MyMidiPlayer::MyMidiPlayer()	: repeating(false),current_track(-1),
				  midi_driver_name("default"), midi_driver(0), initialized(false),
				  timbre_lib(TIMBRE_LIB_GM), timbre_lib_filename(),
				  timbre_lib_index(0), timbre_lib_game(NONE),
				  music_conversion(XMIDIFILE_CONVERT_MT32_TO_GM),
				  effects_conversion(XMIDIFILE_CONVERT_GS127_TO_GS),
				  ogg_enabled(false), oggmusic(NULL)
{
	init_device();
}

MyMidiPlayer::~MyMidiPlayer()
{
	ogg_stop_track();
	if (midi_driver) midi_driver->destroyMidiDriver();
	Mix_HookMusic(NULL,NULL);
	delete midi_driver;
}

void MyMidiPlayer::sdl_music_hook(void *udata, uint8 *stream, int len)
{
	MyMidiPlayer *midi = reinterpret_cast<MyMidiPlayer *>(udata);

	MidiDriver *midi_driver = midi->midi_driver;

	if (midi_driver && midi_driver->isInitialized() && midi_driver->isSampleProducer())
		midi_driver->produceSamples(reinterpret_cast<sint16*>(stream), len);
}

#ifdef ENABLE_MIDISFX
void    MyMidiPlayer::start_sound_effect(int num)
{
#ifdef DEBUG
	cout << "Audio subsystem request: sound effect # " << num << endl;
#endif

	int real_num = num;

	if (Game::get_game_type() == BLACK_GATE) real_num = bgconv[num];

	cout << "Real num " << real_num << endl;

	// No driver
	if (!midi_driver && !init_device())return;

	// Only support SFX on devices with 2 or more sequences
	if (midi_driver->maxSequences() < 2) return;
	
	char		*buffer;
	size_t		size;
	DataSource 	*mid_data;

	U7object	track("<DATA>/midisfx.flx",real_num);

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
	XMidiFile		midfile(mid_data, effects_conversion);
	
	delete mid_data;
	delete [] buffer;

	// Now give the xmidi object to the midi device
	XMidiEventList *eventlist = midfile.GetEventList(0);
	if (eventlist) midi_driver->startSequence(1,eventlist,false,255);
	
}

void    MyMidiPlayer::stop_sound_effects()
{
	if (midi_driver)
	{
		// Only support SFX on devices with 2 or more sequences
		if (midi_driver->maxSequences() >= 2) midi_driver->finishSequence(1);
	}
}
#endif

bool MyMidiPlayer::ogg_play_track(std::string filename, int num, bool repeat)
{
	string ogg_name = "";

	if (filename == EXULT_FLX && num == EXULT_FLX_MEDITOWN_MID)
	{
		ogg_name = "exult.ogg";
	}
	if (Game::get_game_type() == BLACK_GATE) 
	{
		if(filename == INTROMUS || filename == INTROMUS_AD)
		{
			if (num == 0)
				ogg_name = "00bg.ogg";
			else if (num == 1)
				ogg_name = "01bg.ogg";
			else if (num == 2)
				ogg_name = "02bg.ogg";
			else if (num == 3)
				ogg_name = "03bg.ogg";
			else if (num == 4)
				ogg_name = "endcr01.ogg";
			else if (num == 5)
				ogg_name = "endcr02.ogg";
		}
		else if (filename == ENDSCORE_XMI)
		{
			if (num == 1 || num == 3)
				ogg_name = "end01bg.ogg";
			else if (num == 2 || num == 4)
				ogg_name = "end02bg.ogg";
		}
		else if (filename == MAINMUS || filename == MAINMUS_AD)
		{
			char outputstr[255];
			snprintf(outputstr, 255, "%02dbg.ogg", num);
			ogg_name = outputstr;
		}
	}
	else if (Game::get_game_type() == SERPENT_ISLE) 
	{
		if(filename == MAINSHP_FLX)
		{
			if (num == 28 || num == 27) 
				ogg_name = "03bg.ogg";
			else if(num == 30 || num == 29)
				ogg_name = "endcr01.ogg";
			else if(num == 32 || num == 31)
				ogg_name = "endcr02.ogg";
		}
		else if(filename == R_SINTRO || filename == A_SINTRO)
		{
			ogg_name = "si01.ogg";
		}
		else if(filename == R_SEND || filename == A_SEND)
		{
			ogg_name = "si13.ogg";
		}
		else if (filename == MAINMUS || filename == MAINMUS_AD)
		{
			ogg_name = bgconvmusic[num];
		}
	}

	if (ogg_name == "") return false;

	ogg_name = get_system_path("<MUSIC>/" + ogg_name);

	Mix_Music *newmusic = Mix_LoadMUS(ogg_name.c_str());
	if (!newmusic) return false;

	if(oggmusic) Mix_FreeMusic(oggmusic);
	else Mix_HookMusic(NULL,NULL);

	oggmusic = newmusic;

#ifdef DEBUG
	cout << "OGG audio: Music track " << ogg_name << endl;
#endif

	int ret = Mix_PlayMusic(oggmusic, repeat?2:0);

	if (ret != 0) std::cerr << "Failed to play OGG Music Track " << ogg_name << ". Reason: " << Mix_GetError() << std::endl;

	return  ret == 0;
}

void MyMidiPlayer::ogg_stop_track(void)
{
	if(oggmusic)
	{
		Mix_FreeMusic(oggmusic);
		oggmusic = NULL;

		if (midi_driver && midi_driver->isSampleProducer())
			Mix_HookMusic(sdl_music_hook,this);
	}
}

bool MyMidiPlayer::ogg_is_playing(void)
{
	int playing = Mix_PlayingMusic();
	if (!playing && oggmusic) ogg_stop_track();

	return playing!=0;
}

