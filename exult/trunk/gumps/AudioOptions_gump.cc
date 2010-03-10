/*
 *  Copyright (C) 2003-2004  The Exult Team
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

#include "MidiDriver.h"

using std::cerr;
using std::endl;

static const int rowy[] = { 5,  
			    19, 31, 43, 55, 67, 79,  
			    91, 103,  
			    117, 127,  
			    137, 156 };
static const int colx[] = { 35, 52, 130 };

static const char* oktext = "OK";
static const char* canceltext = "CANCEL";

class AudioOptions_button : public Text_button {
public:
	AudioOptions_button(Gump *par, const std::string &text, int px, int py)
		: Text_button(par, text, px, py, 59, 11)
		{  }
					// What to do when 'clicked':
	virtual void activate();
};

void AudioOptions_button::activate()
{
	if (text == canceltext) {
		((AudioOptions_gump*)parent)->cancel();
	} else if (text == oktext) {
		((AudioOptions_gump*)parent)->close();
	}
}

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
	if (btn == buttons[0]) {		// audio on/off
		audio_enabled = state;
		rebuild_buttons();
		paint();
	} else if (btn == buttons[1]) {	// midi on/off 
		midi_enabled = state;
		rebuild_midi_buttons();
		paint();
	} else if (btn == buttons[11]) {	// digital music on/off 
		midi_ogg_enabled = state;
		paint();
	} else if (btn == buttons[2]) { // midi driver
		midi_driver = state;
		rebuild_mididriveroption_buttons();
		paint();
	} else if (btn == buttons[5]) { // midi looping
		midi_looping = state;
	} else if (btn == buttons[3]) { // midi conversion
		midi_conversion = state;
	} else if (btn == buttons[4]) { // midi reverb/chorus
		midi_reverb_chorus = state;
	} else if (btn == buttons[6]) { // sfx on/off
		sfx_enabled = state;
		rebuild_sfx_buttons();
		paint();
	} else if (btn == buttons[7]) { // sfx conversion
		if (sfx_enabled == 1) {
			sfx_package = state;
#ifdef ENABLE_MIDISFX
		} else if (sfx_enabled == 2) {
			if (state == 1) {
				sfx_conversion = XMIDIFILE_CONVERT_GS127_TO_GS;
			} else {
				sfx_conversion = XMIDIFILE_CONVERT_NOCONVERSION;
			}
#endif
		}
	} else if (btn == buttons[8]) { // speech on/off
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
	for (unsigned int i = 1; i < 9; ++i) {
		delete buttons[i];
		buttons[i] = 0;
	}

	if (!audio_enabled) return;

	// music on/off
	buttons[1] = new AudioEnabledToggle(this, colx[2], rowy[1], midi_enabled);
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
	if (have_midi_pack)
		sfx_options[i++] = "Midi";
#endif
	buttons[6] = new AudioTextToggle(this, sfx_options, colx[2], rowy[8],
									 59, sfx_enabled, nsfxopts);
	if (sfx_enabled)
		rebuild_sfx_buttons();
	
	// speech on/off
	buttons[8] = new AudioEnabledToggle(this,colx[2],rowy[11], speech_enabled);
}

void AudioOptions_gump::rebuild_midi_buttons()
{
	unsigned int i;
	for (i = 2; i < 6; ++i) {
		delete buttons[i];
		buttons[i] = 0;
	}
	delete buttons[11];
	buttons[11] = 0;

	if (!midi_enabled) return;

	// ogg enabled/disabled
	buttons[11] = new AudioEnabledToggle(this, colx[2], rowy[3], midi_ogg_enabled);

	unsigned int num_midi_drivers = MidiDriver::getDriverCount();
	std::string* midi_drivertext = new std::string[num_midi_drivers+1];
	for (i = 0; i < num_midi_drivers; i++) 
		midi_drivertext[i] = MidiDriver::getDriverName(i);
	midi_drivertext[i] = "Default";

	// midi driver
	buttons[2] = new AudioTextToggle(this, midi_drivertext, 
									 colx[2], rowy[4], 59, midi_driver, num_midi_drivers+1);

	// looping on/off
	buttons[5] = new AudioEnabledToggle(this, colx[2], rowy[2], midi_looping);

	rebuild_mididriveroption_buttons();

}


void AudioOptions_gump::rebuild_sfx_buttons()
{
	delete buttons[7];
	buttons[7] = 0;

	if (!sfx_enabled)
		return;
	else if (sfx_enabled == 1)
		{
		std::string* sfx_digitalpacks = new std::string[nsfxpacks];
		int i = 0;
		if (have_roland_pack)
			sfx_digitalpacks[i++] = "Roland MT-32";
		if (have_blaster_pack)
			sfx_digitalpacks[i++] = "Sound Blaster";
		if (have_custom_pack)
			sfx_digitalpacks[i++] = "Custom";
		buttons[7] = new AudioTextToggle(this, sfx_digitalpacks, colx[2]-33,
										 rowy[9], 92, sfx_package, nsfxpacks);
		}
#ifdef ENABLE_MIDISFX
	else
		{
		std::string* sfx_conversiontext = new std::string[2];
		sfx_conversiontext[0] = "None";
		sfx_conversiontext[1] = "GS";

		// sfx conversion
		buttons[7] = new AudioTextToggle(this, sfx_conversiontext, colx[2],
										 rowy[9], 59, sfx_conversion/4,2);
		}
#endif
}

void AudioOptions_gump::rebuild_mididriveroption_buttons()
{
	delete buttons[3];
	buttons[3] = 0;
	delete buttons[4];
	buttons[4] = 0;


	std::string s = "Default";
	if (midi_driver != MidiDriver::getDriverCount()) s = MidiDriver::getDriverName(midi_driver);

	if (s != "FMOpl" && s != "MT32Emu" && s != "Disabled")
	{
		std::string* midi_conversiontext = new std::string[5];
		midi_conversiontext[0] = std::string("MT32");
		midi_conversiontext[1] = std::string("GM");
		midi_conversiontext[2] = std::string("GS");
		midi_conversiontext[3] = std::string("GS127");
		midi_conversiontext[4] = std::string("Fake MT32");

		// midi conversion
		buttons[3] = new AudioTextToggle(this, midi_conversiontext, 
										 colx[2], rowy[5], 66,
										 midi_conversion, 5);
	}

	if (s != "FMOpl" && s != "Disabled")
	{
		std::string* midi_reverbchorustext = new std::string[4];
		midi_reverbchorustext[0] = std::string("Disabled");
		midi_reverbchorustext[1] = std::string("Reverb");
		midi_reverbchorustext[2] = std::string("Chorus");
		midi_reverbchorustext[3] = std::string("Both");

		// reverb/chorus combo
		buttons[4] = new AudioTextToggle(this, midi_reverbchorustext, 
										 colx[2], rowy[6], 59,
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

	if (Audio::get_ptr()->get_midi()) {
		midi_conversion = Audio::get_ptr()->get_midi()->get_music_conversion();
		midi_ogg_enabled = Audio::get_ptr()->get_midi()->get_ogg_enabled();

		s = Audio::get_ptr()->get_midi()->get_midi_driver();
		for (midi_driver = 0; midi_driver < MidiDriver::getDriverCount(); midi_driver++) {
			std::string name = MidiDriver::getDriverName(midi_driver);
			if (!Pentagram::strcasecmp(name.c_str(),s.c_str())) break;
		}

#ifdef ENABLE_MIDISFX
		sfx_conversion =Audio::get_ptr()->get_midi()->get_effects_conversion();
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
		if (s == "yes")
			sfx_enabled = nsfxopts-1;
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
	set_object_area(Rectangle(0,0,0,0), 8, 172);//++++++ ???

	for (int i=0; i<12; i++) buttons[i] = 0;

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
    buttons[0] = new AudioEnabledToggle(this, colx[2], rowy[0], audio_enabled);
	// Ok
	buttons[9] = new AudioOptions_button(this, oktext, colx[0], rowy[12]);
	// Cancel
	buttons[10] = new AudioOptions_button(this, canceltext, colx[2], rowy[12]);
}

AudioOptions_gump::~AudioOptions_gump()
{
	for (int i=0; i<12; i++)
		if (buttons[i]) delete buttons[i];
}

void AudioOptions_gump::save_settings()
{
	Audio::get_ptr()->set_audio_enabled(audio_enabled == 1);
	Audio::get_ptr()->set_music_enabled(midi_enabled == 1);
	if (!midi_enabled)		// Stop what's playing.
		Audio::get_ptr()->stop_music();
	Audio::get_ptr()->set_effects_enabled(sfx_enabled != 0);
	if (!sfx_enabled)		// Stop what's playing.
		Audio::get_ptr()->stop_sound_effects();
	Audio::get_ptr()->set_speech_enabled(speech_enabled == 1);
	Audio::get_ptr()->set_allow_music_looping(midi_looping == 1);

	config->set("config/audio/enabled", audio_enabled ? "yes" : "no", true);
	config->set("config/audio/midi/enabled",midi_enabled ? "yes" : "no", true);
	config->set("config/audio/effects/enabled",sfx_enabled?"yes":"no", true);
	config->set("config/audio/speech/enabled",speech_enabled?"yes":"no", true);

	config->set("config/audio/midi/chorus/enabled", (midi_reverb_chorus&2) ? "yes" : "no", true);
	config->set("config/audio/midi/reverb/enabled", (midi_reverb_chorus&1)? "yes" : "no", true);
	config->set("config/audio/midi/looping", midi_looping ? "yes" : "no", true);

	std::string d = "config/disk/game/" + Game::get_gametitle() + "/waves";
	std::string waves;
	int i = 0;
	if (have_roland_pack && sfx_package == i++)
		waves = rolandpack;
	else if (have_blaster_pack && sfx_package == i++)
		waves = blasterpack;
	else if (have_custom_pack && sfx_package == i++)
		waves = sfx_custompack;
	config->set(d.c_str(), waves, true);
#ifdef ENABLE_MIDISFX
	config->set("config/audio/effects/midi",sfx_enabled==2?"yes":"no",true);
#endif

	if (Audio::get_ptr()->get_midi()) {
		std::string s = "default";
		if (midi_driver != MidiDriver::getDriverCount()) s = MidiDriver::getDriverName(midi_driver);
		Audio::get_ptr()->get_midi()->set_midi_driver(s,midi_ogg_enabled!=0);
		Audio::get_ptr()->get_midi()->set_music_conversion(midi_conversion);
#ifdef ENABLE_MIDISFX
		Audio::get_ptr()->get_midi()->set_effects_conversion(sfx_conversion);
#endif
	} else {
		switch(midi_conversion) {
		case XMIDIFILE_CONVERT_MT32_TO_GS:
			config->set("config/audio/midi/convert","gs",true);
			break;
		case XMIDIFILE_CONVERT_NOCONVERSION:
			config->set("config/audio/midi/convert","mt32",true);
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

		if (midi_driver == MidiDriver::getDriverCount())
			config->set("config/audio/midi/driver","default",true);
		else
			config->set("config/audio/midi/driver",MidiDriver::getDriverName(midi_driver),true);

#ifdef ENABLE_MIDISFX
		switch(sfx_conversion) {
		case XMIDIFILE_CONVERT_NOCONVERSION:
			config->set("config/audio/effects/convert","mt32",true);
			break;
		default:
			config->set("config/audio/effects/convert","gs",true);
			break;
		}
#endif
	}

	Audio::get_ptr()->Init_sfx();
}

void AudioOptions_gump::paint()
{
	Gump::paint();
	for (int i=0; i<12; i++)
		if (buttons[i])
			buttons[i]->paint();

	sman->paint_text(2, "Audio:", x + colx[0], y + rowy[0] + 1);
	if (audio_enabled) {
		sman->paint_text(2, "Music options:", x + colx[0], y + rowy[1] + 1);
		if (midi_enabled) {
			sman->paint_text(2, "looping", x + colx[1], y + rowy[2] + 1);
			sman->paint_text(2, "digital music", x + colx[1], y + rowy[3] + 1);
			sman->paint_text(2, "midi driver", x + colx[1], y + rowy[4] + 1);
			if (buttons[3]) sman->paint_text(2, "device type", x+colx[1], y+rowy[5] + 1);
			if (buttons[4]) sman->paint_text(2, "effects", x + colx[1], y + rowy[6] + 1);
		}
		sman->paint_text(2, "SFX options:", x + colx[0], y + rowy[7] + 1);
		sman->paint_text(2, "SFX", x + colx[1], y + rowy[8] + 1);
		if (sfx_enabled == 1) {
			sman->paint_text(2, "pack", x + colx[1], y + rowy[9] + 1);
		}
#ifdef ENABLE_MIDISFX
		else if (sfx_enabled == 2) {
			sman->paint_text(2, "conversion", x + colx[1], y + rowy[9] + 1);
		}
#endif
		sman->paint_text(2, "Speech options:", x + colx[0], y + rowy[10] + 1);
		sman->paint_text(2, "speech", x + colx[1], y + rowy[11] + 1);
	}
	gwin->set_painted();
}

void AudioOptions_gump::mouse_down(int mx, int my)
{
	pushed = Gump::on_button(mx, my);
					// First try checkmark.
	// Try buttons at bottom.
	if (!pushed)
		for (int i=0; i<12; i++)
			if (buttons[i] && buttons[i]->on_button(mx, my)) {
				pushed = buttons[i];
				break;
			}

	if (pushed)			// On a button?
	{
		pushed->push();
		return;
	}
}

void AudioOptions_gump::mouse_up(int mx, int my)
{
	if (pushed)			// Pushing a button?
	{
		pushed->unpush();
		if (pushed->on_button(mx, my))
			((Gump_button*)pushed)->activate();
		pushed = 0;
	}
}
