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

#ifndef _AUDIOOPTIONS_GUMP_H
#define _AUDIOOPTIONS_GUMP_H

#include "Modal_gump.h"

class Gump_button;

class AudioOptions_gump : public Modal_gump
{
	UNREPLICATABLE_CLASS_I(AudioOptions_gump,Modal_gump(0,0,0,0));

 private:
	Gump_button* buttons[11];

	int audio_enabled;
	int midi_enabled;
	int midi_conversion;
	int midi_reverb;
	int midi_chorus;
	int midi_looping;
	int sfx_enabled;
	int sfx_conversion;
	int speech_enabled;

 public:
	AudioOptions_gump();
	~AudioOptions_gump();

					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	virtual void close(Game_window *gwin);

 					// Handle events:
	virtual void mouse_down(int mx, int my);
	virtual void mouse_up(int mx, int my);

	void toggle(Gump_button* btn, int state);
	void build_buttons();
	void build_midi_buttons();
	void build_sfx_buttons();

	void load_settings();
	void save_settings();
	void cancel();
};

#endif
