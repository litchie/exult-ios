/*
 *  Copyright (C) 2003-2011  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

// Includes Pentagram headers so we must include pent_include.h 
#include "pent_include.h"

#include "SDL_events.h"

#include <iostream>

#include "Audio.h"
#include "AudioOptions_gump.h"
#include "Configuration.h"
#include "Gump_button.h"
#include "Gump_ToggleButton.h"
#include "exult.h"
#include "exult_flx.h"
#include "gamewin.h"
#include "mouse.h"
#include "XMidiFile.h"
#include "Enabled_button.h"
#include "game.h"
#include "exult_constants.h"
#include "font.h"

#include "MidiDriver.h"
#include "AudioMixer.h"
#include <sstream>

using namespace Pentagram;

static const int rowy[] = { 5, 17, 29, 41, 53, 65, 77, 89,
							101, 113, 125, 137, 149, 161, 173 };
static const int colx[] = { 35, 50, 132 };

static const char* oktext = "OK";
static const char* canceltext = "CANCEL";

uint32 AudioOptions_gump::sample_rates[5] = {11025, 22050, 44100, 48000, 0};
int AudioOptions_gump::num_sample_rates = 0;

class AudioOptions_button : public Text_button {
public:
	AudioOptions_button(Gump *par, const std::string &text, int px, int py)
		: Text_button(par, text, px, py, 59, 11)
		{  }
					// What to do when 'clicked':
	virtual bool activate(int button)
	{
		if (button != 1) return false;
		
		if (text == canceltext) {
			((AudioOptions_gump*)parent)->cancel();
		} else if (text == oktext) {
			((AudioOptions_gump*)parent)->close();
		}
		return true;
	}
};

class AudioTextToggle : public Gump_ToggleTextButton {
public:
	AudioTextToggle(Gump* par, std::string *s, int px, int py, int width,
					   int selectionnum, int numsel)
		: Gump_ToggleTextButton(par, s, selectionnum, numsel, px, py, width)
	{ }

	friend class AudioOptions_gump;
	virtual void toggle(int state) { 
		((AudioOptions_gump*)parent)->toggle((Gump_button*)this, state);
	}
};


class AudioEnabledToggle : public Enabled_button {
public:
	AudioEnabledToggle(Gump* par, int px, int py, int selectionnum)
		: Enabled_button(par, selectionnum, px, py, 59)
	{ }

	friend class AudioOptions_gump;
	virtual void toggle(int state) {
		((AudioOptions_gump*)parent)->toggle((Gump_button*)this, state);
	}
};	

void AudioOptions_gump::close()
{
	save_settings();
	done = 1;
}

void AudioOptions_gump::cancel()
{
	done = 1;
}

void AudioOptions_gump::toggle(Gump_button* btn, int state)
{
	if (btn == buttons[id_audio_enabled]) {		// audio on/off
		audio_enabled = state;
		rebuild_buttons();
		paint();
	} else if (btn == buttons[id_sample_rate]) {	// sample rate
		sample_rate = state;
	} else if (btn == buttons[id_speaker_type]) {	// speaker type
		speaker_type = state;
	} else if (btn == buttons[id_music_enabled]) {	// music on/off 
		midi_enabled = state;
		rebuild_midi_buttons();
		paint();
	} else if (btn == buttons[id_music_digital]) {	// digital music on/off 
		midi_ogg_enabled = state;
		paint();
	} else if (btn == buttons[id_midi_driver]) { // midi driver
		midi_driver = state;
		rebuild_mididriveroption_buttons();
		paint();
	} else if (btn == buttons[id_music_looping]) { // midi looping
		midi_looping = state;
	} else if (btn == buttons[id_midi_conv]) { // midi conversion
		midi_conversion = state;
	} else if (btn == buttons[id_midi_effects]) { // midi reverb/chorus
		midi_reverb_chorus = state;
	} else if (btn == buttons[id_sfx_enabled]) { // sfx on/off
		sfx_enabled = state;
		rebuild_sfx_buttons();
		paint();
	} else if (btn == buttons[id_sfx_pack]) { // sfx conversion
		if (have_digital_sfx() && sfx_enabled == 1) {
			sfx_package = state;
#ifdef ENABLE_MIDISFX
		} else if (sfx_enabled && have_midi_pack) {
			if (state == 1) {
				sfx_conversion = XMIDIFILE_CONVERT_GS127_TO_GS;
			} else {
				sfx_conversion = XMIDIFILE_CONVERT_NOCONVERSION;
			}
#endif
		}
	} else if (btn == buttons[id_speech_enabled]) { // speech on/off
		speech_enabled = state;
	}
}

static void strip_path(std::string& file)
{
	if (!file.size())
		return;
	size_t sep = file.rfind('/');
	if (sep != std::string::npos)
	{
		sep++;
		file = file.substr(sep);
	}
}

void AudioOptions_gump::rebuild_buttons()
{
	for (size_t i = id_sample_rate; i < id_count; ++i) {
		FORGET_OBJECT(buttons[i]);
	}

	if (!audio_enabled) return;

	std::string *sampleRates = new std::string[5];
	sampleRates[0] = "11025";
	sampleRates[1] = "22050";
	sampleRates[2] = "44100";
	sampleRates[3] = "48000";
	sampleRates[4] = sample_rate_str;

	buttons[id_sample_rate] = new AudioTextToggle (this, sampleRates, colx[2], rowy[1], 59,
									  sample_rate, num_sample_rates);

	std::string *speaker_types = new std::string[2];
	speaker_types[0] = "Mono";
	speaker_types[1] = "Stereo";
	buttons[id_speaker_type] = new AudioTextToggle (this, speaker_types, colx[2], rowy[2], 59,
									  speaker_type, 2);

	// music on/off
	buttons[id_music_enabled] = new AudioEnabledToggle(this, colx[2], rowy[3], midi_enabled);
	if (midi_enabled)
		rebuild_midi_buttons();
	
	// sfx on/off
	std::string* sfx_options = new std::string[nsfxopts];
	sfx_options[0] = "Disabled";
	int i = 1;
	if (have_digital_sfx())
#ifndef ENABLE_MIDISFX
		sfx_options[i++] = "Enabled";
#else
		sfx_options[i++] = "Digital";
	midi_state = -1;
	if (have_midi_pack) {
		sfx_options[i++] = "Midi";
		midi_state = i -1;
	}
#endif
	buttons[id_sfx_enabled] = new AudioTextToggle(this, sfx_options, colx[2], rowy[10],
									 59, sfx_enabled, nsfxopts);
	if (sfx_enabled)
		rebuild_sfx_buttons();
	
	// speech on/off
	buttons[id_speech_enabled] = new AudioEnabledToggle(this,colx[2],rowy[13], speech_enabled);
}

void AudioOptions_gump::rebuild_midi_buttons()
{
	unsigned int i;
	for (i = id_music_looping; i < id_sfx_enabled; ++i) {
		FORGET_OBJECT(buttons[i]);
	}

	if (!midi_enabled) return;

	// ogg enabled/disabled
	buttons[id_music_digital] = new AudioEnabledToggle(this, colx[2], rowy[5], midi_ogg_enabled);

	unsigned int num_midi_drivers = MidiDriver::getDriverCount();
	std::string* midi_drivertext = new std::string[num_midi_drivers+1];
	for (i = 0; i < num_midi_drivers; i++) 
		midi_drivertext[i] = MidiDriver::getDriverName(i);
	midi_drivertext[i] = "Default";

	// midi driver
	buttons[id_midi_driver] = new AudioTextToggle(this, midi_drivertext, 
									 colx[2]-15, rowy[6], 74, midi_driver, num_midi_drivers+1);

	// looping on/off
	buttons[id_music_looping] = new AudioEnabledToggle(this, colx[2], rowy[4], midi_looping);

	rebuild_mididriveroption_buttons();

}


void AudioOptions_gump::rebuild_sfx_buttons()
{
	FORGET_OBJECT(buttons[id_sfx_pack]);

	if (!sfx_enabled)
		return;
	else if (sfx_enabled == 1 && have_digital_sfx() && !gwin->is_in_exult_menu())
		{
		std::string* sfx_digitalpacks = new std::string[nsfxpacks];
		int i = 0;
		if (have_roland_pack)
			sfx_digitalpacks[i++] = "Roland MT-32";
		if (have_blaster_pack)
			sfx_digitalpacks[i++] = "Sound Blaster";
		if (have_custom_pack)
			sfx_digitalpacks[i++] = "Custom";
		buttons[id_sfx_pack] = new AudioTextToggle(this, sfx_digitalpacks, colx[2]-33,
										 rowy[11], 92, sfx_package, nsfxpacks);
		}
#ifdef ENABLE_MIDISFX
	else if (sfx_enabled == midi_state)
		{
		std::string* sfx_conversiontext = new std::string[2];
		sfx_conversiontext[0] = "None";
		sfx_conversiontext[1] = "GS";

		// sfx conversion
		buttons[id_sfx_pack] = new AudioTextToggle(this, sfx_conversiontext, colx[2],
										 rowy[11], 59, sfx_conversion == 5 ? 1: 0,2);
		}
#endif
}

void AudioOptions_gump::rebuild_mididriveroption_buttons()
{
	FORGET_OBJECT(buttons[id_midi_conv]);
	FORGET_OBJECT(buttons[id_midi_effects]);


	std::string s = "Default";
	if (midi_driver != MidiDriver::getDriverCount()) s = MidiDriver::getDriverName(midi_driver);

	if (s != "FMOpl" && s != "MT32Emu" && s != "Disabled")
	{
		int string_size = 5;
#ifdef MACOSX
		if (s == "Default" || s == "CoreAudio"){
			if (midi_conversion > 3)
				midi_conversion = 0;
			string_size = 4;
		}
#endif
		std::string* midi_conversiontext = new std::string[string_size];
		midi_conversiontext[0] = std::string("Fake MT32");
		midi_conversiontext[1] = std::string("GM");
		midi_conversiontext[2] = std::string("GS");
		midi_conversiontext[3] = std::string("GS127");
		if (string_size == 5)
			midi_conversiontext[4] = std::string("MT32");

		// midi conversion
		buttons[id_midi_conv] = new AudioTextToggle(this, midi_conversiontext, 
										 colx[2]-7, rowy[7], 66,
										 midi_conversion, string_size);
	}

	if (s != "FMOpl" && s != "Disabled")
	{
		std::string* midi_reverbchorustext = new std::string[4];
		midi_reverbchorustext[0] = std::string("Disabled");
		midi_reverbchorustext[1] = std::string("Reverb");
		midi_reverbchorustext[2] = std::string("Chorus");
		midi_reverbchorustext[3] = std::string("Both");

		// reverb/chorus combo
		buttons[id_midi_effects] = new AudioTextToggle(this, midi_reverbchorustext, 
										 colx[2], rowy[8], 59,
										 midi_reverb_chorus, 4);
	}
}

void AudioOptions_gump::load_settings()
{
	std::string s;
	audio_enabled = (Audio::get_ptr()->is_audio_enabled() ? 1 : 0);
	midi_enabled = (Audio::get_ptr()->is_music_enabled() ? 1 : 0);
	bool sfx_on = (Audio::get_ptr()->are_effects_enabled() ? true : false);
	speech_enabled = (Audio::get_ptr()->is_speech_enabled() ? 1 : 0);
	midi_looping = (Audio::get_ptr()->is_music_looping_allowed() ? 1 : 0);

#ifdef UNDER_CE
	speaker_type = 0; // mono
	sample_rate = 11025;
#else
	speaker_type = 1; // stereo
	sample_rate = 22050;
#endif
	config->value("config/audio/stereo", speaker_type, speaker_type);
	config->value("config/audio/sample_rate", sample_rate, sample_rate);
	num_sample_rates = 4;
	o_sample_rate = sample_rate;
	o_speaker_type = speaker_type;
	if (sample_rate == 11025)
		sample_rate = 0;
	else if (sample_rate == 22050)
		sample_rate = 1;
	else if (sample_rate == 44100)
		sample_rate = 2;
	else if (sample_rate == 48000)
		sample_rate = 3;
	else {
		sample_rates[4] = sample_rate;
		std::ostringstream strStream;
		strStream << sample_rate;
		sample_rate_str = strStream.str();
		sample_rate = 4;
		num_sample_rates = 5;
	}

	MyMidiPlayer *midi = Audio::get_ptr()->get_midi();
	if (midi ) {
		midi_conversion = midi->get_music_conversion();
		midi_ogg_enabled = midi->get_ogg_enabled();

		s = midi->get_midi_driver();
		for (midi_driver = 0; midi_driver < MidiDriver::getDriverCount(); midi_driver++) {
			std::string name = MidiDriver::getDriverName(midi_driver);
			if (!Pentagram::strcasecmp(name.c_str(),s.c_str())) break;
		}

#ifdef ENABLE_MIDISFX
		sfx_conversion = midi->get_effects_conversion();
#endif
	} else {
		// String for default value for driver type
		std::string driver_default = "default";

		config->value("config/audio/midi/convert",s,"gm");
		if (s == "gs")
			midi_conversion = XMIDIFILE_CONVERT_MT32_TO_GS;
		else if (s == "none" || s == "mt32")
			midi_conversion = XMIDIFILE_CONVERT_NOCONVERSION;
		else if (s == "gs127")
			midi_conversion = XMIDIFILE_CONVERT_MT32_TO_GS127;
		else if (s == "gs127drum")
			midi_conversion = XMIDIFILE_CONVERT_MT32_TO_GS;
		else if (s == "fakemt32")
			midi_conversion = XMIDIFILE_CONVERT_GM_TO_MT32;
		else
		{
			midi_conversion = XMIDIFILE_CONVERT_MT32_TO_GM;
			config->set("config/audio/midi/convert","gm",true);

			driver_default = "s";
		}

		// OGG Vorbis support
		config->value("config/audio/midi/use_oggs",s,"no");
		midi_ogg_enabled = (s == "yes" ? 1 : 0);

		config->value("config/audio/midi/driver",s,driver_default.c_str());

		if (s == "digital") {
			midi_ogg_enabled = true;
			config->set("config/audio/midi/driver","default",true);
			config->set("config/audio/midi/use_oggs","yes",true);
			midi_driver = MidiDriver::getDriverCount();
		}
		else for (midi_driver = 0; midi_driver < MidiDriver::getDriverCount(); midi_driver++) {
			std::string name = MidiDriver::getDriverName(midi_driver);
			if (!Pentagram::strcasecmp(name.c_str(),s.c_str())) break;
		}


#ifdef ENABLE_MIDISFX
		config->value("config/audio/effects/convert",s,"gs");
		if (s == "none" || s == "mt32")
			sfx_conversion = XMIDIFILE_CONVERT_NOCONVERSION;
		else if (s == "gs127")
			sfx_conversion = XMIDIFILE_CONVERT_NOCONVERSION;
		else
			sfx_conversion = XMIDIFILE_CONVERT_GS127_TO_GS;
#endif
	}

	config->value("config/audio/midi/reverb/enabled",s,"no");
	midi_reverb_chorus = (s == "yes" ? 1 : 0);

	config->value("config/audio/midi/chorus/enabled",s,"no");
	midi_reverb_chorus |= (s == "yes" ? 2 : 0);
	
	std::string d = "config/disk/game/" + Game::get_gametitle() + "/waves";
	config->value(d.c_str(),s,"---");
	if (have_roland_pack && s == rolandpack)
		sfx_package = 0;
	else if (have_blaster_pack && s == blasterpack)
		sfx_package = have_roland_pack?1:0;
	else if (have_custom_pack)
		sfx_package = nsfxpacks - 1;
	else	// This should *never* happen.
		sfx_package = 0;
	if (!sfx_on)
		sfx_enabled = 0;
	else
		{
#ifdef ENABLE_MIDISFX
		config->value("config/audio/effects/midi",s,"no");
#else
		s = "no";
#endif
		if (s == "yes" && have_midi_pack)
			sfx_enabled = 1 + have_digital_sfx();
		else if (have_digital_sfx())
			sfx_enabled = 1;
		else
			// Actually disable sfx if no sfx packs and no midi sfx.
			// This is just in case -- it should not be needed.
			sfx_enabled = 0;
		}
}

AudioOptions_gump::AudioOptions_gump() : Modal_gump(0, EXULT_FLX_AUDIOOPTIONS_SHP, SF_EXULT_FLX)
{
	set_object_area(Rectangle(0,0,0,0), 8, 187);//++++++ ???

	for (int i = id_first; i < id_count; i++) buttons[i] = 0;

	Exult_Game game = Game::get_game_type();
	std::string title = Game::get_gametitle();
	have_config_pack  = Audio::have_config_sfx(title, &configpack);
	have_roland_pack  = Audio::have_roland_sfx(game, &rolandpack);
	have_blaster_pack = Audio::have_sblaster_sfx(game, &blasterpack);
	have_midi_pack    = Audio::have_midi_sfx(&midipack);
	strip_path(configpack);
	strip_path(rolandpack);
	strip_path(blasterpack);
	strip_path(midipack);
	have_custom_pack  = have_config_pack && 
	                   configpack != rolandpack &&
	                   configpack != blasterpack;

	nsfxopts = 1;	// For "Disabled".
	nsfxpacks = 0;
	if (have_digital_sfx())
		{	// Have digital sfx.
		nsfxopts++;
		nsfxpacks += (int)have_roland_pack + (int)have_blaster_pack
		           + (int)have_custom_pack;
		if (have_custom_pack)
			sfx_custompack = configpack;
		}
	if (have_midi_pack)	// Midi SFX.
		nsfxopts++;

	load_settings();

	rebuild_buttons();


	// audio on/off
	buttons[id_audio_enabled] = new AudioEnabledToggle(this, colx[2], rowy[0], audio_enabled);
	// Ok
	buttons[id_ok] = new AudioOptions_button(this, oktext, colx[0], rowy[14]);
	// Cancel
	buttons[id_cancel] = new AudioOptions_button(this, canceltext, colx[2], rowy[14]);
}

AudioOptions_gump::~AudioOptions_gump()
{
	for (int i = id_first; i < id_count; i++)
		if (buttons[i]) delete buttons[i];
}

void AudioOptions_gump::save_settings()
{
	int track_playing = -1;
	if (Audio::get_ptr()->get_midi())
		track_playing = Audio::get_ptr()->get_midi()->get_current_track();
	config->set("config/audio/sample_rate", sample_rates[sample_rate], false);
	config->set("config/audio/stereo", speaker_type ? "yes": "no", false);
	if (sample_rates[sample_rate] != o_sample_rate ||
			speaker_type != o_speaker_type) {
		Audio::Destroy();
		Audio::Init();
	}
	Audio::get_ptr()->set_audio_enabled(audio_enabled == 1);
	Audio::get_ptr()->set_music_enabled(midi_enabled == 1);
	if (!midi_enabled)		// Stop what's playing.
		Audio::get_ptr()->stop_music();
	Audio::get_ptr()->set_effects_enabled(sfx_enabled != 0);
	if (!sfx_enabled)		// Stop what's playing.
		Audio::get_ptr()->stop_sound_effects();
	Audio::get_ptr()->set_speech_enabled(speech_enabled == 1);
	Audio::get_ptr()->set_allow_music_looping(midi_looping == 1);

	config->set("config/audio/enabled", audio_enabled ? "yes" : "no", false);
	config->set("config/audio/midi/enabled",midi_enabled ? "yes" : "no", false);
	config->set("config/audio/effects/enabled",sfx_enabled?"yes":"no", false);
	config->set("config/audio/speech/enabled",speech_enabled?"yes":"no", false);

	config->set("config/audio/midi/chorus/enabled", (midi_reverb_chorus&2) ? "yes" : "no", false);
	config->set("config/audio/midi/reverb/enabled", (midi_reverb_chorus&1)? "yes" : "no", false);
	config->set("config/audio/midi/looping", midi_looping ? "yes" : "no", false);

	std::string d = "config/disk/game/" + Game::get_gametitle() + "/waves";
	std::string waves;
	if (!gwin->is_in_exult_menu()) {
		int i = 0;
		if (have_roland_pack && sfx_package == i++)
			waves = rolandpack;
		else if (have_blaster_pack && sfx_package == i++)
			waves = blasterpack;
		else if (have_custom_pack && sfx_package == i++)
			waves = sfx_custompack;
		if (waves != sfx_custompack)
			config->set(d.c_str(), waves, false);
	}
#ifdef ENABLE_MIDISFX
	config->set("config/audio/effects/midi",sfx_enabled==1 + have_digital_sfx()?"yes":"no",false);
#endif

	MyMidiPlayer *midi = Audio::get_ptr()->get_midi();

	if (midi) {
		std::string s = "default";
		if (midi_driver != MidiDriver::getDriverCount()) s = MidiDriver::getDriverName(midi_driver);
		midi->set_midi_driver(s,midi_ogg_enabled!=0);
		midi->set_music_conversion(midi_conversion);
#ifdef ENABLE_MIDISFX
		midi->set_effects_conversion(sfx_conversion);
#endif
	} else {
		switch(midi_conversion) {
		case XMIDIFILE_CONVERT_MT32_TO_GS:
			config->set("config/audio/midi/convert","gs",false);
			break;
		case XMIDIFILE_CONVERT_NOCONVERSION:
			config->set("config/audio/midi/convert","mt32",false);
			break;
		case XMIDIFILE_CONVERT_MT32_TO_GS127:
			config->set("config/audio/midi/convert","gs127",false);
			break;
		case XMIDIFILE_CONVERT_GM_TO_MT32:
			config->set("config/audio/midi/convert","fakemt32",false);
			break;
		default:
			config->set("config/audio/midi/convert","gm",false);
			break;
		}

		if (midi_driver == MidiDriver::getDriverCount())
			config->set("config/audio/midi/driver","default",false);
		else
			config->set("config/audio/midi/driver",MidiDriver::getDriverName(midi_driver),false);

#ifdef ENABLE_MIDISFX
		switch(sfx_conversion) {
		case XMIDIFILE_CONVERT_NOCONVERSION:
			config->set("config/audio/effects/convert","mt32",false);
			break;
		default:
			config->set("config/audio/effects/convert","gs",false);
			break;
		}
#endif
	}
	config->write_back();
	Audio::get_ptr()->Init_sfx();
	// restart music track if one was playing and isn't anymore
	if (Audio::get_ptr()->get_midi() && Audio::get_ptr()->is_music_enabled() &&
			Audio::get_ptr()->get_midi()->get_current_track() != track_playing) {
		if (gwin->is_in_exult_menu())
			Audio::get_ptr()->start_music(EXULT_FLX_MEDITOWN_MID, true, EXULT_FLX);
		else
			Audio::get_ptr()->start_music(track_playing, midi_looping == 1);
	}
}

void AudioOptions_gump::paint()
{
	Gump::paint();
	for (int i = id_first; i < id_count; i++)
		if (buttons[i])
			buttons[i]->paint();

	Font *font = fontManager.get_font("SMALL_BLACK_FONT");
	Image_window8 *iwin = gwin->get_win();
	
	font->paint_text(iwin->get_ib8(), "Audio:", x + colx[0], y + rowy[0] + 1);
	if (audio_enabled) {
		font->paint_text(iwin->get_ib8(), "sample rate", x + colx[1], y + rowy[1] + 1);
		font->paint_text(iwin->get_ib8(), "speaker type", x + colx[1], y + rowy[2] + 1);
		font->paint_text(iwin->get_ib8(), "Music options:", x + colx[0], y + rowy[3] + 1);
		if (midi_enabled) {
			font->paint_text(iwin->get_ib8(), "looping", x + colx[1], y + rowy[4] + 1);
			font->paint_text(iwin->get_ib8(), "digital music", x + colx[1], y + rowy[5] + 1);
			font->paint_text(iwin->get_ib8(), "midi driver", x + colx[1], y + rowy[6] + 1);
			if (buttons[id_midi_conv]) font->paint_text(iwin->get_ib8(), "device type", x+colx[1], y+rowy[7] + 1);
			if (buttons[id_midi_effects]) font->paint_text(iwin->get_ib8(), "effects", x + colx[1], y + rowy[8] + 1);
		}
		font->paint_text(iwin->get_ib8(), "SFX options:", x + colx[0], y + rowy[9] + 1);
		font->paint_text(iwin->get_ib8(), "SFX", x + colx[1], y + rowy[10] + 1);
		if (sfx_enabled == 1 && have_digital_sfx() && !gwin->is_in_exult_menu()) {
			font->paint_text(iwin->get_ib8(), "pack", x + colx[1], y + rowy[11] + 1);
		}
#ifdef ENABLE_MIDISFX
		else if (sfx_enabled == midi_state) {
			font->paint_text(iwin->get_ib8(), "conversion", x + colx[1], y + rowy[11] + 1);
		}
#endif
		font->paint_text(iwin->get_ib8(), "Speech options:", x + colx[0], y + rowy[12] + 1);
		font->paint_text(iwin->get_ib8(), "speech", x + colx[1], y + rowy[13] + 1);
	}
	gwin->set_painted();
}

bool AudioOptions_gump::mouse_down(int mx, int my, int button)
{
	// Only left and right buttons
	if (button != 1 && button != 3) return false;

	// We'll eat the mouse down if we've already got a button down
	if (pushed) return true;

	// First try checkmark
	pushed = Gump::on_button(mx, my);
					
	// Try buttons at bottom.
	if (!pushed) {
		for (int i = id_first; i < id_count; i++) {
			if (buttons[i] && buttons[i]->on_button(mx, my)) {
				pushed = buttons[i];
				break;
			}
		}
	}

	if (pushed && !pushed->push(button))			// On a button?
		pushed = 0;
		
	return button == 1 || pushed != 0;
}

bool AudioOptions_gump::mouse_up(int mx, int my, int button)
{
	// Not Pushing a button?
	if (!pushed) return false;

	if (pushed->get_pushed() != button) return button == 1;

	bool res = false;
	pushed->unpush(button);
	if (pushed->on_button(mx, my))
		res = ((Gump_button*)pushed)->activate(button);
	pushed = 0;
	return res;
}
