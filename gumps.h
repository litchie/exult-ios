/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Gumps.h - Open containers and their contents.
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

#ifndef INCL_GUMPS
#define INCL_GUMPS

#include "rect.h"
#include "shapeid.h"
#include "utils.h"

class Actor;
class Game_object;
class Container_game_object;
class Game_window;
class Gump_object;
class Gump_button;
class Gump_text;
class Checkmark_gump_button;
class Heart_gump_button;
class Disk_gump_button;
class Combat_gump_button;
class Halo_gump_button;
class Combat_mode_gump_button;
class Cstats_gump_button;
class Yesno_gump_button;
class Slider_gump_button;

/*
 *	A gump widget, such as a button or text field:
 */
class Gump_widget
	{
	UNREPLICATABLE_CLASS(Gump_widget);
protected:
	Gump_widget() : parent(0) {  }
	Gump_object *parent;		// Who this is in.
	int shapenum;			// In "gumps.vga".
	int framenum;			// Frame # (usually 0) when unpushed.
	short x, y;			// Coords. relative to parent.
public:
	friend class Gump_object;
	friend class Spellbook_gump;
	Gump_widget(Gump_object *par, int shnum, int px, int py)
		: parent(par), shapenum(shnum), framenum(0), x(px), y(py)
		{  }
					// Is a given point on the widget?
	int on_widget(Game_window *gwin, int mx, int my);
	};

/*
 *	A pushable button on a gump:
 */
class Gump_button : public Gump_widget
	{
private:
	UNREPLICATABLE_CLASS(Gump_button);
protected:
	unsigned char pushed;		// 1 if in pushed state.
public:
	friend class Gump_object;
	Gump_button(Gump_object *par, int shnum, int px, int py)
		: Gump_widget(par, shnum, px, py), pushed(0)
		{  }
					// Is a given point on the checkmark?
	int on_button(Game_window *gwin, int mx, int my)
		{ return on_widget(gwin, mx, my); }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin) = 0;
					// Or double-clicked.
	virtual void double_clicked(Game_window *gwin);
	void push(Game_window *gwin);	// Redisplay as pushed.
	void unpush(Game_window *gwin);
	};

/*
 *	A gump contains an image of an open container from "gumps.vga".
 */
class Gump_object : public ShapeID
	{
	UNREPLICATABLE_CLASS(Gump_object);
protected:
	Gump_object() : ShapeID(), paperdoll_shape (false) {   };
	Gump_object *next;		// ->next to draw.
	Container_game_object *container;// What this gump shows.
	int x, y;			// Location on screen.
	unsigned char shapenum;
	Rectangle object_area;		// Area to paint objects in, rel. to
					// Where the 'checkmark' goes.
	Checkmark_gump_button *check_button;
	void initialize();		// Initialize object_area.
	void initialize2();		// Initialize object_area (paperdoll).
	bool paperdoll_shape;
public:
	Gump_object(Container_game_object *cont, int initx, int inity, 
								int shnum, bool pdoll = false);
					// Create centered.
	Gump_object(Container_game_object *cont, int shnum);
	virtual ~Gump_object();
	int get_x()			// Get coords.
		{ return x; }
	int get_y()
		{ return y; }
	void set_pos(int newx, int newy)// Set new spot on screen.
		{
		x = newx;
		y = newy;
		}
					// Append to end of chain.
	void append_to_chain(Gump_object *& chain);
					// Remove from chain.
	void remove_from_chain(Gump_object *& chain);
	Gump_object *get_next()		// (Chain ends with ->next == 0.)
		{ return next; }
	Container_game_object *get_container()
		{ return container; }
					// Get screen rect. of obj. in here.
	Rectangle get_shape_rect(Game_object *obj);
					// Get screen loc. of object.
	void get_shape_location(Game_object *obj, int& ox, int& oy);
					// Find obj. containing mouse point.
	Game_object *find_object(int mx, int my);
	virtual Game_object *get_owner();// Get object this belongs to.
					// Is a given point on a button?
	virtual Gump_button *on_button(Game_window *gwin, int mx, int my);
					// Paint button.
	virtual void paint_button(Game_window *gwin, Gump_button *btn);
					// Add object.
	virtual int add(Game_object *obj, int mx = -1, int my = -1,
						int sx = -1, int sy = -1);
	virtual void remove(Game_object *obj);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
					// Close (and delete).
	virtual void close(Game_window *gwin);
					// Use the paperdoll shapes?
	bool is_paperdoll() const { return paperdoll_shape; }
	};

/*
 *	A rectangular area showing a character and his/her possessions:
 */
class Actor_gump_object : public Gump_object
	{
	UNREPLICATABLE_CLASS(Actor_gump_object);
	Heart_gump_button *heart_button;// For bringing up stats.
	Disk_gump_button *disk_button;	// For bringing up 'save' box.
	Combat_gump_button *combat_button;
	Halo_gump_button *halo_button;
	Combat_mode_gump_button *cmode_button;
	static short coords[24];	// Coords. of where to draw things,
					//   indexed by spot # (0-11).
	static int spotx(int i) { return coords[2*i]; }
	static int spoty(int i) { return coords[2*i + 1]; }
					// Find index of closest spot.
	int find_closest(int mx, int my, int only_empty = 0);
	void set_to_spot(Game_object *obj, int index);
	static short diskx, disky;	// Where to show 'diskette' button.
	static short heartx, hearty;	// Where to show 'stats' button.
	static short combatx, combaty;	// Combat button.
	static short halox, haloy;	// "Protected" halo.
	static short cmodex, cmodey;	// Combat mode.
public:
	Actor_gump_object(Container_game_object *cont, int initx, int inity, 
								int shnum);
	~Actor_gump_object();
					// Is a given point on a button?
	virtual Gump_button *on_button(Game_window *gwin, int mx, int my);
					// Add object.
	virtual int add(Game_object *obj, int mx = -1, int my = -1,
						int sx = -1, int sy = -1);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

class Paperdoll_gump_object : public Gump_object 
	{
	UNREPLICATABLE_CLASS(Paperdoll_gump_object);
	Heart_gump_button *heart_button;// For bringing up stats.
	Disk_gump_button *disk_button;	// For bringing up 'save' box.
	Combat_gump_button *combat_button;
	Cstats_gump_button *cstats_button;
	static short coords[26];	// Coords. of where to draw things,
					//   indexed by spot # (0-11).
	static int spotx(int i) { return coords[2*i]; }
	static int spoty(int i) { return coords[2*i + 1]; }
					// Find index of closest spot.
	int find_closest(int mx, int my, int only_empty = 0);
	void set_to_spot(Game_object *obj, int index);
	static short diskx, disky;	// Where to show 'diskette' button.
	static short heartx, hearty;	// Where to show 'stats' button.
	static short combatx, combaty;	// Combat button.
	static short cstatx, cstaty;	// Combat mode.
public:
	Paperdoll_gump_object(Container_game_object *cont, int initx, int inity, 
								int shnum);

	~Paperdoll_gump_object();
					// Is a given point on a button?
	virtual Gump_button *on_button(Game_window *gwin, int mx, int my);
					// Add object.
	virtual int add(Game_object *obj, int mx = -1, int my = -1,
						int sx = -1, int sy = -1);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

/*
 *	A rectangular area showing a character's statistics:
 */
class Stats_gump_object : public Gump_object
	{
	UNREPLICATABLE_CLASS(Stats_gump_object);
	Actor *get_actor()
		{ return (Actor *) container; }
	static short textx;		// X-coord. of where to write.
	static short texty[10];		// Y-coords.
public:
	Stats_gump_object(Container_game_object *cont, int initx, int inity);
	~Stats_gump_object()
		{  }
					// Add object.
	virtual int add(Game_object *obj, int mx = -1, int my = -1,
						int sx = -1, int sy = -1)
		{ return 0; }		// Can't drop onto it.
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

/*
 *	A sign showing runes.
 */
class Sign_gump : public Gump_object
	{
	UNREPLICATABLE_CLASS(Sign_gump);
	char **lines;			// Lines of text.
	int num_lines;
public:
	Sign_gump(int shapenum, int nlines);
	~Sign_gump();
					// Set a line of text.
	void add_text(int line, const char *txt);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

/*
 *	A text gump is the base class for books and scrolls.
 */
class Text_gump : public Gump_object
	{
	UNREPLICATABLE_CLASS(Text_gump);
	char *text;			// The text.
	int textlen;			// Length of text.
protected:
	int curtop;			// Offset of top of current page.
	int curend;			// Offset past end of current page(s).
public:
	Text_gump(int shapenum) : Gump_object(0, shapenum),
				text(0), textlen(0), curtop(0), curend(0)
		{  }
	~Text_gump()
		{ delete text; }
	void add_text(char *str);	// Append text.
	int paint_page(Game_window *gwin, Rectangle box, int start);
					// Next page of book/scroll.
	int show_next_page(Game_window *gwin);
	};

/*
 *	A book shows text side-by-side.
 */
class Book_gump : public Text_gump
	{
	UNREPLICATABLE_CLASS_I(Book_gump,Text_gump(0));
public:
	Book_gump();
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

/*
 *	A scroll:
 */
class Scroll_gump : public Text_gump
	{
	UNREPLICATABLE_CLASS_I(Scroll_gump,Text_gump(0));
public:
	Scroll_gump();
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	};

/*
 *	A modal gump object represents a 'dialog' that grabs the mouse until
 *	the user clicks okay.
 */
class Modal_gump_object : public Gump_object
	{
	UNREPLICATABLE_CLASS(Modal_gump_object);
protected:
	int done;			// 1 when user clicks checkmark.
	Gump_button *pushed;		// Button currently being pushed.
public:
	Modal_gump_object(Container_game_object *cont, int initx, int inity, 
								int shnum)
		: Gump_object(cont, initx, inity, shnum), done(0), pushed(0)
		{  }
					// Create centered.
	Modal_gump_object(Container_game_object *cont, int shnum)
		: Gump_object(cont, shnum), done(0), pushed(0)
		{  }
	int is_done()
		{ return done; }
					// Handle events:
	virtual void mouse_down(int mx, int my) = 0;
	virtual void mouse_up(int mx, int my) = 0;
	virtual void mouse_drag(int mx, int my)
		{  }
	virtual void key_down(int chr)	// Character typed.
		{  }
	};

/*
 *	A slider for choosing a number.
 */
class Slider_gump_object : public Modal_gump_object
	{
	UNREPLICATABLE_CLASS_I(Slider_gump_object,Modal_gump_object(0,0,0,0));
					// The arrows at each end:
	Slider_gump_button *left_arrow, *right_arrow;
	int diamondx;			// Rel. pos. where diamond is shown.
	static short diamondy;
	int min_val, max_val;		// Max., min. values to choose from.
	int step_val;			// Amount to step by.
	int val;			// Current value.
	unsigned char dragging;		// 1 if dragging the diamond.
	int prev_dragx;			// Prev. x-coord. of mouse.
	void set_val(int newval);	// Set to new value.
					// Coords:
	static short leftbtnx, rightbtnx, btny;
	static short xmin, xmax;
public:
	Slider_gump_object(int mival, int mxval, int step, int defval);
	~Slider_gump_object();
	int get_val()			// Get last value set.
		{ return val; }
					// An arrow was clicked on.
	void clicked_arrow(Slider_gump_button *arrow);
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	virtual void close(Game_window *gwin)
		{ done = 1; }
					// Handle events:
	virtual void mouse_down(int mx, int my);
	virtual void mouse_up(int mx, int my);
	virtual void mouse_drag(int mx, int my);
	virtual void key_down(int chr);	// Character typed.
	};

/*
 *	The file save/load box:
 */
class File_gump_object : public Modal_gump_object
	{
	UNREPLICATABLE_CLASS_I(File_gump_object,Modal_gump_object(0,0,0,0));
	static short textx, texty;	// Where to draw first text field.
	static short texth;		// Distance down to next text field.
	static short btn_rows[2];	// y-coord of each button row.
	static short btn_cols[3];	// x-coord of each button column.
	Gump_text *names[10];		// 10 filename slots.
	Gump_button *buttons[6];	// 2 rows, 3 cols of buttons.
	Gump_text *pushed_text;		// Text mouse is down on.
	Gump_text *focus;		// Text line that has focus.
	unsigned char restored;		// Set to 1 if we restored a game.
public:
	File_gump_object();
	~File_gump_object();
					// Find savegame index of text field.
	int get_save_index(Gump_text *txt);
	void load();			// 'Load' was clicked.
	void save();			// 'Save' was clicked.
	void quit();			// 'Quit' was clicked.
					// Handle one of the toggles.
	int toggle_option(Gump_button *btn);
	int restored_game()		// 1 if user restored.
		{ return restored; }
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	virtual void close(Game_window *gwin)
		{ done = 1; }
					// Handle events:
	virtual void mouse_down(int mx, int my);
	virtual void mouse_up(int mx, int my);
	virtual void key_down(int chr);	// Character typed.
	};

/*
 *	A yes/no box.  
 */
class Yesno_gump_object : public Modal_gump_object
	{
	UNREPLICATABLE_CLASS_I(Yesno_gump_object,Modal_gump_object(0,0,0,0));
	static short yesx, yesnoy, nox;	// Coords. of the buttons.
	const char *text;			// Text of question.  It is drawn in
					//   object_area.
	int answer;			// 1 for yes, 0 for no.
	Yesno_gump_button *yes_button, *no_button;
	void set_answer(int y)		// Done from 'yes'/'no' button.
		{
		answer = (y != 0);
		done = 1;
		}
public:
	friend class Yesno_gump_button;
	Yesno_gump_object(const char *txt);
	~Yesno_gump_object();
	int get_answer()
		{ return answer; }
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
					// Handle events:
	virtual void mouse_down(int mx, int my);
	virtual void mouse_up(int mx, int my);
	virtual void key_down(int chr);	// Character typed.
	static int ask(const char *txt);	// Ask question, get answer.
	};

#endif	/* INCL_GUMPS */
