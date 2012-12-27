/*
Copyright (C) 2000-2012 The Exult Team

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
#include "msgfile.h"
#include "fnames.h"
#include "cheat.h"
#include "U7file.h"

using std::ofstream;
using std::ifstream;
using std::ostream;
using std::endl;
using std::string;
using std::cout;

vector<One_note *> Notebook_gump::notes;
bool Notebook_gump::initialized = false;	// Set when read in.
bool Notebook_gump::initialized_auto_text = false;
Notebook_gump *Notebook_gump::instance = 0;
vector<Notebook_top> Notebook_gump::page_info;
vector<char *> Notebook_gump::auto_text;

/*
 *	Defines in 'gumps.vga':
 */
#define LEFTPAGE  (GAME_BG ? 44 : 39)	// At top-left of left page.
#define RIGHTPAGE  (GAME_BG ? 45 : 40)	// At top-right of right page.

const int font = 4;			// Small black.
const int vlead = 1;			// Extra inter-line spacing.
const int pagey = 10;			// Top of text area of page.
const int lpagex = 36, rpagex = 174;	// X-coord. of text area of page.

class One_note
{
	int day, hour, minute;		// Game time when note was written.
	int tx, ty;			// Tile coord. where written.
	char *text;			// Text, 0-delimited.
	int textlen;			// Length, not counting ending NULL.
	int textmax;			// Max. space.
	int gflag;			// >=0 if created automatically when
					//   the global flag was set.
	bool is_new;			// Newly created at cur. time/place.
public:
	friend class Notebook_gump;
	One_note() : day(0), hour(0), minute(0), tx(0), ty(0), 
				text(0), textlen(0), textmax(0), gflag(-1)
		{  }
	void set_time(int d, int h, int m)
		{ day = d; hour = h; minute = m; }
	void set_loc(int x, int y)
		{ tx = x; ty = y; }
	void set_text(char *txt);
	void set_gflag(int gf)
		{ gflag = gf; }
	void set(int d, int h, int m, int x, int y, char *txt = 0, int gf = -1)
		{
		set_time(d, h,m);
		set_loc(x, y);
		set_text(txt);
		gflag = gf;
		}
	One_note(int d, int h, int m, int x, int y, char *txt = 0, int gf = -1)
		: text(0)
		{ set(d, h, m, x, y, txt, gf); }
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
	delete [] text;
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
	if (gflag >= 0)
		out << "<gflag> " << gflag << " </gflag>" << endl;
	out << "<text>" << endl;
	// Encode entities (prevents crashes on load with ampersands).
	string txtenc = encode_entity(text);
	out.write(txtenc.c_str(), txtenc.size());
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
		return right ? Rectangle(rpagex, pagey, 122, 130)
			     : Rectangle(lpagex, pagey, 122, 130);
	else
		return right ? Rectangle(rpagex, pagey+ninf, 122, 130-ninf)
			     : Rectangle(lpagex, pagey+ninf, 122, 130-ninf);
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
	virtual bool activate(int button=1);
	virtual void push() {}
	virtual void unpush() {}
	};

/*
 *	Handle click.
 */

bool Notebook_page_button::activate
	(
	int button
	)
	{
	if (button != 1) return false;
	((Notebook_gump *) parent)->change_page(leftright ? 1 : -1);
	return true;
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
				newstrdup("Note  #1\nHello")));
	notes.push_back(new One_note(2, 2,20, 20, 20, newstrdup(
				"Note  #2\nworld.\n\nHow are you?")));
	notes.push_back(new One_note(3, 3,30, 30, 30, newstrdup(
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
	page_info.clear();
	initialized = false;
	}


/*
 *	Add a new note at the current time/place.
 */

void Notebook_gump::add_new
	(
	char *text,
	int gflag
	)
	{
	Game_clock *clk = gwin->get_clock();
	Tile_coord t = gwin->get_main_actor()->get_tile();
#if 0
	int lat = (t.tx - 0x3a5)/10,	// +++++(May have these switched.)
	    lng = (t.ty - 0x46e)/10;
#endif
	One_note *note = new One_note(clk->get_day(), clk->get_hour(),
			clk->get_minute(), t.tx, t.ty, text, gflag);
	note->is_new = true;
	notes.push_back(note);
	}

/*
 *	Create notebook gump.
 */

Notebook_gump::Notebook_gump
	(
	) : Gump(0, EXULT_FLX_NOTEBOOK_SHP, SF_EXULT_FLX), curnote(0),
	    curpage(0), updnx(0)
	{
	handles_kbd = true;
	cursor.offset = 0;
					// (Obj. area doesn't matter.)
	set_object_area(Rectangle(36, 10, 100, 100), 7, 40);
	if (page_info.empty())
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
 *		Cursor.x is possibly set.
 */

bool Notebook_gump::paint_page
	(
	Rectangle const& box,			// Display box rel. to gump.
	One_note *note,			// Note to print.
	int& offset,			// Starting offset into text.  Updated.
	int pagenum
	)
{
	bool find_cursor = (note == notes[curnote] && cursor.x < 0);
	if (offset == 0)		// Print note info. at start.
		{
		char buf[60];
		const char *ampm = "am";
		int h = note->hour;
		if (h >= 12)
			{
			h -= 12;
			ampm = "pm";
			}
		snprintf(buf, sizeof(buf), "Day %d, %02d:%02d%s",
			note->day, h?h:12, note->minute, ampm);
		sman->paint_text(2, buf, x + box.x, y + pagey);
		//when cheating show location of entry (in dec - could use sextant postions)
		if (cheat())
			{
			snprintf(buf, sizeof(buf), "%d, %d",
			note->tx, note->ty);
			sman->paint_text(4, buf, x + box.x + 80, y + pagey - 4);
			}
		// Use bright green for automatic text.
		gwin->get_win()->fill8(sman->get_special_pixel(
			note->gflag >= 0 ? POISON_PIXEL : CHARMED_PIXEL),
			box.w, 1, x + box.x, y + box.y - 3);
		}
	char *str = note->text + offset;
	cursor.offset -= offset;
	int endoff = sman->paint_text_box(font, str, x + box.x,
			y + box.y, box.w, box.h, vlead,
			false, false, -1, find_cursor? &cursor : 0);
	cursor.offset += offset;
	if (endoff > 0)			// All painted?
		{			// Value returned is height.
		str = note->text + note->textlen;
		}
	else				// Out of room.
		str += -endoff;
	if (find_cursor && cursor.x >= 0)
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
		updnx = cursor.offset = 0;
		}
	else if (delta < 0)
		{
		if (topleft == 0)
			return;
		curpage = topleft - 2;
		curnote = page_info[curpage].notenum;
		updnx = cursor.offset = 0;
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
		updnx = cursor.x - x - lpagex;
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
			updnx = cursor.x - x - rpagex;
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
	cursor.x = -1;
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
 *	Move to end of prev. page.
 */

void Notebook_gump::prev_page
	(
	)
	{
	if (!curpage)
		return;
	Notebook_top &pinfo = page_info[curpage];
	--curpage;
	curnote = page_info[curpage].notenum;
	if (!pinfo.offset)		// Going to new note?
		cursor.offset = notes[curnote]->textlen;
	else
		cursor.offset = pinfo.offset - 1;
	}

/*
 *	Move to next page.
 */

void Notebook_gump::next_page
	(
	)
	{
	if (curpage >= page_info.size())
		return;
	++curpage;
	Notebook_top &pinfo = page_info[curpage];
	curnote = pinfo.notenum;
	cursor.offset = pinfo.offset;	// Start of page.
	}

/*
 *	See if on last/first line of current page.
 *	Note:	These assume paint() was done, so cursor is correct.
 */

bool Notebook_gump::on_last_page_line
	(
	)
	{
	return cursor.line == cursor.nlines - 1;
	}

bool Notebook_gump::on_first_page_line
	(
	)
	{
	return cursor.line == 0;
	}

/*
 *	Handle down/up arrows.
 */

void Notebook_gump::down_arrow
	(
	)
	{
	int offset = page_info[curpage].offset;
	Rectangle box = Get_text_area((curpage%2) != 0, offset == 0);
	int ht = sman->get_text_height(font);
	if (on_last_page_line())
		{
		if (curpage >= page_info.size() - 1)
			return;
		next_page();
		paint();
		offset = page_info[curpage].offset;
		box = Get_text_area((curpage%2)!= 0, offset == 0);
		cursor.y = y + box.y - ht;
		}
	box.shift(x, y);		// Window coords.
	int mx = box.x + updnx + 1, my = cursor.y + ht + ht/2;
	int notenum = page_info[curpage].notenum;
	One_note *note = notes[notenum];
	int coff = sman->find_cursor(font, note->text + offset, box.x,
				box.y, box.w, box.h, mx, my, vlead);
	if (coff >= 0)			// Found it?
		{
		cursor.offset = offset + coff;
		paint();
		}
	}

void Notebook_gump::up_arrow
	(
	)
	{
	Notebook_top &pinfo = page_info[curpage];
	int ht = sman->get_text_height(font);
	int offset = pinfo.offset;
	int notenum = pinfo.notenum;
	if (on_first_page_line())	// Above top.
		{
		if (!curpage)
			return;
		prev_page();
		Notebook_top &pinfo2 = page_info[curpage];
		notenum = pinfo2.notenum;
		if (pinfo.notenum == notenum)	// Same note?
			cursor.offset = offset - 1;
		else
			cursor.offset = notes[notenum]->textlen;
		paint();
		offset = pinfo2.offset;
		cursor.y += ht/2;		// Past bottom line.
		}
	Rectangle box = Get_text_area((curpage%2)!= 0, offset == 0);
	box.shift(x, y);		// Window coords.
	int mx = box.x + updnx + 1, my = cursor.y - ht/2;
	One_note *note = notes[notenum];
	int coff = sman->find_cursor(font, note->text + offset, box.x,
				box.y, box.w, box.h, mx, my, vlead);
	if (coff >= 0)			// Found it?
		{
		cursor.offset = offset + coff;
		paint();
		}
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
	Notebook_top& pinfo = page_info[curpage];
	One_note *note = notes[pinfo.notenum];
	switch (chr) {
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
		note->insert('\n', cursor.offset);
		++cursor.offset;
		paint();		// (Not very efficient...)
		if (need_next_page())
			{
			next_page();
			paint();
			}
		break;		
	case SDLK_BACKSPACE:
		if (note->del(cursor.offset - 1))
			{
			if (--cursor.offset < pinfo.offset && curpage%2 == 0)
				prev_page();
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
			if (--cursor.offset < pinfo.offset && curpage%2 == 0)
				prev_page();
			paint();
			}
		break;
	case SDLK_RIGHT:
		if (cursor.offset < note->textlen)
			{
			++cursor.offset;
			paint();
			if (need_next_page())
				{
				next_page();
				paint();
				}
			}
		break;
	case SDLK_UP:
		up_arrow();
		return true;			// Don't set updnx.
	case SDLK_DOWN:
		down_arrow();
		return true;			// Don't set updnx.
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
		if (need_next_page())
			{
			next_page();
			paint();
			}
		break;		
	}
	updnx = cursor.x - x - (curpage%2 ? rpagex : lpagex);
#if DEBUG
	std::cout << "updnx = " << updnx << std::endl;
//	std::cout << "Notebook chr: " << chr << std::endl;
#endif
	return true;
	}

/*
 *	Automatically add a text entry when a usecode global flag is set.
 */

void Notebook_gump::add_gflag_text
	(
	int gflag,
	char *text
	)
	{
	if (!initialized)
		initialize();
	// See if already there.
	for (vector<One_note*>::iterator it = notes.begin();
					it != notes.end(); ++it)
		if ((*it)->gflag == gflag)
			return;
	if (gwin->get_allow_autonotes())
	add_new(newstrdup(text), gflag);
	}

/*
 *	Write it out.
 */

void Notebook_gump::write
	(
	)
	{
	ofstream out;

	U7open(out, NOTEBOOKXML);
	out << "<notebook>" << endl;
	if (initialized)
		{
		for (vector<One_note*>::iterator it = notes.begin();
					it != notes.end(); ++it)
			if ((*it)->textlen || !(*it)->is_new)
				(*it)->write(out);
		}
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
	// not spamming the terminal with all the notes in normal play
#if DEBUG
	conf.dump(cout, identstr);
#endif
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
					newstrdup(notend.second.c_str()));
			}
		else if (notend.first == "note/gflag")
			{
			int gf;
			sscanf(notend.second.c_str(), "%d", &gf);
			if (note)
				note->set_gflag(gf);
			}
		}
	}

/*
 *	Read in text to be inserted automatically when global flags are set.
 */

// read in from external file
void Notebook_gump::read_auto_text_file(const char* filename)
{
	if (gwin->get_allow_autonotes())
		{
		cout << "Loading autonotes from file " << filename << endl;
		initialized_auto_text = true;
		ifstream notesfile;
		U7open(notesfile, filename, true);
		Read_text_msg_file(notesfile, auto_text);
		notesfile.close();
		}
}

// read in from flx bundled file
void Notebook_gump::read_auto_text
	(
	)
	{
	if (gwin->get_allow_autonotes())
		{
		//cout << "Loading default autonotes" << endl;
		initialized_auto_text = true;
		const str_int_pair& resource = game->get_resource("config/autonotes");
		U7object txtobj(resource.str, resource.num);
		size_t len;
		char *txt = txtobj.retrieve(len);
		if (txt && len > 0)
			{
			BufferDataSource buf(txt,len);
			Read_text_msg_file(&buf, auto_text);
			}
		delete[] txt;
		}
	}
