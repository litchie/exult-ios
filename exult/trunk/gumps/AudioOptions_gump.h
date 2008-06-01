/*
Copyright (C) 2003-2004 The Exult Team

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
	Gump_button* buttons[12];

	int audio_enabled;
	int midi_enabled;
	int midi_conversion;
	int midi_ogg_enabled;
	int midi_driver;
	int midi_reverb_chorus;
	int midi_looping;
	int sfx_enabled;
	int sfx_conversion;
	int speech_enabled;

 public:
	AudioOptions_gump();
	virtual ~AudioOptions_gump();

					// Paint it and its contents.
	virtual void paint();
	virtual void close();

 					// Handle events:
	virtual void mouse_down(int mx, int my);
	virtual void mouse_up(int mx, int my);

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
