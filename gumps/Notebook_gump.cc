/*
Copyright (C) 2000-2004 The Exult Team

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

#include "Notebook_gump.h"
#include "Gump_button.h"
#include "exult_flx.h"
#include "game.h"
#include "gamewin.h"
#include "gameclk.h"
#include "actors.h"
#include "SDL_events.h"
#include "Configuration.h"

using std::ofstream;
using std::ostream;
using std::endl;
using std::string;
using std::cout;

vector<One_note *> Notebook_gump::notes;
bool Notebook_gump::initialized = false;	// Set when read in.
Notebook_gump *Notebook_gump::instance = 0;
vector<Notebook_top> Notebook_gump::page_info;

/*
 *	Defines in 'gumps.vga':
 */
#define LEFTPAGE  (GAME_BG ? 44 : 39)	// At top-left of left page.
#define RIGHTPAGE  (GAME_BG ? 45 : 40)	// At top-right of right page.

const int font = 4;			// Small black.
const int vlead = 1;			// Extra inter-line spacing.
const int pagey = 10;			// Top of page.

class One_note
{
	int day, hour, minute;		// Game time when note was written.
	int tx, ty;			// Tile coord. where written.
	char *text;			// Text, 0-delimited.
	int textlen;			// Length, not counting ending NULL.
	int textmax;			// Max. space.
	bool is_new;			// Newly created at cur. time/place.
public:
	friend class Notebook_gump;
	One_note() : day(0), hour(0), minute(0), tx(0), ty(0), 
					text(0), textlen(0), textmax(0)
		{  }
	void set_time(int d, int h, int m)
		{ day = d; hour = h; minute = m; }
	void set_loc(int x, int y)
		{ tx = x; ty = y; }
	void set_text(char *txt);
	void set(int d, int h, int m, int x, int y, char *txt = 0)
		{
		set_time(d, h,m);
		set_loc(x, y);
		set_text(txt);
		}
	One_note(int d, int h, int m, int x, int y, char *txt = 0) : text(0)
		{ set(d, h, m, x, y, txt); }
	~One_note()
		{ delete [] text; }
	void insert(int chr, int offset);	// Insert text.
	bool del(int offset);			// Delete text.
	void write(ostream& out);		// Write out as XML.
};

/*
 *	Setup one note (from already-allocated text).
 */

void One_note::set_text
	(
	char *txt
	)
	{
	delete text;
	text = txt;
	if (text)
		{
		textlen = strlen(text);
		textmax = textlen + 1;
		}
	else
		{
		textlen = 0;
		textmax = 16;
		text = new char[textmax];
		text[0] = 0;
		}
	}

/*
 *	Insert one character.
 */

void One_note::insert
	(
	int chr,
	int offset
	)
	{
	if (textlen + 1 >= textmax)
		{			// Need more space.
		textmax = textmax ? (textmax + textmax/4 + 1) : 16;
		char *newtext = new char[textmax];
		memcpy(newtext, text, offset);
		newtext[offset] = chr;
		memcpy(newtext + offset + 1, text + offset, 
					textlen + 1 - offset);
		delete [] text;
		text = newtext;
		}
	else
		{
		memmove(text + offset + 1, text + offset,
					textlen + 1 - offset);
		text[offset] = chr;
		}
	++textlen;
	}

/*
 *	Delete one character.
 *
 *	Output:	true if successful.
 */

bool One_note::del
	(
	int offset			// Delete to right of this.
	)
	{
	if (offset >= textlen || offset < 0)
		return false;
	memmove(text + offset, text + offset + 1, textlen - offset);
	--textlen;
	return true;
	}

/*
 *	Write out as XML.
 */

void One_note::write
	(
	ostream& out
	)
	{
	out << "<note>" << endl;
	out << "<time> " << day << ':' << hour << ':' << minute <<
		" </time>" << endl;
	out << "<place> " << tx << ':' << ty << " </place>" << endl;
	out << "<text>" << endl;
	out.write(text, textlen);
	out << endl << "</text>" << endl;
	out << "</note>" << endl;
	}

/*
 *	Get left/right text area.
 */

inline Rectangle Get_text_area(bool right, bool startnote)
	{
	const int ninf = 12;		// Space for note info.
	if (!startnote)
		return right ? Rectangle(174, pagey, 122, 130)
			     : Rectangle(36, pagey, 122, 130);
	else
		return right ? Rectangle(174, pagey+ninf, 122, 130-ninf)
			     : Rectangle(36, pagey+ninf, 122, 130-ninf);
	}

/*
 *	A 'page-turner' button.
 */
class Notebook_page_button : public Gump_button
	{
	int leftright;			// 0=left, 1=right.
public:
	Notebook_page_button(Gump *par, int px, int py, int lr)
		: Gump_button(par, lr ? RIGHTPAGE : LEFTPAGE, px, py),
		  leftright(lr)
		{  }
					// What to do when 'clicked':
	virtual void activate();
	virtual void push() {}
	virtual void unpush() {}
	};

/*
 *	Handle click.
 */

void Notebook_page_button::activate
	(
	)
	{
	((Notebook_gump *) parent)->change_page(leftright ? 1 : -1);
	}

/*
 *	Read in notes the first time.
 */

void Notebook_gump::initialize
	(
	)
	{
	initialized = true;
	read();
#if 0
	// ++++TESTING:
	notes.push_back(new One_note(1, 1,10, 10, 10, 
				strdup("Note  #1\nHello")));
	notes.push_back(new One_note(2, 2,20, 20, 20, strdup(
				"Note  #2\nworld.\n\nHow are you?")));
	notes.push_back(new One_note(3, 3,30, 30, 30, strdup(
				"Note #3")));
#endif
	}

/*
 *	Clear out.
 */

void Notebook_gump::clear
	(
	)
	{
	while (notes.size())
		{
		One_note *note = notes.back();
		delete note;
		notes.pop_back();
		}
	initialized = false;
	}


/*
 *	Add a new note at the current time/place.
 */

void Notebook_gump::add_new
	(
	)
	{
	Game_clock *clk = gwin->get_clock();
	Tile_coord t = gwin->get_main_actor()->get_tile();
#if 0
	int lat = (t.tx - 0x3a5)/10,	// +++++(May have these switched.)
	    lng = (t.ty - 0x46e)/10;
#endif
	One_note *note = new One_note(clk->get_day(), clk->get_hour(),
			clk->get_minute(), t.tx, t.ty);
	note->is_new = true;
	notes.push_back(note);
	}

/*
 *	Create notebook gump.
 */

Notebook_gump::Notebook_gump
	(
	) : Gump(0, EXULT_FLX_NOTEBOOK_SHP, SF_EXULT_FLX), curpage(0),
	    curnote(0)
	{
	handles_kbd = true;
	cursor.offset = 0;
					// (Obj. area doesn't matter.)
	set_object_area(Rectangle(36, 10, 100, 100), 7, 40);
	page_info.push_back(Notebook_top(0, 0));
					// Where to paint page marks:
	const int lpagex = 35, rpagex = 300, lrpagey = 12;
	leftpage = new Notebook_page_button(this, lpagex, lrpagey, 0);
 	rightpage = new Notebook_page_button(this, rpagex, lrpagey, 1);
	add_new();			// Add new note to end.
	}

Notebook_gump *Notebook_gump::create
	(
	)
	{
	if (!initialized)
		initialize();
	if (!instance)
		instance = new Notebook_gump;
	return instance;
	}

/*
 *	Cleanup.
 */
Notebook_gump::~Notebook_gump
	(
	)
	{
	if (notes.size())
		{			// Check for empty 'new' note.
		One_note *note = notes.back();
		if (note->is_new && !note->textlen)
			{
			notes.pop_back();
			delete note;
			}
		else
			note->is_new = false;
		}
	delete leftpage;
	delete rightpage;
	if (this == instance)
		instance = 0;
	}

/*
 *	Paint a page and find where its text ends.
 *
 *	Output:	True if finished entire note.
 */

bool Notebook_gump::paint_page
	(
	Rectangle box,			// Display box rel. to gump.
	One_note *note,			// Note to print.
	int& offset,			// Starting offset into text.  Updated.
	int pagenum
	)
{
	bool in_curnote = (note == notes[curnote]);
	if (offset == 0)		// Print note info. at start.
		{
		char buf[60];
		char *ampm = "am";
		int h = note->hour;
		if (h >= 12)
			{
			h -= 12;
			ampm = "pm";
			}
		snprintf(buf, sizeof(buf), "Day %d, %02d:%02d%s",
			note->day, h?h:12, note->minute, ampm);
		sman->paint_text(2, buf, x + box.x, y + pagey);
		gwin->get_win()->fill8(sman->get_special_pixel(CHARMED_PIXEL),
			box.w, 1, x + box.x, y + box.y - 3);
		}
	int textheight = sman->get_text_height(font) + vlead;
	char *str = note->text + offset;
	cursor.offset -= offset;
	int endoff = sman->paint_text_box(font, str, x + box.x,
			y + box.y, box.w, box.h, vlead,
			0, -1, in_curnote? &cursor : 0);
	cursor.offset += offset;
	if (endoff > 0)			// All painted?
		{			// Value returned is height.
		str = note->text + note->textlen;
		}
	else				// Out of room.
		str += -endoff;
	if (in_curnote && cursor.x >= 0)
		{
		gwin->get_win()->fill8(sman->get_special_pixel(POISON_PIXEL), 
			1, 
			sman->get_text_height(font), cursor.x-1, cursor.y-1);
		curpage = pagenum;
		}
	offset = str - note->text;	// Return offset past end.
					// Watch for exactly filling page.
	return (endoff > 0 && endoff < box.h);
}

/*
 *	Change page.
 */

void Notebook_gump::change_page
	(
	int delta
	)
{
	int topleft = curpage & ~1;
	if (delta > 0)
		{
		int nxt = topleft + 2;
		if (nxt >= page_info.size())
			return;
		curpage = nxt;
		curnote = page_info[curpage].notenum;
		cursor.offset = 0;
		}
	else if (delta < 0)
		{
		if (topleft == 0)
			return;
		curpage = topleft - 2;
		curnote = page_info[curpage].notenum;
		cursor.offset = 0;
		}
	paint();
}

/*
 *	Is a given screen point on one of our buttons?  If not, we try to
 *	set cursor pos. within the text.
 *
 *	Output: ->button if so.
 */

Gump_button *Notebook_gump::on_button
	(
	int mx, int my			// Point in window.
	)
{
	Gump_button *btn = Gump::on_button(mx, my);
	if (btn)
		return btn;
	else if (leftpage->on_button(mx, my))
		return leftpage;
	else if (rightpage->on_button(mx, my))
		return rightpage;
	int topleft = curpage & ~1;
	int notenum = page_info[topleft].notenum;
	if (notenum < 0)
		return 0;
	int offset = page_info[topleft].offset;
	Rectangle box = Get_text_area(false, offset == 0);	// Left page.
	One_note *note = notes[notenum];
	int coff = sman->find_cursor(font, note->text + offset, x + box.x,
			y + box.y, box.w, box.h, mx, my, vlead);
	if (coff >= 0)			// Found it?
		{
		curpage = topleft;
		curnote = page_info[curpage].notenum;
		cursor.offset = offset + coff;
		paint();
		}
	else
		{
		offset += -coff;		// New offset.
		if (offset >= note->textlen)
			{
			if (notenum == notes.size() - 1)
				return 0;	// No more.
			note = notes[++notenum];
			offset = 0;
			}
		box = Get_text_area(true, offset == 0);	// Right page.
		box.shift(x, y);		// Window area.
		coff = box.has_point(mx, my) ?
			sman->find_cursor(font, note->text + offset, box.x,
				box.y, box.w, box.h, mx, my, vlead)
			: -1;
		if (coff >= 0)			// Found it?
			{
			curpage = curpage | 1;
			curnote = page_info[curpage].notenum;
			cursor.offset = offset + coff;
			paint();
			}
		}
	return 0;
}

/*
 *	Paint notebook.
 */

void Notebook_gump::paint
	(
	)
{
	Gump::paint();
	if (curpage > 0)		// Not the first?
		leftpage->paint();
	int topleft = curpage & ~1;
	int notenum = page_info[topleft].notenum;
	if (notenum < 0)
		return;
	int offset = page_info[topleft].offset;
	One_note *note = notes[notenum];
					// Paint left page.
	if (paint_page(Get_text_area(false, offset == 0), 
						note, offset, topleft))
		{			// Finished note?
		if (notenum == notes.size() - 1)
			return;
		++notenum;
		note = notes[notenum];
		offset = 0;
		}
	if (topleft + 1 >= page_info.size())	// Store right-page info.
		page_info.resize(topleft + 2);
	page_info[topleft + 1].notenum = notenum;
	page_info[topleft + 1].offset = offset;
					// Paint right page.
	if (paint_page(Get_text_area(true, offset == 0), 
						note, offset, topleft + 1))
		{			// Finished note?
		if (notenum == notes.size() - 1)
			return;		// No more.
		++notenum;
		offset = 0;
		}
	rightpage->paint();
	int nxt = topleft + 2;		// For next pair of pages.
	if (nxt >= page_info.size())
		page_info.resize(nxt + 1);
	page_info[nxt].notenum = notenum;
	page_info[nxt].offset = offset;
}

/*
 *	Handle keystroke.
 */
bool Notebook_gump::handle_kbd_event
	(
	void *vev
	)
	{
	SDL_Event& ev = *(SDL_Event *)vev;
	int chr = ev.key.keysym.sym, unicode = ev.key.keysym.unicode;

	if (ev.type == SDL_KEYUP)
		return true;		// Ignoring key-up at present.
	if (ev.type != SDL_KEYDOWN)
		return false;
	if (curpage >= page_info.size())
		return false;		// Shouldn't happen.
	One_note *note = notes[page_info[curpage].notenum];
	switch (chr) {
	case SDLK_RETURN:
		note->insert('\n', cursor.offset);
		++cursor.offset;
		paint();		// (Not very efficient...)
		break;		
	case SDLK_BACKSPACE:
		if (note->del(cursor.offset - 1))
			{
			--cursor.offset;
			paint();
			}
		break;
	case SDLK_DELETE:
		if (note->del(cursor.offset))
			paint();
		break;
	case SDLK_LEFT:
		if (cursor.offset)
			{
			--cursor.offset;
			paint();
			}
		break;
	case SDLK_RIGHT:
		if (cursor.offset < note->textlen)
			{
			++cursor.offset;
			paint();
			}
		break;
	case SDLK_UP:
	case SDLK_DOWN:
	case SDLK_HOME:
	case SDLK_END:
		// ++++++Finish.
		break;		
	case SDLK_PAGEUP:
		change_page(-1);
		break;
	case SDLK_PAGEDOWN:
		change_page(1);
		break;
	default:
#if 1	/* Assumes unicode is enabled. */
		if ((unicode & 0xFF80) == 0 )
			chr = unicode & 0x7F;
		else
			chr = 0;
#else
		if (ev.key.keysym.mod & KMOD_SHIFT)
			chr = toupper(chr);
#endif
		if (chr < ' ')
			return false;		// Ignore other special chars.
		if (chr >= 256 || !isascii(chr))
			return false;
		note->insert(chr, cursor.offset);
		++cursor.offset;
		paint();		// (Not very efficient...)
		break;		
	}
	// ++++++Finish.
	std::cout << "Notebook chr: " << chr << std::endl;
	return true;
	}

/*
 *	Write it out.
 */

void Notebook_gump::write
	(
	)
	{
	ofstream out;

	if (!initialized)
		return;
	U7open(out, NOTEBOOKXML);
	out << "<notebook>" << endl;
	for (vector<One_note*>::iterator it = notes.begin();
					it != notes.end(); ++it)
		if ((*it)->textlen || !(*it)->is_new)
			(*it)->write(out);
	out << "</notebook>" << endl;
	out.close();
	}

/*
 *	Read it in from 'notebook.xml'.
 */

void Notebook_gump::read
	(
	)
	{
	string root;
	Configuration conf;

	conf.read_abs_config_file(NOTEBOOKXML, root);
	string identstr;
	conf.dump(cout, identstr);

	Configuration::KeyTypeList note_nds;
	string basekey = "notebook";
	conf.getsubkeys(note_nds, basekey);
	One_note *note = 0;
	for (Configuration::KeyTypeList::iterator it = note_nds.begin();
				it != note_nds.end(); ++it)
		{
		Configuration::KeyType notend = *it;
#if 0
		cout << note_pair.first << ": " << endl;
		cout << note_pair.second << endl;
#endif
		if (notend.first == "note")
			{
			note = new One_note();
			notes.push_back(note);
			}
		else if (notend.first == "note/time")
			{
			int d, h, m;
			sscanf(notend.second.c_str(), "%d:%d:%d", &d, &h, &m);
			if (note)
				note->set_time(d, h, m);
			}
		else if (notend.first == "note/place")
			{
			int x, y;
			sscanf(notend.second.c_str(), "%d:%d", &x, &y);
			if (note)
				note->set_loc(x, y);
			}
		else if (notend.first == "note/text")
			{
			if (note)
				note->set_text(
					strdup(notend.second.c_str()));
			}
		}
	}
