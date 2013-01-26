/*
Copyright (C) 2000-2013 The Exult Team

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

#ifndef _YESNO_GUMP_H_
#define _YESNO_GUMP_H_

#include <string>

#include "Modal_gump.h"

#include <SDL.h>

class Yesno_button;

/*
 *  A yes/no box.
 */
class Yesno_gump : public Modal_gump {
	UNREPLICATABLE_CLASS_I(Yesno_gump, Modal_gump(0, 0, 0, 0));

protected:
	static short yesx, yesnoy, nox; // Coords. of the buttons.
	std::string text;           // Text of question.  It is drawn in
	//   object_area.
	const char *fontname;
	int answer;         // 1 for yes, 0 for no.
	void set_answer(int y) {    // Done from 'yes'/'no' button.
		answer = (y != 0);
		done = 1;
	}

public:
	friend class Yesno_button;
	Yesno_gump(const std::string &txt, const char *font = "SMALL_BLACK_FONT");
	virtual ~Yesno_gump();
	int get_answer() {
		return answer;
	}
	// Paint it and its contents.
	virtual void paint();
	// Handle events:
	virtual bool mouse_down(int mx, int my, int button);
	virtual bool mouse_up(int mx, int my, int button);
	virtual void key_down(int chr); // Character typed.
	static int ask(const char *txt, const char *font = "SMALL_BLACK_FONT"); // Ask question, get answer.
};

class Countdown_gump : public Yesno_gump {
	std::string text_fmt;           // Text of question.  It is drawn in
	int timer;
	int start_time;
public:
	Countdown_gump(const std::string &txt, int timeout, const char *font);

	virtual bool run();

	static int ask(const char *txt, int timeout, const char *font = "SMALL_BLACK_FONT"); // Ask question, get answer, timeout to no after timeout seconds
};
#endif
