/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Gumps.cc - Open containers and their contents.
 **
 **	Written: 3/4/2000 - JSF
 **/
/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#include "gumps.h"
#include "gamewin.h"
#include "actors.h"
#include "mouse.h"
#include "game.h"
#include "Audio.h"
#include "Configuration.h"
#include "objiter.h"

using std::cout;
using std::endl;
using std::string;

extern Configuration *config;

/*
 *	Some gump shape numbers:
 */
const int ASLEEP = 0, POISONED = 1, CHARMED = 2, HUNGRY = 3,
		  PROTECTED = 4, CURSED = 5, PARALYZED = 6;

/*
 *	Statics:
 */
short Actor_gump_object::diskx = 124, Actor_gump_object::disky = 115;
short Actor_gump_object::heartx = 124, Actor_gump_object::hearty = 132;
short Actor_gump_object::combatx = 52, Actor_gump_object::combaty = 103;
short Actor_gump_object::halox = 47, Actor_gump_object::haloy = 110;
short Actor_gump_object::cmodex = 48, Actor_gump_object::cmodey = 132;
short Actor_gump_object::coords[24] = {
	114, 10,	/* head */	115, 24,	/* back */
	115, 37,	/* belt */	115, 55,	/* lhand */
	115, 71,	/* lfinger */	114, 85,	/* legs */
	76, 98,		/* feet */	35, 70,		/* rfinger */
	37, 56,		/* rhand */	37, 37,		/* torso */
	37, 24,		/* neck */	37, 11		/* ammo */
	};
short Stats_gump_object::textx = 123;
short Stats_gump_object::texty[10] = {17, 26, 35, 46, 55, 67, 76, 86,
							95, 104};
short Slider_gump_object::leftbtnx = 31;
short Slider_gump_object::rightbtnx = 103;
short Slider_gump_object::btny = 14;
short Slider_gump_object::diamondy = 6;
short Slider_gump_object::xmin = 35;	// Min., max. positions of diamond.
short Slider_gump_object::xmax = 93;

short File_gump_object::btn_rows[2] = {143, 156};
short File_gump_object::btn_cols[3] = {94, 163, 232};
short File_gump_object::textx = 237, File_gump_object::texty = 14,
      File_gump_object::texth = 13;

short Yesno_gump_object::yesx = 63;
short Yesno_gump_object::yesnoy = 45;
short Yesno_gump_object::nox = 84;

// Paperdoll is completely different to Actor
short Paperdoll_gump_object::diskx = 123, Paperdoll_gump_object::disky = 137;
short Paperdoll_gump_object::heartx = 98, Paperdoll_gump_object::hearty = 137;
short Paperdoll_gump_object::combatx = 51, Paperdoll_gump_object::combaty = 142;
short Paperdoll_gump_object::cstatx = 73, Paperdoll_gump_object::cstaty = 137;
short Paperdoll_gump_object::coords[26] = {
	114, 10,	/* ears */	115, 24,	/* back */
	115, 37,	/* belt */	37, 56,		/* lhand */
	115, 71,	/* lfinger */	114, 85,	/* legs */
	76, 98,		/* feet */	35, 70,		/* rfinger */
	115, 55,	/* rhand */	37, 37,		/* torso */
	37, 24,		/* head */	37, 11,		/* ammo */
	57, 51		/* back2 */
	};


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
	int focus;			// Use frame 1 if focused, else 0.
public:
	Gump_text(Gump_object *par, int shnum, int px, int py, int maxsz,
						int tx, int ty)
		: Gump_widget(par, shnum, px, py), text(new char[maxsz + 1]),
		  max_size(maxsz), length(0), textx(x + tx), texty(y + ty),
		  cursor(0), focus(0)
		{
		text[0] = text[maxsz] = 0;
		Shape_frame *shape = Game_window::get_game_window()->
			get_gump_shape(shnum, 0);
					// Want text coords. rel. to parent.
		textx -= shape->get_xleft();
		texty -= shape->get_yabove();
		}
	~Gump_text()
		{ delete text; }
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
			paint();
			}
		}
	void paint();			// Paint.
					// Handle mouse click.
	int mouse_clicked(Game_window *gwin, int mx, int my);
	void insert(int chr);		// Insert a character.
	int delete_left();		// Delete char. to left of cursor.
	int delete_right();		// Delete char. to right of cursor.
	void lose_focus();
	};

/*
 *	A checkmark for closing its parent:
 */
class Checkmark_gump_button : public Gump_button
	{
public:
	Checkmark_gump_button(Gump_object *par, int px, int py)
		: Gump_button(par, game->get_shape("gumps/check"), px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	A 'heart' button for bringing up stats.
 */
class Heart_gump_button : public Gump_button
	{
public:
	Heart_gump_button(Gump_object *par, int px, int py)
		: Gump_button(par, game->get_shape("gumps/heart"), px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	A diskette for bringing up the 'save' box.
 */
class Disk_gump_button : public Gump_button
	{
public:
	Disk_gump_button(Gump_object *par, int px, int py)
		: Gump_button(par, game->get_shape("gumps/disk"), px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	The combat toggle button.
 */
class Combat_gump_button : public Gump_button
	{
public:
	Combat_gump_button(Gump_object *par, int px, int py)
		: Gump_button(par, game->get_shape("gumps/combat"),
			px, py)
		{
		pushed = Game_window::get_game_window()->in_combat();
		}
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	The halo button.
 */
class Halo_gump_button : public Gump_button
	{
public:
	Halo_gump_button(Gump_object *par, int px, int py)
		: Gump_button(par, game->get_shape("gumps/halo"), px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	Combat mode.  Has 10 frames corresponding to Actor::Attack_mode.
 */
class Combat_mode_gump_button : public Gump_button
	{
	Actor *actor;			// Who this represents.
public:
	Combat_mode_gump_button(Gump_object *par, int px, int py, Actor *a)
		: Gump_button(par, game->get_shape("gumps/combatmode"), px, py), actor(a)
		{
		framenum = (int) actor->get_attack_mode();
		}
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	A 'yes' or 'no' button.
 */
class Yesno_gump_button : public Gump_button
	{
	int isyes;			// 1 for 'yes', 0 for 'no'.
public:
	Yesno_gump_button(Gump_object *par, int px, int py, int yes)
		: Gump_button(par, yes ? 
			game->get_shape("gumps/yesbtn") 
			: game->get_shape("gumps/nobtn"), px, py),
		  isyes(yes)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	One of the two arrow button on the slider:
 */
class Slider_gump_button : public Gump_button
	{
public:
	Slider_gump_button(Gump_object *par, int px, int py, int shapenum)
		: Gump_button(par, shapenum, px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	Load or save button.
 */
class Load_save_gump_button : public Gump_button
	{
public:
	Load_save_gump_button(Gump_object *par, int px, int py, int shapenum)
		: Gump_button(par, shapenum, px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	Quit button.
 */
class Quit_gump_button : public Gump_button
	{
public:
	Quit_gump_button(Gump_object *par, int px, int py)
		: Gump_button(par, 
			game->get_shape("gumps/quitbtn"), px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	Sound 'toggle' buttons.
 */
class Sound_gump_button : public Gump_button
	{
public:
	Sound_gump_button(Gump_object *par, int px, int py, int shapenum,
								bool enabled)
		: Gump_button(par, shapenum, px, py)
		{ pushed = enabled ? 1 : 0; }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	Is a given screen point on this widget?
 */

int Gump_widget::on_widget
	(
	Game_window *gwin,
	int mx, int my			// Point in window.
	)
	{
	mx -= parent->get_x() + x;	// Get point rel. to gump.
	my -= parent->get_y() + y;
	Shape_frame *cshape = gwin->get_gump_shape(shapenum, 0);
	return (cshape->has_point(mx, my));
	}

/*
 *	Paint text field.
 */

void Gump_text::paint
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	gwin->paint_gump(parent->get_x() + x, 
					parent->get_y() + y, shapenum, focus);
					// Show text.
	gwin->paint_text(2, text, parent->get_x() + textx,
						parent->get_y() + texty);
	if (focus)			// Focused?  Show cursor.
		gwin->get_win()->fill8(0, 1, gwin->get_text_height(2),
			parent->get_x() + textx +
					gwin->get_text_width(2, text, cursor),
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
	Game_window *gwin,
	int mx, int my			// Mouse position on screen.
	)
	{
	if (!on_widget(gwin, mx, my))	// Not in our area?
		return (0);
	mx -= textx + parent->get_x();	// Get pt. rel. to text area.
	if (!focus)			// Gaining focus?
		{
		focus = 1;		// We have focus now.
		cursor = 0;		// Put cursor at start.
		}
	else
		{
		for (cursor = 0; cursor <= length; cursor++)
			if (gwin->get_text_width(2, text, cursor) > mx)
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
	if (!focus || length == max_size)
		return;			// Can't.
	if (cursor < length)		// Open up space.
		memmove(text + cursor + 1, text + cursor, length - cursor);
	text[cursor++] = chr;		// Store, and increment cursor.
	length++;
	text[length] = 0;
	paint();
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
	if (!focus || !cursor)		// Can't do it.
		return (0);
	if (cursor < length)		// Shift text left.
		memmove(text + cursor - 1, text + cursor, length - cursor);
	text[--length] = 0;		// 0-delimit.
	cursor--;
	paint();
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
	if (!focus || cursor == length)
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
	focus = 0;
	paint();
	}

/*
 *	Redisplay as 'pushed'.
 */

void Gump_button::push
	(
	Game_window *gwin
	)
	{
	pushed = 1;
	parent->paint_button(gwin, this);
	gwin->set_painted();
	}

/*
 *	Redisplay as 'unpushed'.
 */

void Gump_button::unpush
	(
	Game_window *gwin
	)
	{
	pushed = 0;
	parent->paint_button(gwin, this);
	gwin->set_painted();
	}

/*
 *	Default method for double-click.
 */

void Gump_button::double_clicked
	(
	Game_window *gwin
	)
	{
	}

/*
 *	Handle click on a 'checkmark'.
 */

void Checkmark_gump_button::activate
	(
	Game_window *gwin
	)
	{
	parent->close(gwin);
	}

/*
 *	Handle click on a heart.
 */

void Heart_gump_button::activate
	(
	Game_window *gwin
	)
	{
	gwin->show_gump(parent->get_container(), game->get_shape("gumps/statsdisplay"));
	}

/*
 *	Handle click on a diskette.
 */

void Disk_gump_button::activate
	(
	Game_window *gwin
	)
	{
	extern int Modal_gump(Modal_gump_object *, Mouse::Mouse_shapes);
	File_gump_object *fileio = new File_gump_object();
	Modal_gump(fileio, Mouse::hand);
	delete fileio;
	}

/*
 *	Handle click on a combat toggle button.
 */

void Combat_gump_button::activate
	(
	Game_window *gwin
	)
	{
	gwin->toggle_combat();
	pushed = gwin->in_combat();
	parent->paint(gwin);
	}

/*
 *	Handle click on a halo toggle button.
 */

void Halo_gump_button::activate
	(
	Game_window *gwin
	)
	{
					// ++++++Later.
	}

/*
 *	Handle click on a combat toggle button.
 */

void Combat_mode_gump_button::activate
	(
	Game_window *gwin
	)
	{
	framenum = (framenum + 1)%10;
	actor->set_attack_mode((Actor::Attack_mode) framenum);
	parent->paint_button(gwin, this);
	gwin->set_painted();
	}


/*
 *	Handle 'yes' or 'no' button.
 */

void Yesno_gump_button::activate
	(
	Game_window *gwin
	)
	{
	((Yesno_gump_object *) parent)->set_answer(isyes);
	}

/*
 *	Handle click on a slider's arrow.
 */

void Slider_gump_button::activate
	(
	Game_window *gwin
	)
	{
	((Slider_gump_object *) parent)->clicked_arrow(this);
	}

/*
 *	Clicked a 'load' or 'save' button.
 */

void Load_save_gump_button::activate
	(
	Game_window *gwin
	)
	{
	if (shapenum == game->get_shape("gumps/loadbtn"))
		((File_gump_object *) parent)->load();
	else
		((File_gump_object *) parent)->save();
	}

/*
 *	Clicked on 'quit'.
 */

void Quit_gump_button::activate
	(
	Game_window *gwin
	)
	{
	((File_gump_object *) parent)->quit();
	}

/*
 *	Clicked on one of the sound options.
 */

void Sound_gump_button::activate
	(
	Game_window *gwin
	)
	{
	pushed = ((File_gump_object *) parent)->toggle_option(this);
	parent->paint(gwin);
	}

/*
 *	Initialize.
 */

void Gump_object::initialize
	(
	)
	{

	if (is_paperdoll())
		{
		initialize2();
		return;
		}
		
	int checkx = 8, checky = 64;	// Default.
	int shnum = get_shapenum();
	if(shnum==game->get_shape("gumps/yesnobox"))
		{
		object_area = Rectangle(6, 6, 116, 28);
		}
	else if(shnum==game->get_shape("gumps/fileio"))
		{
		checkx = 8, checky = 150;
		}
	else if(shnum==game->get_shape("gumps/statsdisplay"))
		{
		object_area = Rectangle(0, 0, 0, 0);
    	        checkx = 6; checky = 136;
	        }
        else if(shnum==game->get_shape("gumps/spellbook"))
        	{
		checkx = 7; checky = 54;
                object_area = Rectangle(36, 28, 102, 66);
	        }
        else if(shnum==game->get_shape("gumps/box"))
        	{
		object_area = Rectangle(46, 28, 74, 32);
		checkx = 8; checky = 56;
	        }
        else if(shnum==game->get_shape("gumps/crate"))
        	{
		object_area = Rectangle(50, 20, 80, 24);
		checkx = 8; checky = 64;
	        }
        else if(shnum==game->get_shape("gumps/barrel"))
        	{
		object_area = Rectangle(32, 32, 40, 40);
		checkx = 12; checky = 124;
	        }
        else if(shnum==game->get_shape("gumps/bag"))
	        {
		object_area = Rectangle(48, 20, 66, 44);
		checkx = 8; checky = 66;
        	}
        else if(shnum==game->get_shape("gumps/backpack"))
	        {
		object_area = Rectangle(36, 36, 85, 40);
		checkx = 8; checky = 62;
        	}
        else if(shnum==game->get_shape("gumps/basket"))
	        {
		object_area = Rectangle(42, 32, 70, 26);
		checkx = 8; checky = 56;
        	}
        else if(shnum==game->get_shape("gumps/chest"))
	        {
		object_area = Rectangle(40, 18, 60, 37);
		checkx = 8; checky = 46;
        	}
        else if(shnum==game->get_shape("gumps/shipshold"))
	        {
		object_area = Rectangle(38, 10, 82, 80);
		checkx = 8; checky = 92;
        	}
        else if(shnum==game->get_shape("gumps/drawer"))
	        {
		object_area = Rectangle(36, 12, 70, 26);
		checkx = 8; checky = 46;
        	}
        else if(shnum==game->get_shape("gumps/slider"))
	        {
		object_area = Rectangle(0, 0, 0, 0);
		checkx = 6; checky = 30;
        	}
        else if(shnum==game->get_shape("gumps/woodsign"))
	        {
		object_area = Rectangle(0, 4, 196, 92);
	        }
        else if(shnum==game->get_shape("gumps/tombstone"))
	        {
		object_area = Rectangle(0, 8, 200, 112);
        	}
        else if(shnum==game->get_shape("gumps/goldsign"))
	        {
		object_area = Rectangle(0, 4, 232, 96);
	        }
        else if(shnum==game->get_shape("gumps/body"))
        	{
		object_area = Rectangle(36, 46, 84, 40);
		checkx = 8; checky = 70;
	        }
        else if ((shnum >= 57 && shnum <= 68 && Game::get_game_type() == BLACK_GATE))
		{		// Character pictures:
				// Want whole rectangle.
		object_area = Rectangle(26, 0, 104, 132);
		checkx = 6; checky = 136;
		}
	else
		object_area = Rectangle(52, 22, 60, 40);

	checkx += 16; checky -= 12;
	check_button = new Checkmark_gump_button(this, checkx, checky);
	}

void Gump_object::initialize2
	(
	)
	{
	int checkx = 8, checky = 64;	// Default.
	int shnum = get_shapenum();
	if (shnum == 123)
		{		// Character pictures:
				// Want whole rectangle.
		object_area = Rectangle(26, 0, 104, 140);
		checkx = 6; checky = 145;
		}
	else
		object_area = Rectangle(52, 22, 60, 40);

	checkx += 16; checky -= 12;
	check_button = new Checkmark_gump_button(this, checkx, checky);
	}

/*
 *	Create a gump.
 */

Gump_object::Gump_object
	(
	Container_game_object *cont,	// Container it represents.
	int initx, int inity, 		// Coords. on screen.
	int shnum,			// Shape #.
	bool pdoll			// Use the shapes from paperdoll.flx
	) : ShapeID(shnum, 0), container(cont), x(initx), y(inity),
	paperdoll_shape (pdoll)
	{
	initialize();
	}

/*
 *	Create, centered on screen.
 */

Gump_object::Gump_object
	(
	Container_game_object *cont,	// Container it represents.
	int shnum			// Shape #.
	) : ShapeID(shnum, 0), container(cont), paperdoll_shape (false)
	{
	Game_window *gwin = Game_window::get_game_window();
	Shape_frame *shape = gwin->get_gump_shape(shnum, 0);
	x = (gwin->get_width() - shape->get_width())/2;
	y = (gwin->get_height() - shape->get_height())/2;
	initialize();
	}

/*
 *	Delete gump.
 */

Gump_object::~Gump_object
	(
	)
	{
	delete check_button; 
	}

/*
 *	Add this gump to the end of a chain.
 */

void Gump_object::append_to_chain
	(
	Gump_object *& chain		// Head.
	)
	{
	next = 0;			// Put at end of chain.
	if (!chain)
		{
		chain = this;		// First one.
		return;
		}
	Gump_object *last;
	for (last = chain; last->next; last = last->next)
		;
	if (!last)			// First one?
		chain = this;
	else
		last->next = this;
	}

/*
 *	Remove from a chain.
 */

void Gump_object::remove_from_chain
	(
	Gump_object *& chain		// Head.
	)
	{
	if (chain == this)
		chain = next;
	else
		{
		Gump_object *p;		// Find prev. to this.
		for (p = chain; p->next != this; p = p->next)
			;
		p->next = next;
		}
	}

/*
 *	Get screen rectangle for one of our objects.
 */

Rectangle Gump_object::get_shape_rect
	(
	Game_object *obj
	)
	{
	Shape_frame *s = Game_window::get_game_window()->get_shape(*obj);
	return Rectangle(x + object_area.x + obj->get_cx() - s->get_xleft(), 
			 y + object_area.y + obj->get_cy() - s->get_yabove(), 
				 s->get_width(), s->get_height());
	}

/*
 *	Get screen location of object within.
 */

void Gump_object::get_shape_location
	(
	Game_object *obj, 
	int& ox, int& oy
	)
	{
	ox = x + object_area.x + obj->get_cx(),
	oy = y + object_area.y + obj->get_cy();
	}

/*
 *	Find object a screen point is on.
 *
 *	Output:	Object found, or null.
 */

Game_object *Gump_object::find_object
	(
	int mx, int my			// Mouse pos. on screen.
	)
	{
	int cnt = 0;
	Game_object *list[100];
	if (!container)
		return (0);
	Object_iterator next(container->get_objects());
	Game_object *obj;
	while ((obj = next.get_next()) != 0)
		{
		Rectangle box = get_shape_rect(obj);
		if (box.has_point(mx, my))
			list[cnt++] = obj;
		obj = obj->get_next();
		}
					// ++++++Return top item.
	return (cnt ? list[cnt - 1] : 0);
	}

/*
 *	Get the entire screen rectangle covered by this gump and its contents.
 */

Rectangle Gump_object::get_dirty
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Rectangle rect = gwin->get_gump_rect(this);
	if (!container)
		return rect;
	Object_iterator next(container->get_objects());
	Game_object *obj;
	while ((obj = next.get_next()) != 0)
		{
		Rectangle orect = get_shape_rect(obj);
		rect = rect.add(orect);
		}
	return rect;
	}

/*
 *	Get object this belongs to.
 */

Game_object *Gump_object::get_owner()
	{ 
	return container; 
	}

/*
 *	Is a given screen point on the checkmark?
 *
 *	Output: ->button if so.
 */

Gump_button *Gump_object::on_button
	(
	Game_window *gwin,
	int mx, int my			// Point in window.
	)
	{
	return (check_button->on_button(gwin, mx, my) ?
			check_button : 0);
	}

/*
 *	Repaint checkmark, etc.
 */

void Gump_object::paint_button
	(
	Game_window *gwin,
	Gump_button *btn
	)
	{
	if (btn) gwin->paint_gump(x + btn->x, y + btn->y, btn->shapenum, 
					btn->framenum + btn->pushed);
	}

/*
 *	Add an object.  If mx, my, sx, sy are all -1, the object's position
 *	is calculated by 'paint()'.  If they're all -2, it's assumed that
 *	obj->cx, obj->cy are already correct.
 *
 *	Output:	0 if cannot add it.
 */

int Gump_object::add
	(
	Game_object *obj,
	int mx, int my,			// Mouse location.
	int sx, int sy			// Screen location of obj's hotspot.
	)
	{
	if (!container || !container->has_room(obj))
		return (0);		// Full.
					// Dropping on same thing?
	Game_object *onobj = find_object(mx, my);
					// If possible, combine.

	if (onobj && onobj != obj && onobj->drop(obj))
		return (1);

	if (!container->add(obj))
		return (0);

					// Not a valid spot?
	if (sx == -1 && sy == -1 && mx == -1 && my == -1)
					// Let paint() set spot.
		obj->set_chunk(255, 255);
					// -2's mean cx, cy are already set.
	else if (sx != -2 && sy != -2 && mx != -2 && my != -2)
		{			// Put it where desired.
		sx -= x + object_area.x;// Get point rel. to object_area.
		sy -= y + object_area.y;
		Shape_frame *shape = Game_window::get_game_window()->get_shape(
									*obj);
					// But shift within range.
		if (sx - shape->get_xleft() < 0)
			sx = shape->get_xleft();
		else if (sx + shape->get_xright() > object_area.w)
			sx = object_area.w - shape->get_xright();
		if (sy - shape->get_yabove() < 0)
			sy = shape->get_yabove();
		else if (sy + shape->get_ybelow() > object_area.h)
			sy = object_area.h - shape->get_ybelow();
		obj->set_chunk(sx, sy);
		}
	return (1);
	}

/*
 *	Remove object.
 */

void Gump_object::remove
	(
	Game_object *obj
	)
	{
	container->remove(obj); 
	}

/*
 *	Paint on screen.
 */

void Gump_object::paint
	(
	Game_window *gwin
	)
	{
					// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum(), is_paperdoll());
					// Paint red "checkmark".
	paint_button(gwin, check_button);
	if (!container)
		return;			// Empty.
	Object_list& objects = container->get_objects();
	if (objects.is_empty())
		return;			// Empty.
	Rectangle box = object_area;	// Paint objects inside.
	box.shift(x, y);		// Set box to screen location.
	int cury = 0, curx = 0;
	int endy = box.h, endx = box.w;
	int loop = 0;			// # of times covering container.
	Game_object *obj;
	Object_iterator next(objects);
	while ((obj = next.get_next()) != 0)
		{
		Shape_frame *shape = gwin->get_shape(*obj);
		int objx = obj->get_cx() - shape->get_xleft() + 
							1 + object_area.x;
		int objy = obj->get_cy() - shape->get_yabove() + 
							1 + object_area.y;
					// Does obj. appear to be placed?
		if (!object_area.has_point(objx, objy) ||
		    !object_area.has_point(objx + shape->get_xright() - 1,
					objy + shape->get_ybelow() - 1))
			{		// No.
			int px = curx + shape->get_width(),
			    py = cury + shape->get_height();
			if (px > endx)
				px = endx;
			if (py > endy)
				py = endy;
			obj->set_chunk(px - shape->get_xright(),
					py - shape->get_ybelow());
			curx += 8;
			if (curx >= endx)
				{
				cury += 8;
				curx = 0;
				if (cury >= endy)
					cury = 2*(++loop);
				}
			}
		gwin->paint_shape(box.x + obj->get_cx(),box.y + obj->get_cy(), 
				obj->get_shapenum(), obj->get_framenum());
		obj = obj->get_next();
		}
	}

/*
 *	Close and delete.
 */

void Gump_object::close
	(
	Game_window *gwin
	)
	{
	gwin->remove_gump(this);
	}

/*
 *	Find the index of the closest 'spot' to a mouse point.
 *
 *	Output:	Index, or -1 if unsuccessful.
 */

int Actor_gump_object::find_closest
	(
	int mx, int my,			// Mouse point in window.
	int only_empty			// Only allow empty spots.
	)
	{
	mx -= x; my -= y;		// Get point rel. to us.
	long closest_squared = 1000000;	// Best distance squared.
	int closest = -1;		// Best index.
	for (size_t i = 0; i < sizeof(coords)/(2*sizeof(coords[0])); i++)
		{
		int dx = mx - spotx(i), dy = my - spoty(i);
		long dsquared = dx*dx + dy*dy;
					// Better than prev.?
		if (dsquared < closest_squared && (!only_empty ||
						!container->get_readied(i)))
			{
			closest_squared = dsquared;
			closest = i;
			}
		}
	return (closest);
	}

/*
 *	Create the gump display for an actor.
 */

Actor_gump_object::Actor_gump_object
	(
	Container_game_object *cont,	// Container it represents.
	int initx, int inity, 		// Coords. on screen.
	int shnum			// Shape #.
	) : Gump_object(cont, initx, inity, shnum)
	{
	heart_button = new Heart_gump_button(this, heartx, hearty);
	if (cont->get_npc_num() == 0) disk_button = new Disk_gump_button(this, diskx, disky);
	else disk_button = NULL;
	if (cont->get_npc_num() == 0) combat_button = new Combat_gump_button(this, combatx, combaty);
	else combat_button = NULL;
	halo_button = new Halo_gump_button(this, halox, haloy);
	cmode_button = new Combat_mode_gump_button(this, cmodex, cmodey, (Actor *) cont);
							
	for (size_t i = 0; i < sizeof(coords)/2*sizeof(coords[0]); i++)
		{			// Set object coords.
		Game_object *obj = container->get_readied(i);
		if (obj)
			set_to_spot(obj, i);
		}
	}

/*
 *	Delete actor display.
 */

Actor_gump_object::~Actor_gump_object
	(
	)
	{
	if (heart_button) delete heart_button;
	if (disk_button) delete disk_button;
	if (combat_button) delete combat_button;
	if (halo_button) delete halo_button;
	if (cmode_button) delete cmode_button;
	}

/*
 *	Is a given screen point on one of our buttons?
 *
 *	Output: ->button if so.
 */

Gump_button *Actor_gump_object::on_button
	(
	Game_window *gwin,
	int mx, int my			// Point in window.
	)
	{
	Gump_button *btn = Gump_object::on_button(gwin, mx, my);
	if (btn)
		return btn;
	else if (heart_button && heart_button->on_button(gwin, mx, my))
		return heart_button;
	else if (disk_button && disk_button->on_button(gwin, mx, my))
		return disk_button;
	else if (combat_button && combat_button->on_button(gwin, mx, my))
		return combat_button;
	else if (halo_button && halo_button->on_button(gwin, mx, my))
		return halo_button;
	else if (cmode_button && cmode_button->on_button(gwin, mx, my))
		return cmode_button;
	return 0;
	}

/*
 *	Add an object.
 *
 *	Output:	0 if cannot add it.
 */

int Actor_gump_object::add
	(
	Game_object *obj,
	int mx, int my,			// Screen location of mouse.
	int sx, int sy			// Screen location of obj's hotspot.
	)
	{
					// Find index of closest spot.
	int index = find_closest(mx, my);
	if (!container->add_readied(obj, index))
		{			// Can't add it there?
					// Try again for an empty spot.
		index = find_closest(mx, my, 1);
		if (index < 0 || !container->add_readied(obj, index))
					// Just try to add it.
			if (!container->add(obj))
				return (0);
		}
					// In case it went in another obj:
	index = container->find_readied(obj);
	if (index >= 0)
		set_to_spot(obj, index);// Set obj. coords.
	return (1);
	}

/*
 *	Set object's coords. to given spot.
 */

void Actor_gump_object::set_to_spot
	(
	Game_object *obj,
	int index			// Spot index.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Get shape info.
	Shape_frame *shape = gwin->get_shape(*obj);
	int w = shape->get_width(), h = shape->get_height();
					// Set object's position.
	obj->set_chunk(spotx(index) + shape->get_xleft() - w/2 - object_area.x,
		spoty(index) + shape->get_yabove() - h/2 - object_area.y);
					// Shift if necessary.
	int x0 = obj->get_cx() - shape->get_xleft(), 
	    y0 = obj->get_cy() - shape->get_yabove();
	int newcx = obj->get_cx(), newcy = obj->get_cy();
	if (x0 < 0)
		newcx -= x0;
	if (y0 < 0)
		newcy -= y0;
	int x1 = x0 + w, y1 = y0 + h;
	if (x1 > object_area.w)
		newcx -= x1 - object_area.w;
	if (y1 > object_area.h)
		newcy -= y1 - object_area.h;
	obj->set_chunk(newcx, newcy);
	}

/*
 *	Paint on screen.
 */

void Actor_gump_object::paint
	(
	Game_window *gwin
	)
	{
					// Watch for any newly added objs.
	for (size_t i = 0; i < sizeof(coords)/2*sizeof(coords[0]); i++)
		{			// Set object coords.
		Game_object *obj = container->get_readied(i);
		if (obj && !obj->get_cx() && !obj->get_cy())
			set_to_spot(obj, i);
		}
	Gump_object::paint(gwin);	// Paint gump & objects.
					// Paint buttons.
	if (heart_button) paint_button(gwin, heart_button);
	if (disk_button) paint_button(gwin, disk_button);
	if (combat_button) paint_button(gwin, combat_button);
	if (halo_button) paint_button(gwin, halo_button);
	if (cmode_button) paint_button(gwin, cmode_button);
					// Show weight.
	int max_weight = 2*container->get_property(Actor::strength);
	int weight = container->get_weight()/10;
	char text[20];
	sprintf(text, "%d/%d", weight, max_weight);
	int twidth = gwin->get_text_width(2, text);
	const int boxw = 102;
	gwin->paint_text(2, text, x + 28 + (boxw - twidth)/2, y + 120);
	}

/*
 *	Create stats display.
 */
Stats_gump_object::Stats_gump_object
	(
	Container_game_object *cont, 
	int initx, int inity
	) : Gump_object(cont, initx, inity, game->get_shape("gumps/statsdisplay"))
	{
	}

/*
 *	Show a number.
 */

static void Paint_num
	(
	Game_window *gwin,
	int num,
	int x,				// Coord. of right edge of #.
	int y				// Coord. of top of #.
	)
	{
	const int font = 2;
	char buf[20];
  	sprintf(buf, "%d", num);
	gwin->paint_text(font, buf, x - gwin->get_text_width(font, buf), y);
	}

/*
 *	Show one of the atts.
 *
 *	Output:	Amount to increment x-pos for the next one.
 */

static int Show_atts
	(
	Game_window *gwin,
	int x, int y,			// Pos. on screen.
	int framenum
	)
	{
	Shape_frame *s = gwin->get_gump_shape(game->get_shape(
						"gumps/statatts"), framenum);
	gwin->paint_shape(x + s->get_xleft(),
				 y + s->get_ybelow(), s, 1);
	return s->get_width() + 2;
	}

/*
 *	Paint on screen.
 */

void Stats_gump_object::paint
	(
	Game_window *gwin
	)
	{
					// Area to print name in.
	const int namex = 30, namey = 6, namew = 95;
					// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum());
					// Paint red "checkmark".
	paint_button(gwin, check_button);
	Actor *act = get_actor();	// Show statistics.
	std::string nm = act->get_name();
	gwin->paint_text(2, nm.c_str(), x + namex +
		(namew - gwin->get_text_width(2, nm.c_str()))/2, y + namey);
	Paint_num(gwin, act->get_property(Actor::strength),
						x + textx, y + texty[0]);
	Paint_num(gwin, act->get_property(Actor::dexterity),
						x + textx, y + texty[1]);
	Paint_num(gwin, act->get_property(Actor::intelligence),
						x + textx, y + texty[2]);
  	Paint_num(gwin, act->get_property(Actor::combat),
						x + textx, y + texty[3]);
  	Paint_num(gwin, act->get_property(Actor::magic),
						x + textx, y + texty[4]);
  	Paint_num(gwin, act->get_property(Actor::health),
						x + textx, y + texty[5]);
  	Paint_num(gwin, act->get_property(Actor::mana),
						x + textx, y + texty[6]);
  	Paint_num(gwin, act->get_property(Actor::exp),
						x + textx, y + texty[7]);
	Paint_num(gwin, act->get_level(), x + textx, y + texty[8]);
  	Paint_num(gwin, act->get_property(Actor::training),
						x + textx, y + texty[9]);
					// Now show atts. at bottom.
	const int attsy = 130, attsx0 = 29;
	int attsx = attsx0;
	if (act->get_flag(Actor::asleep))
		attsx += Show_atts(gwin, x + attsx, y + attsy, ASLEEP);
	if (act->get_flag(Actor::poisoned))
		attsx += Show_atts(gwin, x + attsx, y + attsy, POISONED);
	if (act->get_flag(Actor::charmed))
		attsx += Show_atts(gwin, x + attsx, y + attsy, CHARMED);
	if (act->get_property((int) Actor::food_level) <= 4)
		attsx += Show_atts(gwin, x + attsx, y + attsy, HUNGRY);
	if (act->get_flag(Actor::protection))
		attsx += Show_atts(gwin, x + attsx, y + attsy, PROTECTED);
	if (act->get_flag(Actor::cursed))
		attsx += Show_atts(gwin, x + attsx, y + attsy, CURSED);
	if (act->get_flag(Actor::paralyzed))
		attsx += Show_atts(gwin, x + attsx, y + attsy, PARALYZED);
	}

/*
 *	Create a sign gump.
 */

Sign_gump::Sign_gump
	(
	int shapenum,
	int nlines			// # of text lines.
	) : Gump_object(0, shapenum), num_lines(nlines)
	{
	lines = new char *[num_lines];
	for (int i = 0; i < num_lines; i++)
		lines[i] = 0;
	}

/*
 *	Delete sign.
 */

Sign_gump::~Sign_gump
	(
	)
	{
	for (int i = 0; i < num_lines; i++)
		delete lines[i];
	delete lines;
	}

/*
 *	Add a line of text.
 */

void Sign_gump::add_text
	(
	int line,
	const char *txt
	)
	{
	if (line < 0 || line >= num_lines)
		return;
	delete lines[line];
	lines[line] = txt ? strdup(txt) : 0;
	}

/*
 *	Paint sign.
 */

void Sign_gump::paint
	(
	Game_window *gwin
	)
	{
	int font = 1;			// Normal runes.
	if (get_shapenum() == 0x33)
		font = 6;		// Embossed.
					// Get height of 1 line.
	int lheight = gwin->get_text_height(font);
					// Get space between lines.
	int lspace = (object_area.h - num_lines*lheight)/(num_lines + 1);
					// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum());
	int ypos = y + object_area.y;	// Where to paint next line.
	for (int i = 0; i < num_lines; i++)
		{
		ypos += lspace;
		if (!lines[i])
			continue;
		gwin->paint_text(font, lines[i],
			x + object_area.x + 
				(object_area.w - 
				    gwin->get_text_width(font, lines[i]))/2,
			ypos);
		ypos += lheight;
		}
	gwin->set_painted();
	}

/*
 *	Add to the text.
 */

void Text_gump::add_text
	(
	char *str
	)
	{
	int slen = strlen(str);		// Length of new text.
					// Allocate new space.
	char *newtext = new char[textlen + slen + 1];
	if (textlen)			// Copy over old.
		strcpy(newtext, text);
	strcpy(newtext + textlen, str);	// Append new.
	delete text;
	text = newtext;
	textlen += slen;
	}

/*
 *	Paint a page and find where its text ends.
 *
 *	Output:	Index past end of displayed page.
 */

int Text_gump::paint_page
	(
	Game_window *gwin,
	Rectangle box,			// Display box rel. to gump.
	int start			// Starting offset into text.
	)
	{
	const int font = 4;		// Black.
	const int vlead = 2;		// Extra inter-line spacing.
	int ypos = 0;
	int textheight = gwin->get_text_height(font) + vlead;
	char *str = text + start;
	while (*str && *str != '*' && ypos + textheight <= box.h)
		{
		if (*str == '~')	// End of paragraph?
			{
			ypos += textheight;
			str++;
			continue;
			}
					// Look for page break.
		char *epage = strchr(str, '*');
					// Look for line break.
		char *eol = strchr(str, '~');
		if (epage && (!eol || eol > epage))
			eol = epage;
		if (!eol)		// No end found?
			eol = text + textlen;
		char eolchr = *eol;	// Save char. at EOL.
		*eol = 0;
		int endoff = gwin->paint_text_box(font, str, x + box.x,
				y + box.y + ypos, box.w, box.h - ypos, vlead);
		*eol = eolchr;		// Restore char.
		if (endoff > 0)		// All painted?
			{		// Value returned is height.
			str = eol;
			ypos += endoff;
			}
		else			// Out of room.
			{
			str += -endoff;
			break;
			}
		}
	if (*str == '*')		// Saw end of page?
		str++;
	gwin->set_painted();		// Force blit.
	return (str - text);		// Return offset past end.
	}

/*
 *	Show next page(s) of book or scroll.
 *
 *	Output:	0 if already at end.
 */

int Text_gump::show_next_page
	(
	Game_window *gwin
	)
	{
	if (curend >= textlen)
		return (0);		// That's all, folks.
	curtop = curend;		// Start next page or pair of pages.
	paint(gwin);			// Paint.  This updates curend.
	return (1);
	}

/*
 *	Create book display.
 */

Book_gump::Book_gump
	(
	) : Text_gump(game->get_shape("gumps/book"))
	{
	}

/*
 *	Paint book.  Updates curend.
 */

void Book_gump::paint
	(
	Game_window *gwin
	)
	{
					// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum());
					// Paint left page.
	curend = paint_page(gwin, Rectangle(36, 10, 122, 130), curtop);
					// Paint right page.
	curend = paint_page(gwin, Rectangle(174, 10, 122, 130), curend);
	}

/*
 *	Create scroll display.
 */

Scroll_gump::Scroll_gump
	(
	) : Text_gump(game->get_shape("gumps/scroll"))
	{  
	}

/*
 *	Paint scroll.  Updates curend.
 */

void Scroll_gump::paint
	(
	Game_window *gwin
	)
	{
					// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum());
	curend = paint_page(gwin, Rectangle(48, 30, 146, 118), curtop);
	}

/*
 *	Set slider value.
 */

void Slider_gump_object::set_val
	(
	int newval
	)
	{
	val = newval;
	static int xdist = xmax - xmin;
	if(max_val-min_val==0)
		{
		val=0;
		diamondx=xmin;
		}
	else
		diamondx = xmin + ((val - min_val)*xdist)/(max_val - min_val);
	}

/*
 *	Create a slider.
 */

Slider_gump_object::Slider_gump_object
	(
	int mival, int mxval,		// Value range.
	int step,			// Amt. to change by.
	int defval			// Default value.
	) : Modal_gump_object(0, game->get_shape("gumps/slider")),
	    min_val(mival), max_val(mxval), step_val(step),
	    val(defval), dragging(0), prev_dragx(0)
	{
cout << "Slider:  " << min_val << " to " << max_val << " by " << step << endl;
	left_arrow = new Slider_gump_button(this, leftbtnx, btny, game->get_shape("gumps/slider_left"));
	right_arrow = new Slider_gump_button(this, rightbtnx, btny, 
								game->get_shape("gumps/slider_right"));
					// Init. to middle value.
	set_val(defval);
	}

/*
 *	Delete slider.
 */

Slider_gump_object::~Slider_gump_object
	(
	)
	{
	delete left_arrow;
	delete right_arrow;
	}

/*
 *	An arrow on the slider was clicked.
 */

void Slider_gump_object::clicked_arrow
	(
	Slider_gump_button *arrow	// What was clicked.
	)
	{
	int newval = val;
	if (arrow == left_arrow)
		{
		newval -= step_val;
		if (newval < min_val)
			newval = min_val;
		}
	else if (arrow == right_arrow)
		{
		newval += step_val;
		if (newval > max_val)
			newval = max_val;
		}
	set_val(newval);
	Game_window *gwin = Game_window::get_game_window();
	paint(gwin);
	gwin->set_painted();
	}

/*
 *	Paint on screen.
 */

void Slider_gump_object::paint
	(
	Game_window *gwin
	)
	{
	const int textx = 128, texty = 7;
					// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum());
					// Paint red "checkmark".
	paint_button(gwin, check_button);
					// Paint buttons.
	paint_button(gwin, left_arrow);
	paint_button(gwin, right_arrow);
					// Paint slider diamond.
	gwin->paint_gump(x + diamondx, y + diamondy, game->get_shape("gumps/slider_diamond"), 0);
					// Print value.
  	Paint_num(gwin, val, x + textx, y + texty);
	gwin->set_painted();
	}

/*
 *	Handle mouse-down events.
 */

void Slider_gump_object::mouse_down
	(
	int mx, int my			// Position in window.
	)
	{
	dragging = 0;
	Game_window *gwin = Game_window::get_game_window();
	Gump_button *btn = Gump_object::on_button(gwin, mx, my);
	if (btn)
		pushed = btn;
	else if (left_arrow->on_button(gwin, mx, my))
		pushed = left_arrow;
	else if (right_arrow->on_button(gwin, mx, my))
		pushed = right_arrow;
	else
		pushed = 0;
	if (pushed)
		{
		pushed->push(gwin);
		return;
		}
					// See if on diamond.
	Shape_frame *diamond = gwin->get_gump_shape(game->get_shape("gumps/slider_diamond"), 0);
	if (diamond->has_point(mx - (x + diamondx), my - (y + diamondy)))
		{			// Start to drag it.
		dragging = 1;
		prev_dragx = mx;
		}
	}

/*
 *	Handle mouse-up events.
 */

void Slider_gump_object::mouse_up
	(
	int mx, int my			// Position in window.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (dragging)			// Done dragging?
		{
		set_val(val);		// Set diamond in correct pos.
		paint(gwin);
		gwin->set_painted();
		dragging = 0;
		}
	if (!pushed)
		return;
	pushed->unpush(gwin);
	if (pushed->on_button(gwin, mx, my))
		pushed->activate(gwin);
	pushed = 0;
	}

/*
 *	Mouse was dragged with left button down.
 */

void Slider_gump_object::mouse_drag
	(
	int mx, int my			// Where mouse is.
	)
	{
	if (!dragging)
		return;
	diamondx += mx - prev_dragx;
	prev_dragx = mx;
	if (diamondx < xmin)		// Stop at ends.
		diamondx = xmin;
	else if (diamondx > xmax)
		diamondx = xmax;
	static int xdist = xmax - xmin;
	int delta = (diamondx - xmin)*(max_val - min_val)/xdist;
					// Round down to nearest step.
	delta -= delta%step_val;
	int newval = min_val + delta;
	if (newval != val)		// Set value.
		val = newval;
	paint(Game_window::get_game_window());
	}

/*
 *	Handle ASCII character typed.
 */

void Slider_gump_object::key_down
	(
	int chr
	)
	{
	if (chr == SDLK_RETURN)
		{
		done = 1;
		}
	}

/*
 *	Create the load/save box.
 */

File_gump_object::File_gump_object
	(
	) : Modal_gump_object(0, game->get_shape("gumps/fileio")),
		pushed_text(0), focus(0), restored(0)
	{
	Game_window *gwin = Game_window::get_game_window();
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
	buttons[2] = new Quit_gump_button(this, btn_cols[2], btn_rows[0]);
					// 2nd row.
	buttons[3] = new Sound_gump_button(this, btn_cols[0], btn_rows[1], 
			game->get_shape("gumps/musicbtn"),
						audio->is_music_enabled());
	buttons[4] = new Sound_gump_button(this, btn_cols[1], btn_rows[1],
			game->get_shape("gumps/speechbtn"),
						audio->is_speech_enabled());
	buttons[5] = new Sound_gump_button(this, btn_cols[2], btn_rows[1],
			game->get_shape("gumps/soundbtn"),
						audio->are_effects_enabled());
	}

/*
 *	Delete the load/save box.
 */

File_gump_object::~File_gump_object
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

int File_gump_object::get_save_index
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
 *	'Load' clicked.
 */

void File_gump_object::load
	(
	)
	{
	if (!focus ||			// This would contain the name.
	    !focus->get_length())
		return;
	int num = get_save_index(focus);// Which one is it?
	if (num == -1)
		return;			// Shouldn't ever happen.
	if (!Yesno_gump_object::ask(
			"Okay to load over your current game?"))
		return;
	Game_window *gwin = Game_window::get_game_window();
	gwin->restore_gamedat(num);	// Aborts if unsuccessful.
	gwin->read();			// And read the files in.
	done = 1;
	restored = 1;
	}

/*
 *	'Save' clicked.
 */

void File_gump_object::save
	(
	)
	{
	if (!focus || 			// This would contain the name.
	    !focus->get_length())
		return;
	int num = get_save_index(focus);// Which one is it?
	if (num == -1)
		return;			// Shouldn't ever happen.
	Game_window *gwin = Game_window::get_game_window();
	if (*gwin->get_save_name(num))	// Already a game in this slot?
		if (!Yesno_gump_object::ask(
			"Okay to write over existing saved game?"))
			return;
	if (gwin->write() &&		// First flush to 'gamedat'.
	    gwin->save_gamedat(num, focus->get_text()))
		cout << "Saved game #" << num << " successfully."<<endl;
	}

/*
 *	'Quit' clicked.
 */

void File_gump_object::quit
	(
	)
	{
	extern unsigned char quitting_time;
	if (!Yesno_gump_object::ask("Do you really want to quit?"))
		return;
	quitting_time = 1;
	done = 1;
	}

/*
 *	One of the option toggle buttons was pressed.
 *
 *	Output:	New state of option (0 or 1).
 */

int File_gump_object::toggle_option
	(
	Gump_button *btn		// Button that was clicked.
	)
	{
	if (btn == buttons[3])		// Music?
		{
		bool music = !audio->is_music_enabled();
		audio->set_music_enabled(music);
		if (!music)		// Stop what's playing.
			audio->stop_music();
		string s = music ? "yes" : "no";
					// Write option out.
		config->set("config/audio/midi/enabled", s, true);
		return music ? 1 : 0;
		}
	if (btn == buttons[4])		// Speech?
		{
		bool speech = !audio->is_speech_enabled();
		audio->set_speech_enabled(speech);
		string s = speech ? "yes" : "no";
					// Write option out.
		config->set("config/audio/speech/enabled", s, true);
		return speech ? 1 : 0;
		}
	if (btn == buttons[5])		// Sound effects?
		{
		bool effects = !audio->are_effects_enabled();
		audio->set_effects_enabled(effects);
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

void File_gump_object::paint
	(
	Game_window *gwin
	)
	{
	Gump_object::paint(gwin);	// Paint gump & objects.
					// Paint text objects.
	size_t i;
	for (i = 0; i < sizeof(names)/sizeof(names[0]); i++)
		if (names[i])
			names[i]->paint();
	for (i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
		if (buttons[i])
			paint_button(gwin, buttons[i]);
	}

/*
 *	Handle mouse-down events.
 */

void File_gump_object::mouse_down
	(
	int mx, int my			// Position in window.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	pushed = 0;
	pushed_text = 0;
					// First try checkmark.
	Gump_button *btn = Gump_object::on_button(gwin, mx, my);
	if (btn)
		pushed = btn;
	else				// Try buttons at bottom.
		for (size_t i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
			if (buttons[i] && buttons[i]->on_button(gwin, mx, my))
				{
				pushed = buttons[i];
				break;
				}
	if (pushed)			// On a button?
		{
		pushed->push(gwin);
		return;
		}
					// See if on text field.
	for (size_t i = 0; i < sizeof(names)/sizeof(names[0]); i++)
		if (names[i]->on_widget(gwin, mx, my))
			{
			pushed_text = names[i];
			break;
			}
	}

/*
 *	Handle mouse-up events.
 */

void File_gump_object::mouse_up
	(
	int mx, int my			// Position in window.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (pushed)			// Pushing a button?
		{
		pushed->unpush(gwin);
		if (pushed->on_button(gwin, mx, my))
			pushed->activate(gwin);
		pushed = 0;
		}
	if (!pushed_text)
		return;
					// Let text field handle it.
	if (!pushed_text->mouse_clicked(gwin, mx, my) ||
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
			buttons[0] = new Load_save_gump_button(this,
					btn_cols[0], btn_rows[0], game->get_shape("gumps/loadbtn"));
		if (!buttons[1])
			buttons[1] = new Load_save_gump_button(this,
					btn_cols[1], btn_rows[0], game->get_shape("gumps/savebtn"));
		}
	else if (!focus->get_length())
		{			// No name yet.
		delete buttons[0];
		delete buttons[1];
		buttons[0] = buttons[1] = 0;
		}
	paint(gwin);			// Repaint.
	gwin->set_painted();
	}

/*
 *	Handle character that was typed.
 */

void File_gump_object::key_down
	(
	int chr
	)
	{
	if (!focus)			// Text field?
		return;
	switch (chr)
		{
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
			paint(Game_window::get_game_window());
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
			paint(Game_window::get_game_window());
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
	if (chr < ' ')
		return;			// Ignore other special chars.
	if (chr < 256 && isascii(chr))
		{
		int old_length = focus->get_length();
		focus->insert(chr);
		Game_window *gwin = Game_window::get_game_window();
					// Added first character?  Need 
					//   'Save' button.
		if (!old_length && focus->get_length() && !buttons[1])
			{
			buttons[1] = new Load_save_gump_button(this,
					btn_cols[1], btn_rows[0], game->get_shape("gumps/savebtn"));
			paint_button(gwin, buttons[1]);
			}
		if (buttons[0])		// Can't load now.
			{
			delete buttons[0];
			buttons[0] = 0;
			paint(gwin);
			}
		gwin->set_painted();
		}
	}

/*
 *	Create a yes/no box.
 */

Yesno_gump_object::Yesno_gump_object
	(
	const char *txt
	) : Modal_gump_object(0, game->get_shape("gumps/yesnobox")), text(strdup(txt)), answer(-1)
	{
	yes_button = new Yesno_gump_button(this, yesx, yesnoy, 1);
	no_button = new Yesno_gump_button(this, nox, yesnoy, 0);
	}

/*
 *	Done with yes/no box.
 */

Yesno_gump_object::~Yesno_gump_object
	(
	)
	{
	delete yes_button;
	delete no_button;
	delete text;
	}

/*
 *	Paint on screen.
 */

void Yesno_gump_object::paint
	(
	Game_window *gwin
	)
	{
					// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum());
					// Paint buttons.
	paint_button(gwin, yes_button);
	paint_button(gwin, no_button);
					// Paint text.
	gwin->paint_text_box(2, text, x + object_area.x, y + object_area.y,
			object_area.w, object_area.h, 2);
	gwin->set_painted();
	}

/*
 *	Handle mouse-down events.
 */

void Yesno_gump_object::mouse_down
	(
	int mx, int my			// Position in window.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Check buttons.
	if (yes_button->on_button(gwin, mx, my))
		pushed = yes_button;
	else if (no_button->on_button(gwin, mx, my))
		pushed = no_button;
	else
		{
		pushed = 0;
		return;
		}
	pushed->push(gwin);		// Show it.
	}

/*
 *	Handle mouse-up events.
 */

void Yesno_gump_object::mouse_up
	(
	int mx, int my			// Position in window.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (pushed)			// Pushing a button?
		{
		pushed->unpush(gwin);
		if (pushed->on_button(gwin, mx, my))
			pushed->activate(gwin);
		pushed = 0;
		}
	}

/*
 *	Handle ASCII character typed.
 */

void Yesno_gump_object::key_down
	(
	int chr
	)
	{
	if (chr == 'y' || chr == 'Y' || chr == SDLK_RETURN)
		set_answer(1);
	else if (chr == 'n' || chr == 'N' || chr == SDLK_ESCAPE)
		set_answer(0);
	}

/*
 *	Get an answer to a question.
 *
 *	Output:	1 if yes, 0 if no or ESC.
 */

int Yesno_gump_object::ask
	(
	const char *txt			// What to ask.
	)
	{
	extern int Modal_gump(Modal_gump_object *, Mouse::Mouse_shapes);
	Yesno_gump_object *dlg = new Yesno_gump_object(txt);
	int answer;
	if (!Modal_gump(dlg, Mouse::hand))
		answer = 0;
	else
		answer = dlg->get_answer();
	delete dlg;
	return (answer);
	}

/*
 *
 *	SERPENT ISLE GUMPS
 *
 */

/*
 *	The Combat Stats Button.
 */
class Cstats_gump_button : public Gump_button
	{
public:
	Cstats_gump_button(Gump_object *par, int px, int py)
		: Gump_button(par, game->get_shape("gumps/combat_stats"), px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
	};

/*
 *	Handle click on a combat stats button
 */

void Cstats_gump_button::activate
	(
	Game_window *gwin
	)
	{
					// ++++++Later.
	}


/*
 *	Find the index of the closest 'spot' to a mouse point.
 *
 *	Output:	Index, or -1 if unsuccessful.
 */

int Paperdoll_gump_object::find_closest
	(
	int mx, int my,			// Mouse point in window.
	int only_empty			// Only allow empty spots.
	)
	{
	mx -= x; my -= y;		// Get point rel. to us.
	long closest_squared = 1000000;	// Best distance squared.
	int closest = -1;		// Best index.
	for (size_t i = 0; i < sizeof(coords)/(2*sizeof(coords[0])); i++)
		{
		int dx = mx - spotx(i), dy = my - spoty(i);
		long dsquared = dx*dx + dy*dy;
					// Better than prev.?
		if (dsquared < closest_squared && (!only_empty ||
						!container->get_readied(i)))
			{
			closest_squared = dsquared;
			closest = i;
			}
		}
	return (closest);
	}

/*
 *	Create the gump display for an actor.
 */

Paperdoll_gump_object::Paperdoll_gump_object
	(
	Container_game_object *cont,	// Container it represents.
	int initx, int inity, 		// Coords. on screen.
	int shnum			// Shape #.
	) : Gump_object(cont, initx, inity, 123, true)
	{
	heart_button = new Heart_gump_button(this, heartx, hearty);
	if (cont->get_npc_num() == 0) disk_button = new Disk_gump_button(this, diskx, disky);
	else disk_button = NULL;
	if (cont->get_npc_num() == 0) combat_button = new Combat_gump_button(this, combatx, combaty);
	else combat_button = NULL;
	cstats_button = new Cstats_gump_button(this, cstatx, cstaty);
							
	for (size_t i = 0; i < sizeof(coords)/2*sizeof(coords[0]); i++)
		{			// Set object coords.
		Game_object *obj = container->get_readied(i);
		if (obj)
			set_to_spot(obj, i);
		}
	}

/*
 *	Delete actor display.
 */

Paperdoll_gump_object::~Paperdoll_gump_object
	(
	)
	{
	if (heart_button) delete heart_button;
	if (disk_button) delete disk_button;
	if (combat_button) delete combat_button;
	if (cstats_button) delete cstats_button;
	}

/*
 *	Is a given screen point on one of our buttons?
 *
 *	Output: ->button if so.
 */

Gump_button *Paperdoll_gump_object::on_button
	(
	Game_window *gwin,
	int mx, int my			// Point in window.
	)
	{
	Gump_button *btn = Gump_object::on_button(gwin, mx, my);
	if (btn)
		return btn;
	else if (heart_button && heart_button->on_button(gwin, mx, my))
		return heart_button;
	else if (disk_button && disk_button->on_button(gwin, mx, my))
		return disk_button;
	else if (combat_button && combat_button->on_button(gwin, mx, my))
		return combat_button;
	else if (cstats_button && cstats_button->on_button(gwin, mx, my))
		return cstats_button;
	return 0;
	}

/*
 *	Add an object.
 *
 *	Output:	0 if cannot add it.
 */

int Paperdoll_gump_object::add
	(
	Game_object *obj,
	int mx, int my,			// Screen location of mouse.
	int sx, int sy			// Screen location of obj's hotspot.
	)
	{
					// Find index of closest spot.
	int index = find_closest(mx, my);
	if (!container->add_readied(obj, index))
		{			// Can't add it there?
					// Try again for an empty spot.
		index = find_closest(mx, my, 1);
		if (index < 0 || !container->add_readied(obj, index))
					// Just try to add it.
			if (!container->add(obj))
				return (0);
		}
					// In case it went in another obj:
	index = container->find_readied(obj);
	if (index >= 0)
		set_to_spot(obj, index);// Set obj. coords.
	return (1);
	}

/*
 *	Set object's coords. to given spot.
 */

void Paperdoll_gump_object::set_to_spot
	(
	Game_object *obj,
	int index			// Spot index.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Get shape info.
	Shape_frame *shape = gwin->get_shape(*obj);
	int w = shape->get_width(), h = shape->get_height();
					// Set object's position.
	obj->set_chunk(spotx(index) + shape->get_xleft() - w/2 - object_area.x,
		spoty(index) + shape->get_yabove() - h/2 - object_area.y);
					// Shift if necessary.
	int x0 = obj->get_cx() - shape->get_xleft(), 
	    y0 = obj->get_cy() - shape->get_yabove();
	int newcx = obj->get_cx(), newcy = obj->get_cy();
	if (x0 < 0)
		newcx -= x0;
	if (y0 < 0)
		newcy -= y0;
	int x1 = x0 + w, y1 = y0 + h;
	if (x1 > object_area.w)
		newcx -= x1 - object_area.w;
	if (y1 > object_area.h)
		newcy -= y1 - object_area.h;
	obj->set_chunk(newcx, newcy);
	}

/*
 *	Paint on screen.
 */

void Paperdoll_gump_object::paint
	(
	Game_window *gwin
	)
	{
					// Watch for any newly added objs.
	for (size_t i = 0; i < sizeof(coords)/2*sizeof(coords[0]); i++)
		{			// Set object coords.
		Game_object *obj = container->get_readied(i);
		if (obj && !obj->get_cx() && !obj->get_cy())
			set_to_spot(obj, i);
		}
	Gump_object::paint(gwin);	// Paint gump & objects.
					// Paint buttons.
	if (heart_button) paint_button(gwin, heart_button);
	if (disk_button) paint_button(gwin, disk_button);
	if (combat_button) paint_button(gwin, combat_button);
	if (cstats_button) paint_button(gwin, cstats_button);
	}

