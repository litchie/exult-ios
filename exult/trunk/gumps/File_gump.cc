/*
 *  Copyright (C) 2000-2002  The Exult Team
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

#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#endif 

#include "SDL_events.h"

#include "Audio.h"
#include "Configuration.h"
#include "File_gump.h"
#include "exult.h"
#include "game.h"
#include "gamewin.h"
#include "Gump_button.h"
#include "mouse.h"
#include "Yesno_gump.h"

using std::cout;
using std::endl;
using std::memmove;
using std::string;
using std::strlen;
using std::strncpy;

/*
 *	Statics:
 */
short File_gump::btn_rows[2] = {143, 156};
short File_gump::btn_cols[3] = {94, 163, 232};
short File_gump::textx = 237, File_gump::texty = 14,
      File_gump::texth = 13;


/*
 *	Load or save button.
 */
class Load_save_button : public Gump_button
{
public:
	Load_save_button(Gump *par, int px, int py, int shapenum)
		: Gump_button(par, shapenum, px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate();
};

/*
 *	Quit button.
 */
class Quit_button : public Gump_button
{
public:
	Quit_button(Gump *par, int px, int py)
		: Gump_button(par, 
			game->get_shape("gumps/quitbtn"), px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate();
};

/*
 *	Sound 'toggle' buttons.
 */
class Sound_button : public Gump_button
{
public:
	Sound_button(Gump *par, int px, int py, int shapenum,
								bool enabled)
	  : Gump_button(par, shapenum, px, py)
		{ pushed = enabled; }
					// What to do when 'clicked':
	virtual void activate();
};

/*
 *	Clicked a 'load' or 'save' button.
 */

void Load_save_button::activate
	(
	)
{
	if (get_shapenum() == game->get_shape("gumps/loadbtn"))
		((File_gump *) parent)->load();
	else
		((File_gump *) parent)->save();
}

/*
 *	Clicked on 'quit'.
 */

void Quit_button::activate
	(
	)
{
	((File_gump *) parent)->quit();
}

/*
 *	Clicked on one of the sound options.
 */

void Sound_button::activate
	(
	)
{
	pushed = ((File_gump *) parent)->toggle_option(this);
	parent->paint();
}


/*
 *	An editable text field:
 */
class Gump_text : public Gump_widget
{
	char *text;			// Holds text, 0-delimited.
	int max_size;			// Size (max) of text.
	int length;			// Current # chars.
	int textx, texty;		// Where to show text rel. to parent.
	int cursor;			// Index of char. cursor is before.
public:
	Gump_text(Gump *par, int shnum, int px, int py, int maxsz,
						int tx, int ty)
		: Gump_widget(par, shnum, px, py), text(new char[maxsz + 1]),
		  max_size(maxsz), length(0), textx(x + tx), texty(y + ty),
		  cursor(0)
		{
		text[0] = text[maxsz] = 0;
		Shape_frame *shape = 
				ShapeID(shnum, 0, SF_GUMPS_VGA).get_shape();
					// Want text coords. rel. to parent.
		textx -= shape->get_xleft();
		texty -= shape->get_yabove();
		}
	~Gump_text()
		{ delete [] text; }
	int get_length()
		{ return length; }
	char *get_text()
		{ return text; }
	void set_text(char *newtxt)	// Set text.
		{
			strncpy(text, newtxt ? newtxt : "", max_size);
			length = strlen(text);
		}
	int get_cursor()
		{ return cursor; }
	void set_cursor(int pos)	// Set cursor (safely).
		{
			if (pos >= 0 && pos <= length)
			{
				cursor = pos;
				refresh();
			}
		}
	void paint();			// Paint.
					// Handle mouse click.
	int mouse_clicked(int mx, int my);
	void insert(int chr);		// Insert a character.
	int delete_left();		// Delete char. to left of cursor.
	int delete_right();		// Delete char. to right of cursor.
	void lose_focus();

protected:
	void refresh()
		{
			paint();
		}
};

/*
 *	Paint text field.
 */

void Gump_text::paint
	(
	)
	{
	paint_shape(parent->get_x() + x, parent->get_y() + y);
					// Show text.
	sman->paint_text(2, text, parent->get_x() + textx,
						parent->get_y() + texty);
	if (get_framenum())			// Focused?  Show cursor.
		gwin->get_win()->fill8(0, 1, sman->get_text_height(2),
			parent->get_x() + textx +
					sman->get_text_width(2, text, cursor),
				parent->get_y() + texty + 1);
	gwin->set_painted();
	}

/*
 *	Handle click on text object.
 *
 *	Output:	1 if point is within text object, else 0.
 */

int Gump_text::mouse_clicked
	(
	int mx, int my			// Mouse position on screen.
	)
	{
	if (!on_widget(mx, my))		// Not in our area?
		return (0);
	mx -= textx + parent->get_x();	// Get pt. rel. to text area.
	if (!get_framenum())		// Gaining focus?
		{
		set_frame(1);		// We have focus now.
		cursor = 0;		// Put cursor at start.
		}
	else
		{
		for (cursor = 0; cursor <= length; cursor++)
			if (sman->get_text_width(2, text, cursor) > mx)
				{
				if (cursor > 0)
					cursor--;
				break;
				}
		if (cursor > length)
			cursor--;	// Passed the end.
		}
	return (1);
	}

/*
 *	Insert a character at the cursor.
 */

void Gump_text::insert
	(
	int chr
	)
	{
	if (!get_framenum() || length == max_size)
		return;			// Can't.
	if (cursor < length)		// Open up space.
		memmove(text + cursor + 1, text + cursor, length - cursor);
	text[cursor++] = chr;		// Store, and increment cursor.
	length++;
	text[length] = 0;
	refresh();
	}

/*
 *	Delete the character to the left of the cursor.
 *
 *	Output:	1 if successful.
 */

int Gump_text::delete_left
	(
	)
	{
	if (!get_framenum() || !cursor)		// Can't do it.
		return (0);
	if (cursor < length)		// Shift text left.
		memmove(text + cursor - 1, text + cursor, length - cursor);
	text[--length] = 0;		// 0-delimit.
	cursor--;
	refresh();
	return (1);
	}

/*
 *	Delete char. to right of cursor.
 *
 *	Output:	1 if successful.
 */

int Gump_text::delete_right
	(
	)
	{
	if (!get_framenum() || cursor == length)
		return (0);		// Past end of text.
	cursor++;			// Move right.
	return (delete_left());		// Delete what was passed.
	}

/*
 *	Lose focus.
 */

void Gump_text::lose_focus
	(
	)
	{
	set_frame(0);
	refresh();
	}

/*
 *	Create the load/save box.
 */

File_gump::File_gump
	(
	) : Modal_gump(0, game->get_shape("gumps/fileio")),
		pushed_text(0), focus(0), restored(0)
{
	set_object_area(Rectangle(0,0,0,0), 8, 150);

	size_t i;
	int ty = texty;
	for (i = 0; i < sizeof(names)/sizeof(names[0]); i++, ty += texth)
	{
		names[i] = new Gump_text(this, 
			game->get_shape("gumps/fntext"), 
							textx, ty, 30, 12, 2);
		names[i]->set_text(gwin->get_save_name(i));
	}
					// First row of buttons:
	buttons[0] = buttons[1] = 0;	// No load/save until name chosen.
	buttons[2] = new Quit_button(this, btn_cols[2], btn_rows[0]);
					// 2nd row.
	buttons[3] = new Sound_button(this, btn_cols[0], btn_rows[1], 
			game->get_shape("gumps/musicbtn"),
						Audio::get_ptr()->is_music_enabled());
	buttons[4] = new Sound_button(this, btn_cols[1], btn_rows[1],
			game->get_shape("gumps/speechbtn"),
						Audio::get_ptr()->is_speech_enabled());
	buttons[5] = new Sound_button(this, btn_cols[2], btn_rows[1],
			game->get_shape("gumps/soundbtn"),
						Audio::get_ptr()->are_effects_enabled());
}

/*
 *	Delete the load/save box.
 */

File_gump::~File_gump
	(
	)
{
	size_t i;
	for (i = 0; i < sizeof(names)/sizeof(names[0]); i++)
		delete names[i];
	for (i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
		delete buttons[i];
}

/*
 *	Get the index of one of the text fields (savegame #).
 *
 *	Output:	Index, or -1 if not found.
 */

int File_gump::get_save_index
	(
	Gump_text *txt
	)
{
	for (size_t i = 0; i < sizeof(names)/sizeof(names[0]); i++)
		if (names[i] == txt)
			return (i);
	return (-1);
}

/*
 *	Remove text focus.
 */

void File_gump::remove_focus
	(
	)
	{
	if (!focus)
		return;
	focus->lose_focus();
	focus = 0;
	delete buttons[0];		// Remove load/save buttons.
	buttons[0] = 0;
	delete buttons[1];
	buttons[1] = 0;
	paint();
	}

/*
 *	'Load' clicked.
 */

void File_gump::load
	(
	)
{
	if (!focus ||			// This would contain the name.
	    !focus->get_length())
		return;
	int num = get_save_index(focus);// Which one is it?
	if (num == -1)
		return;			// Shouldn't ever happen.
	if (!Yesno_gump::ask(
			"Okay to load over your current game?"))
		return;
	gwin->restore_gamedat(num);	// Aborts if unsuccessful.
	gwin->read();			// And read the files in.
	done = 1;
	restored = 1;
}

/*
 *	'Save' clicked.
 */

void File_gump::save
	(
	)
{
	if (!focus || 			// This would contain the name.
	    !focus->get_length())
		return;
	int num = get_save_index(focus);// Which one is it?
	if (num == -1)
		return;			// Shouldn't ever happen.
	if (*gwin->get_save_name(num))	// Already a game in this slot?
		if (!Yesno_gump::ask(
			"Okay to write over existing saved game?"))
			return;
	gwin->write();		// First flush to 'gamedat'.
	gwin->save_gamedat(num, focus->get_text());
	cout << "Saved game #" << num << " successfully." << endl;
	remove_focus();
}

/*
 *	'Quit' clicked.
 */

void File_gump::quit
	(
	)
{
	if (!Yesno_gump::ask("Do you really want to quit?"))
		return;
	quitting_time = QUIT_TIME_YES;
	done = 1;
}

/*
 *	One of the option toggle buttons was pressed.
 *
 *	Output:	New state of option (0 or 1).
 */

int File_gump::toggle_option
	(
	Gump_button *btn		// Button that was clicked.
	)
{
	if (btn == buttons[3])		// Music?
	{
		bool music = !Audio::get_ptr()->is_music_enabled();
		Audio::get_ptr()->set_music_enabled(music);
		if (!music)		// Stop what's playing.
			Audio::get_ptr()->stop_music();
		string s = music ? "yes" : "no";
					// Write option out.
		config->set("config/audio/midi/enabled", s, true);
		return music ? 1 : 0;
	}
	if (btn == buttons[4])		// Speech?
	{
		bool speech = !Audio::get_ptr()->is_speech_enabled();
		Audio::get_ptr()->set_speech_enabled(speech);
		string s = speech ? "yes" : "no";
					// Write option out.
		config->set("config/audio/speech/enabled", s, true);
		return speech ? 1 : 0;
	}
	if (btn == buttons[5])		// Sound effects?
	{
		bool effects = !Audio::get_ptr()->are_effects_enabled();
		Audio::get_ptr()->set_effects_enabled(effects);
		if (!effects)		// Off?  Stop what's playing.
			Audio::get_ptr()->stop_sound_effects();
		string s = effects ? "yes" : "no";
					// Write option out.
		config->set("config/audio/effects/enabled", s, true);
		return effects ? 1 : 0;
	}
	return false;			// Shouldn't get here.
}

/*
 *	Paint on screen.
 */

void File_gump::paint
	(
	)
{
	Gump::paint();			// Paint background
					// Paint text objects.
	size_t i;
	for (i = 0; i < sizeof(names)/sizeof(names[0]); i++)
		if (names[i])
			names[i]->paint();
	for (i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
		if (buttons[i])
			buttons[i]->paint();
}

/*
 *	Handle mouse-down events.
 */

void File_gump::mouse_down
	(
	int mx, int my			// Position in window.
	)
{
	pushed = 0;
	pushed_text = 0;
					// First try checkmark.
	Gump_button *btn = Gump::on_button(mx, my);
	if (btn)
		pushed = btn;
	else				// Try buttons at bottom.
		for (size_t i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
			if (buttons[i] && buttons[i]->on_button(mx, my))
			{
				pushed = buttons[i];
				break;
			}
	if (pushed)			// On a button?
	{
		pushed->push();
		return;
	}
					// See if on text field.
	for (size_t i = 0; i < sizeof(names)/sizeof(names[0]); i++)
		if (names[i]->on_widget(mx, my))
		{
			pushed_text = names[i];
			break;
		}
}

/*
 *	Handle mouse-up events.
 */

void File_gump::mouse_up
	(
	int mx, int my			// Position in window.
	)
{
	if (pushed)			// Pushing a button?
	{
		pushed->unpush();
		if (pushed->on_button(mx, my))
			pushed->activate();
		pushed = 0;
	}
	if (!pushed_text)
		return;
					// Let text field handle it.
	if (!pushed_text->mouse_clicked(mx, my) ||
	    pushed_text == focus)	// Same field already selected?
	{
		pushed_text->paint();
		pushed_text = 0;
		return;
	}
	if (focus)			// Another had focus.
	{
		focus->set_text(gwin->get_save_name(get_save_index(focus)));
		focus->lose_focus();
	}
	focus = pushed_text;		// Switch focus to new field.
	pushed_text = 0;
	if (focus->get_length())	// Need load/save buttons?
	{
		if (!buttons[0])
			buttons[0] = new Load_save_button(this,
					btn_cols[0], btn_rows[0], game->get_shape("gumps/loadbtn"));
		if (!buttons[1])
			buttons[1] = new Load_save_button(this,
					btn_cols[1], btn_rows[0], game->get_shape("gumps/savebtn"));
	}
	else if (!focus->get_length())
	{			// No name yet.
		delete buttons[0];
		delete buttons[1];
		buttons[0] = buttons[1] = 0;
	}
	paint();			// Repaint.
	gwin->set_painted();
}

/*
 *	Handle character that was typed.
 */

void File_gump::text_input(int chr, int unicode)
{
	if (!focus)			// Text field?
		return;
	switch (chr)
		{
	case SDLK_RETURN:		// If only 'Save', do it.
		if (!buttons[0] && buttons[1])
			{
			buttons[1]->push();
			gwin->show(1);
			buttons[1]->unpush();
			gwin->show(1);
			buttons[1]->activate();
			}
		break;
	case SDLK_BACKSPACE:
		if (focus->delete_left())
		{		// Can't restore now.
			delete buttons[0];
			buttons[0] = 0;
		}
		if (!focus->get_length())
		{		// Last char.?
			delete buttons[0];
			delete buttons[1];
			buttons[0] = buttons[1] = 0;
			paint();
		}
		return;
	case SDLK_DELETE:
		if (focus->delete_right())
		{		// Can't restore now.
			delete buttons[0];
			buttons[0] = 0;
		}
		if (!focus->get_length())
		{		// Last char.?
			delete buttons[0];
			delete buttons[1];
			buttons[0] = buttons[1] = 0;
			paint();
		}
		return;
	case SDLK_LEFT:
		focus->set_cursor(focus->get_cursor() - 1);
		return;
	case SDLK_RIGHT:
		focus->set_cursor(focus->get_cursor() + 1);
		return;
	case SDLK_HOME:
		focus->set_cursor(0);
		return;
	case SDLK_END:
		focus->set_cursor(focus->get_length());
		return;
	}

	if ((unicode & 0xFF80) == 0 )
		chr = unicode & 0x7F;
	else
		chr = 0;
	
	if (chr < ' ')
		return;			// Ignore other special chars.
	if (chr < 256 && isascii(chr))
	{
		int old_length = focus->get_length();
		focus->insert(chr);
					// Added first character?  Need 
					//   'Save' button.
		if (!old_length && focus->get_length() && !buttons[1])
		{
			buttons[1] = new Load_save_button(this,
					btn_cols[1], btn_rows[0], game->get_shape("gumps/savebtn"));
			buttons[1]->paint();
		}
		if (buttons[0])		// Can't load now.
		{
			delete buttons[0];
			buttons[0] = 0;
			paint();
		}
		gwin->set_painted();
	}
}
