/*
Copyright (C) 2003-2011 The Exult Team

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

#ifndef _AUDIOOPTIONS_GUMP_H
#define _AUDIOOPTIONS_GUMP_H

#include "Modal_gump.h"
#include <string>

class Gump_button;

class AudioOptions_gump : public Modal_gump
{
	UNREPLICATABLE_CLASS_I(AudioOptions_gump,Modal_gump(0,0,0,0));

 private:
	enum button_ids
		{
		id_first = 0,
		id_ok = id_first,
		id_cancel,
		id_audio_enabled,
		id_sample_rate,
		id_speaker_type,
		id_music_enabled,
		id_music_looping,
		id_music_digital,
		id_midi_driver,
		id_midi_conv,
		id_midi_effects,
		id_sfx_enabled,
		id_sfx_pack,
		id_sfx_conv = id_sfx_pack,
		id_speech_enabled,
		id_count,
		};
	Gump_button* buttons[id_count];

	bool speaker_type; // only mono and stereo atm
	bool o_speaker_type;
	std::string sample_rate_str;
	static uint32 sample_rates[5];
	static int num_sample_rates;
	int audio_enabled;
	int sample_rate;
	int o_sample_rate;
	int midi_enabled;
	int midi_conversion;
	int midi_ogg_enabled;
	int midi_driver;
	int midi_reverb_chorus;
	int midi_looping;
	int sfx_enabled;
	int sfx_package;
	int sfx_conversion;
	int speech_enabled;

	// Auxiliary variables for digital SFX packages:
	int nsfxopts, nsfxpacks;
	int midi_state;
	bool have_config_pack, have_roland_pack, have_blaster_pack,
	     have_midi_pack, have_custom_pack;
	std::string sfx_custompack;
	std::string configpack, rolandpack, blasterpack, midipack;

	bool have_digital_sfx() const
		{ return have_roland_pack || have_blaster_pack || have_custom_pack; }

 public:
	AudioOptions_gump();
	virtual ~AudioOptions_gump();

					// Paint it and its contents.
	virtual void paint();
	virtual void close();

 					// Handle events:
	virtual bool mouse_down(int mx, int my, int button);
	virtual bool mouse_up(int mx, int my, int button);

	void toggle(Gump_button* btn, int state);
	void rebuild_buttons();
	void rebuild_midi_buttons();
	void rebuild_mididriveroption_buttons();
	void rebuild_sfx_buttons();

	void load_settings();
	void save_settings();
	void cancel();
};

#endif
