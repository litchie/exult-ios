/*
Copyright (C) 2001 The Exult Team

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

#include "SDL_events.h"

#include "AudioOptions_gump.h"
#include "mouse.h"
#include "gump_utils.h"
#include "Configuration.h"
#include <iostream>
#include "xmidi.h"
#include "Audio.h"
#include "Gump_button.h"
#include "Gump_ToggleButton.h"
#include "gamewin.h"

using std::cerr;
using std::endl;
using std::string;

extern Configuration *config;

static const int rowy[] = { 5,  19, 29, 41, 53, 65,  79, 89, 101,  
							115, 125,  146 };
static const int colx[] = { 35, 55, 130 };

class AudioOptions_button : public Gump_button {
public:
	AudioOptions_button(Gump *par, int px, int py, int shapenum)
		: Gump_button(par, shapenum, px, py, GSF_EXULT_FLX)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
};

void AudioOptions_button::activate(Game_window *gwin)
{
	switch (shapenum) {
	case 47: // cancel
		((AudioOptions_gump*)parent)->cancel();
		break;
	case 46: // ok
		((AudioOptions_gump*)parent)->close(gwin);
		break;
	}
}

class AudioToggle : public Gump_ToggleButton {
public:
	AudioToggle(Gump* par, int px, int py, int shapenum, 
				int selectionnum, int numsel)
		: Gump_ToggleButton(par, px, py, shapenum, selectionnum, numsel) {}

	friend class AudioOptions_gump;
	virtual void toggle(int state) { 
		((AudioOptions_gump*)parent)->toggle((Gump_button*)this, state);
	}
};


void AudioOptions_gump::close(Game_window* gwin)
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
		if (state == 0) {
			for (int i=1; i<8; i++) {
				if (buttons[i]) delete buttons[i];
				buttons[i] = 0;
			}
		} else {
			build_buttons();
		}
		paint(Game_window::get_game_window());
	} else if (btn == buttons[1]) {	// midi on/off 
		midi_enabled = state;
		if (state == 0) {
			for (int i=2; i<5; i++) {
				if (buttons[i]) delete buttons[i];
				buttons[i] = 0;
			}
		} else {
			build_midi_buttons();
		}
		paint(Game_window::get_game_window());
	} else if (btn == buttons[2]) { // midi conversion
		midi_conversion = state;
	} else if (btn == buttons[3]) { // midi reverb
		midi_reverb = state;
	} else if (btn == buttons[4]) { // midi chorus
		midi_chorus = state;
	} else if (btn == buttons[5]) { // sfx on/off
		sfx_enabled = state;
		if (state == 0) {
			if (buttons[6]) delete buttons[6];
			buttons[6] = 0;
		} else {
			build_sfx_buttons();
		}
		paint(Game_window::get_game_window());
#ifdef ENABLE_MIDISFX
	} else if (btn == buttons[6]) { // sfx conversion
		if (state == 1) {
			((AudioToggle*)buttons[6])->framenum = 4;
			sfx_conversion = XMIDI_CONVERT_GS127_TO_GS;
		} else {
			((AudioToggle*)buttons[6])->framenum = 0;
			sfx_conversion = XMIDI_CONVERT_NOCONVERSION;
		}
#endif
	} else if (btn == buttons[7]) { // speech on/off
		speech_enabled = state;
	}
}

void AudioOptions_gump::build_buttons()
{
	// audio on/off
    buttons[0] = new AudioToggle(this, colx[2], rowy[0], 44, audio_enabled, 2);

	if (audio_enabled) {

		// midi on/off
		buttons[1] = new AudioToggle(this, colx[2], rowy[2],44,midi_enabled,2);
		if (midi_enabled)
			build_midi_buttons();

		// sfx on/off
		buttons[5] = new AudioToggle(this, colx[2], rowy[7], 44,sfx_enabled,2);
		if (sfx_enabled)
			build_sfx_buttons();

		// speech on/off
		buttons[7] =new AudioToggle(this,colx[2],rowy[10],44,speech_enabled,2);
	}
}

void AudioOptions_gump::build_midi_buttons()
{
	// midi conversion
	buttons[2] = new AudioToggle(this, colx[2], rowy[3], 45,midi_conversion,4);
	// reverb on/off
	buttons[3] = new AudioToggle(this, colx[2], rowy[4], 44, midi_reverb, 2);
	// chorus on/off
	buttons[4] = new AudioToggle(this, colx[2], rowy[5], 44, midi_chorus, 2);
}

void AudioOptions_gump::build_sfx_buttons()
{
#ifdef ENABLE_MIDISFX
	// sfx conversion
	buttons[6] = new AudioToggle(this, colx[2], rowy[8],45,sfx_conversion/2,2);
#endif
}

void AudioOptions_gump::load_settings()
{
	string s;
	audio_enabled = (Audio::get_ptr()->is_audio_enabled() ? 1 : 0);
	midi_enabled = (Audio::get_ptr()->is_music_enabled() ? 1 : 0);
	sfx_enabled = (Audio::get_ptr()->are_effects_enabled() ? 1 : 0);
	speech_enabled = (Audio::get_ptr()->is_speech_enabled() ? 1 : 0);

	if (Audio::get_ptr()->get_midi()) {
		midi_conversion = Audio::get_ptr()->get_midi()->get_music_conversion();
#ifdef ENABLE_MIDISFX
		sfx_conversion =Audio::get_ptr()->get_midi()->get_effects_conversion();
#endif
	} else {
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
			midi_conversion = XMIDI_CONVERT_MT32_TO_GM;

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
	midi_reverb = (s == "yes" ? 1 : 0);

	config->value("config/audio/midi/chorus/enabled",s,"no");
	midi_chorus = (s == "yes" ? 1 : 0);
}

AudioOptions_gump::AudioOptions_gump() : Modal_gump(0, 43, GSF_EXULT_FLX)
{
	for (int i=0; i<10; i++) buttons[i] = 0;

	load_settings();
	
	build_buttons();

	// Ok
	buttons[8] = new AudioOptions_button(this, colx[0], rowy[11], 46);
	// Cancel
	buttons[9] = new AudioOptions_button(this, colx[2], rowy[11], 47);
}

AudioOptions_gump::~AudioOptions_gump()
{
	for (int i=0; i<10; i++)
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

	config->set("config/audio/enabled", audio_enabled ? "yes" : "no", true);
	config->set("config/audio/midi/enabled",midi_enabled ? "yes" : "no", true);
	config->set("config/audio/effects/enabled",sfx_enabled?"yes":"no", true);
	config->set("config/audio/speech/enabled",speech_enabled?"yes":"no", true);

	config->set("config/audio/midi/chorus/enabled", midi_chorus ? "yes" : "no", true);
	config->set("config/audio/midi/reverb/enabled", midi_reverb ? "yes" : "no", true);

	if (Audio::get_ptr()->get_midi()) {
		Audio::get_ptr()->get_midi()->set_music_conversion(midi_conversion);
		Audio::get_ptr()->get_midi()->set_effects_conversion(sfx_conversion);
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

void AudioOptions_gump::paint(Game_window* gwin)
{
	Gump::paint(gwin);
	for (int i=0; i<10; i++)
		if (buttons[i])
			paint_button(gwin, buttons[i]);

	gwin->paint_text(2, "Audio:", x + colx[0], y + rowy[0] + 1);
	if (audio_enabled) {
		gwin->paint_text(2, "Music options:", x + colx[0], y + rowy[1] + 1);
		gwin->paint_text(2, "music", x + colx[1], y + rowy[2] + 1);
		if (midi_enabled) {
			gwin->paint_text(2, "conversion", x + colx[1], y + rowy[3] + 1);
			gwin->paint_text(2, "reverb", x + colx[1], y + rowy[4] + 1);
			gwin->paint_text(2, "chorus", x + colx[1], y + rowy[5] + 1);
		}
		gwin->paint_text(2, "SFX options:", x + colx[0], y + rowy[6] + 1);
		gwin->paint_text(2, "SFX", x + colx[1], y + rowy[7] + 1);
#ifdef ENABLE_MIDISFX
		if (sfx_enabled) {
			gwin->paint_text(2, "conversion", x + colx[1], y + rowy[8] + 1);
		}
#endif
		gwin->paint_text(2, "Speech options:", x + colx[0], y + rowy[9] + 1);
		gwin->paint_text(2, "speech", x + colx[1], y + rowy[10] + 1);
	}
	gwin->set_painted();
}

void AudioOptions_gump::mouse_down(int mx, int my)
{
	Game_window *gwin = Game_window::get_game_window();
	pushed = Gump::on_button(gwin, mx, my);
					// First try checkmark.
	// Try buttons at bottom.
	if (!pushed)
		for (int i=0; i<10; i++)
			if (buttons[i] && buttons[i]->on_button(gwin, mx, my)) {
				pushed = buttons[i];
				break;
			}

	if (pushed)			// On a button?
	{
		pushed->push(gwin);
		return;
	}
}

void AudioOptions_gump::mouse_up(int mx, int my)
{
	Game_window *gwin = Game_window::get_game_window();
	if (pushed)			// Pushing a button?
	{
		pushed->unpush(gwin);
		if (pushed->on_button(gwin, mx, my))
			((Gump_button*)pushed)->activate(gwin);
		pushed = 0;
	}
}
