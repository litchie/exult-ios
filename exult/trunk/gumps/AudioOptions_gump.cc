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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

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
#include "xmidi.h"
#include "Enabled_button.h"

using std::cerr;
using std::endl;

static const int rowy[] = { 5,  
			    19, 29, 41, 53, 65, 77,  
			    89, 101,  
			    115, 125,  
			    135, 156 };
static const int colx[] = { 35, 55, 130 };

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
	} else if (btn == buttons[2]) { // midi driver
		midi_driver = state;
		rebuild_mididriveroption_buttons();
		paint();
	} else if (btn == buttons[3]) { // midi conversion
		midi_conversion = state;
	} else if (btn == buttons[4]) { // midi reverb/chorus
		midi_reverb_chorus = state;
	} else if (btn == buttons[5]) { // midi looping
		midi_looping = state;
	} else if (btn == buttons[6]) { // sfx on/off
		sfx_enabled = state;
		rebuild_sfx_buttons();
		paint();
#ifdef ENABLE_MIDISFX
	} else if (btn == buttons[7]) { // sfx conversion
		if (state == 1) {
			sfx_conversion = XMIDI_CONVERT_GS127_TO_GS;
		} else {
			sfx_conversion = XMIDI_CONVERT_NOCONVERSION;
		}
#endif
	} else if (btn == buttons[8]) { // speech on/off
		speech_enabled = state;
	}
}

void AudioOptions_gump::rebuild_buttons()
{
	for (unsigned int i = 1; i < 9; ++i) {
		delete buttons[i];
		buttons[i] = 0;
	}

	if (!audio_enabled) return;

	// midi on/off
	buttons[1] = new AudioEnabledToggle(this, colx[2], rowy[2], midi_enabled);
	if (midi_enabled)
		rebuild_midi_buttons();
	
	// sfx on/off
	buttons[6] = new AudioEnabledToggle(this, colx[2], rowy[8], sfx_enabled);
	if (sfx_enabled)
		rebuild_sfx_buttons();
	
	// speech on/off
	buttons[8] = new AudioEnabledToggle(this,colx[2],rowy[11], speech_enabled);
}

void AudioOptions_gump::rebuild_midi_buttons()
{
	for (unsigned int i = 2; i < 6; ++i) {
		delete buttons[i];
		buttons[i] = 0;
	}

	if (!midi_enabled) return;

	std::string* midi_drivertext = new std::string[NUM_MIDI_DRIVER_TYPES];
	midi_drivertext[MIDI_DRIVER_NORMAL] = std::string("Normal");
	midi_drivertext[MIDI_DRIVER_OGG] = std::string("Digital");
#ifdef USE_FMOPL_MIDI
	midi_drivertext[MIDI_DRIVER_FMSYNTH] = std::string("FMSynth");
#endif
#ifdef USE_MT32EMU_MIDI
	midi_drivertext[MIDI_DRIVER_MT32EMU] = std::string("MT32Emu");
#endif


	// midi driver
	buttons[2] = new AudioTextToggle(this, midi_drivertext, 
									 colx[2], rowy[3], 59, midi_driver, NUM_MIDI_DRIVER_TYPES);

	rebuild_mididriveroption_buttons();

	// looping on/off
	buttons[5] = new AudioEnabledToggle(this, colx[2], rowy[6], midi_looping);

}


void AudioOptions_gump::rebuild_sfx_buttons()
{
	delete buttons[7];
	buttons[7] = 0;

	if (!sfx_enabled)
		return;

#ifdef ENABLE_MIDISFX
	std::string* sfx_conversiontext = new std::string[2];
	sfx_conversiontext[0] = "None";
	sfx_conversiontext[1] = "GS";

	// sfx conversion
	buttons[7] = new AudioTextToggle(this, sfx_conversiontext, colx[2], rowy[9],
									 59, sfx_conversion/4,2);
#endif
}

void AudioOptions_gump::rebuild_mididriveroption_buttons()
{
	delete buttons[3];
	buttons[3] = 0;
	delete buttons[4];
	buttons[4] = 0;

	std::string* midi_conversiontext = new std::string[4];
	midi_conversiontext[0] = std::string("None");
	midi_conversiontext[1] = std::string("GM");
	midi_conversiontext[2] = std::string("GS");
	midi_conversiontext[3] = std::string("GS127");

	std::string* midi_reverbchorustext = new std::string[4];
	midi_reverbchorustext[0] = std::string("Disabled");
	midi_reverbchorustext[1] = std::string("Reverb");
	midi_reverbchorustext[2] = std::string("Chorus");
	midi_reverbchorustext[3] = std::string("Both");

	if (midi_driver == MIDI_DRIVER_NORMAL) {
		// midi conversion
		buttons[3] = new AudioTextToggle(this, midi_conversiontext, 
										 colx[2], rowy[4], 59,
										 midi_conversion, 4);
		// reverb/chorus combo
		buttons[4] = new AudioTextToggle(this, midi_reverbchorustext, 
										 colx[2], rowy[5], 59,
										 midi_reverb_chorus, 4);
	}
}

void AudioOptions_gump::load_settings()
{
	std::string s;
	audio_enabled = (Audio::get_ptr()->is_audio_enabled() ? 1 : 0);
	midi_enabled = (Audio::get_ptr()->is_music_enabled() ? 1 : 0);
	sfx_enabled = (Audio::get_ptr()->are_effects_enabled() ? 1 : 0);
	speech_enabled = (Audio::get_ptr()->is_speech_enabled() ? 1 : 0);
	midi_looping = (Audio::get_ptr()->is_music_looping_allowed() ? 1 : 0);

	if (Audio::get_ptr()->get_midi()) {
		midi_conversion = Audio::get_ptr()->get_midi()->get_music_conversion();
		midi_driver = Audio::get_ptr()->get_midi()->get_output_driver_type();
#ifdef ENABLE_MIDISFX
		sfx_conversion =Audio::get_ptr()->get_midi()->get_effects_conversion();
#endif
	} else {
		// String for default value for driver type
		std::string driver_default = "normal";

		config->value("config/audio/midi/convert",s,"gm");
		if (s == "gs")
			midi_conversion = XMIDI_CONVERT_MT32_TO_GS;
		else if (s == "none")
			midi_conversion = XMIDI_CONVERT_NOCONVERSION;
		else if (s == "gs127")
			midi_conversion = XMIDI_CONVERT_MT32_TO_GS127;
		else if (s == "gs127drum")
			midi_conversion = XMIDI_CONVERT_MT32_TO_GS;
		else
		{
			midi_conversion = XMIDI_CONVERT_MT32_TO_GM;
			config->set("config/audio/midi/convert","gm",true);

			driver_default = "s";
		}

		config->value("config/audio/midi/driver",s,driver_default.c_str());
		if (s == "digital") 
		{
			midi_driver = MIDI_DRIVER_OGG;
			config->set("config/audio/effects/driver","digital",true);
		}
#ifdef USE_FMOPL_MIDI
		else if (s == "fmsynth")
		{
			midi_driver = MIDI_DRIVER_FMSYNTH;
			config->set("config/audio/effects/driver","fmsynth",true);
		}
#endif
#ifdef USE_MT32EMU_MIDI
		else if (s == "mt32emu")
			midi_driver = MIDI_DRIVER_MT32EMU;
#endif
		else
			midi_driver = MIDI_DRIVER_NORMAL;


#ifdef ENABLE_MIDISFX
		config->value("config/audio/effects/convert",s,"gs");
		if (s == "none")
			sfx_conversion = XMIDI_CONVERT_NOCONVERSION;
		else if (s == "gs127")
			sfx_conversion = XMIDI_CONVERT_NOCONVERSION;
		else
			sfx_conversion = XMIDI_CONVERT_GS127_TO_GS;
#endif
	}

	config->value("config/audio/midi/reverb/enabled",s,"no");
	midi_reverb_chorus = (s == "yes" ? 1 : 0);

	config->value("config/audio/midi/chorus/enabled",s,"no");
	midi_reverb_chorus |= (s == "yes" ? 2 : 0);
	
	
}

AudioOptions_gump::AudioOptions_gump() : Modal_gump(0, EXULT_FLX_AUDIOOPTIONS_SHP, SF_EXULT_FLX)
{
	set_object_area(Rectangle(0,0,0,0), 8, 172);//++++++ ???

	for (int i=0; i<12; i++) buttons[i] = 0;

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
	Audio::get_ptr()->set_effects_enabled(sfx_enabled == 1);
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

	if (Audio::get_ptr()->get_midi()) {
		Audio::get_ptr()->get_midi()->set_music_conversion(midi_conversion);
		Audio::get_ptr()->get_midi()->set_effects_conversion(sfx_conversion);
		Audio::get_ptr()->get_midi()->set_output_driver_type(midi_driver);
	} else {
		switch(midi_conversion) {
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

		switch(midi_driver) {
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
		}

#ifdef ENABLE_MIDISFX
		switch(sfx_conversion) {
		case XMIDI_CONVERT_NOCONVERSION:
			config->set("config/audio/effects/convert","none",true);
			break;
		default:
			config->set("config/audio/effects/convert","gs",true);
			break;
		}
#endif
	}
}

void AudioOptions_gump::paint()
{
	Gump::paint();
	for (int i=0; i<11; i++)
		if (buttons[i])
			buttons[i]->paint();

	sman->paint_text(2, "Audio:", x + colx[0], y + rowy[0] + 1);
	if (audio_enabled) {
		sman->paint_text(2, "Music options:", x + colx[0], y + rowy[1] + 1);
		sman->paint_text(2, "music", x + colx[1], y + rowy[2] + 1);
		if (midi_enabled) {
			sman->paint_text(2, "driver", x + colx[1], y + rowy[3] + 1);
			if (midi_driver == MIDI_DRIVER_NORMAL) {
				sman->paint_text(2, "conversion", x+colx[1], y+rowy[4] + 1);
				sman->paint_text(2, "effects", x + colx[1], y + rowy[5] + 1);
			}
			sman->paint_text(2, "looping", x + colx[1], y + rowy[6] + 1);
		}
		sman->paint_text(2, "SFX options:", x + colx[0], y + rowy[7] + 1);
		sman->paint_text(2, "SFX", x + colx[1], y + rowy[8] + 1);
#ifdef ENABLE_MIDISFX
		if (sfx_enabled) {
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
		for (int i=0; i<11; i++)
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
