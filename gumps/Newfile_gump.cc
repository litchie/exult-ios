/*
Copyright (C) 2000 The Exult Team

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

#ifndef ALPHA_LINUX_CXX
#  include <cstring>
#  include <ctime>
#endif 

#include "SDL_events.h"

#include "Audio.h"
#include "Configuration.h"
#include "Newfile_gump.h"
#include "game.h"
#include "gamewin.h"
#include "Gump_button.h"
#include "mouse.h"
#include "Yesno_gump.h"
#include "ucmachine.h"
#include "actors.h"
#include "listfiles.h"

using std::cout;
using std::endl;
using std::memmove;
using std::string;
using std::strlen;
using std::strncpy;
using std::strcpy;
using std::strcat;
using std::memcpy;
using std::memset;

extern Configuration *config;

/*
 *	Macros:
 */

#define NEWFILE_SHAPE_LOAD	26
#define NEWFILE_SHAPE_SAVE	27
#define NEWFILE_SHAPE_DELETE	28
#define NEWFILE_SHAPE_CANCEL	29
#define NEWFILE_SHAPE_PGDN	30
#define NEWFILE_SHAPE_DOWN	31
#define NEWFILE_SHAPE_UP	32
#define NEWFILE_SHAPE_PGUP	33
#define NEWFILE_SHAPE_SLIDER	34
#define NEWFILE_SHAPE_SELECTED	35

/*
 *	Statics:
 */
// Button Coords
const short Newfile_gump::btn_rows[5] = {196, 2, 15, 158, 171};
const short Newfile_gump::btn_cols[5] = {41, 85, 128, 188, 210};

// Text field info
const short Newfile_gump::fieldx = 2;		// Start Y of each field
const short Newfile_gump::fieldy = 2;		// Start X of first
const short Newfile_gump::fieldw = 207;		// Width of each field
const short Newfile_gump::fieldh = 12;		// Height of each field
const short Newfile_gump::fieldgap = 1;		// Gap between fields
const short Newfile_gump::fieldcount = 14;	// Number of fields
const short Newfile_gump::textx = 12;		// X Offset in field
const short Newfile_gump::texty = 2;		// Y Offset in field
const short Newfile_gump::textw = 190;		// Maximum allowable width of text (pixels)
const short Newfile_gump::iconx = 2;		// X Offset in field
const short Newfile_gump::icony = 2;		// Y Offset in field

// Scrollbar and Slider Info
const short Newfile_gump::scrollx = 212;	// X Offset
const short Newfile_gump::scrolly = 28;		// Y Offset
const short Newfile_gump::scrollh = 129;	// Height of Scroll Bar
const short Newfile_gump::sliderw = 7;		// Width of Slider
const short Newfile_gump::sliderh = 7;		// Height of Slider

const short Newfile_gump::infox = 224;
const short Newfile_gump::infoy = 68;
const short Newfile_gump::infow = 92;
const short Newfile_gump::infoh = 69;
const char Newfile_gump::infostring[] =	"Avatar: %s\n"
					"Exp: %i  Hp: %i\n"
					"Str: %i  Dxt: %i\n"
					"Int: %i  Trn: %i\n"
					"\n"
					"Game Day: %i\n"
					"Game Time: %02i:%02i\n"
					"\n"
					"Save Count: %i\n"
					"Date: %i%s %s %04i\n"
					"Time: %02i:%02i";

const char *Newfile_gump::months[12] = {"Jan",
					"Feb",
					"March",
					"April",
					"May",
					"June",
					"July",
					"Aug",
					"Sept",
					"Oct",
					"Nov",
					"Dec" };

/*
 *	One of our buttons.
 */
class Newfile_button : public Gump_button
{
public:
	Newfile_button(Gump *par, int px, int py, int shapenum)
		: Gump_button(par, shapenum, px, py, GSF_EXULT_FLX)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
};


/*
 *	Clicked a 'load' or 'save' button.
 */

void Newfile_button::activate
	(
	Game_window *gwin
	)
{
	if (shapenum == NEWFILE_SHAPE_LOAD)
		((Newfile_gump *) parent)->load();
	else if (shapenum == NEWFILE_SHAPE_SAVE)
		((Newfile_gump *) parent)->save();
	else if (shapenum == NEWFILE_SHAPE_DELETE)
		((Newfile_gump *) parent)->delete_file();
	else if (shapenum == NEWFILE_SHAPE_CANCEL)
		parent->close(gwin);
	else if (shapenum == NEWFILE_SHAPE_PGDN)
		((Newfile_gump *) parent)->scroll_page(1);
	else if (shapenum == NEWFILE_SHAPE_DOWN)
		((Newfile_gump *) parent)->scroll_line(1);
	else if (shapenum == NEWFILE_SHAPE_UP)
		((Newfile_gump *) parent)->scroll_line(-1);
	else if (shapenum == NEWFILE_SHAPE_PGUP)
		((Newfile_gump *) parent)->scroll_page(-1);
}

/*
 *	Create the load/save box.
 */


Newfile_gump::Newfile_gump
	(
	) : Modal_gump(0, Game_window::get_game_window()->get_width()/2-160,
			Game_window::get_game_window()->get_height()/2-100,
			25, GSF_EXULT_FLX),
		restored(0), games(0), num_games(0), first_free(0),
		cur_shot(0), cur_details(0), cur_party(0),
		gd_shot(0), gd_details(0), gd_party(0),
		screenshot(0), details(0), party(0),
		list_position(-2), selected(-3), cursor(0), slide_start(-1)

{
	newname[0] = 0;

	Game_window *gwin = Game_window::get_game_window();

	back = gwin->get_win()->create_buffer(gwin->get_width(), gwin->get_height());
	gwin->get_win()->get(back, 0, 0);

	// Load/Save/Delete
	buttons[0] = buttons[1] = buttons[2] = 0;

	// Cancel
	buttons[3] = new Newfile_button(this, btn_cols[3], btn_rows[0], NEWFILE_SHAPE_CANCEL);

	// Scrollers.
	buttons[4] = new Newfile_button(this, btn_cols[4], btn_rows[1], NEWFILE_SHAPE_PGUP);
	buttons[5] = new Newfile_button(this, btn_cols[4], btn_rows[2], NEWFILE_SHAPE_UP);
	buttons[6] = new Newfile_button(this, btn_cols[4], btn_rows[3], NEWFILE_SHAPE_DOWN);
	buttons[7] = new Newfile_button(this, btn_cols[4], btn_rows[4], NEWFILE_SHAPE_PGDN);

	LoadSaveGameDetails();
}

/*
 *	Delete the load/save box.
 */

Newfile_gump::~Newfile_gump
	(
	)
{
	size_t i;
	for (i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
		delete buttons[i];

	FreeSaveGameDetails();
	
	Game_window::get_game_window()->get_win()->put(back, 0, 0);
	delete back;
}

/*
 *	'Load' clicked.
 */

void Newfile_gump::load()
{
	// Shouldn't ever happen.
	if (selected == -2 || selected == -3)
		return;	

	Game_window *gwin = Game_window::get_game_window();

	// Aborts if unsuccessful.
	if (selected != -1) gwin->restore_gamedat(games[selected].num);

	// Read Gamedat
	gwin->read();

	// Set Done
	done = 1;
	restored = 1;

	// Reset Selection
	selected = -3;

	delete buttons[0];
	buttons[0] = 0;
	delete buttons[1];
	buttons[1] = 0;
	delete buttons[2];
	buttons[2] = 0;

	//Reread save game details (quick save gets overwritten)
	//FreeSaveGameDetails();
	//LoadSaveGameDetails();
	//paint(Game_window::get_game_window());
	//Game_window::get_game_window()->set_painted();
}

/*
 *	'Save' clicked.
 */

void Newfile_gump::save()
{
	// Shouldn't ever happen.
	if (!strlen(newname) || selected == -3)
		return;	

	Game_window *gwin = Game_window::get_game_window();

	// Already a game in this slot? If so ask to delete
	if (selected != -2) if (!Yesno_gump::ask("Okay to write over existing saved game?"))
		return;

	// Write to gamedat
	gwin->write();

	// Now write to savegame file
	if (selected >= 0) gwin->save_gamedat(games[selected].num, newname);
	else if (selected == -2) gwin->save_gamedat(first_free, newname);

	cout << "Saved game #" << selected << " successfully." << endl;

	// Reset everything
	selected = -3;

	delete buttons[0];
	buttons[0] = 0;
	delete buttons[1];
	buttons[1] = 0;
	delete buttons[2];
	buttons[2] = 0;

	FreeSaveGameDetails();
	LoadSaveGameDetails();
	paint(Game_window::get_game_window());
	Game_window::get_game_window()->set_painted();
}

/*
 *	'Delete' clicked.
 */

void Newfile_gump::delete_file()
{
	// Shouldn't ever happen.
	if (selected == -1 || selected == -2 || selected == -3)
		return;	

	Game_window *gwin = Game_window::get_game_window();

	// Ask to delete
	if (!Yesno_gump::ask("Okay to delete saved game?"))
		return;

	U7remove (games[selected].filename);

	cout << "Deleted Save game #" << selected << " (" << games[selected].filename << ") successfully." << endl;

	// Reset everything
	selected = -3;

	delete buttons[0];
	buttons[0] = 0;
	delete buttons[1];
	buttons[1] = 0;
	delete buttons[2];
	buttons[2] = 0;

	FreeSaveGameDetails();
	LoadSaveGameDetails();
	paint(Game_window::get_game_window());
	Game_window::get_game_window()->set_painted();
}

/*
 *	Scroll Line
 */

void Newfile_gump::scroll_line(int dir)
{
	list_position += dir;

	if (list_position > num_games-fieldcount)
		list_position = num_games-fieldcount;

	if (list_position < -2)
		list_position = -2;

#ifdef DEBUG
	cout << "New list position " << list_position << endl;
#endif

	paint(Game_window::get_game_window());
	Game_window::get_game_window()->set_painted();
}

/*
 *	Scroll Page
 */

void Newfile_gump::scroll_page(int dir)
{
	scroll_line (dir * fieldcount);
}

void Newfile_gump::PaintSaveName (int line)
{
	Game_window *gwin = Game_window::get_game_window();

	int	actual_game = line+list_position;

	if (actual_game < -2 || actual_game >= num_games) return;

	char	*text;

	if (actual_game == -1)
		text = "Quick Save";
	else if (actual_game == -2 && selected != -2)
		text = "Empty Slot";
	else if (actual_game != selected || buttons[0])
		text = games[actual_game].savename;
	else
		text = newname;

	gwin->paint_text (2, text, 
		x + fieldx + textx,
		y + fieldy + texty + line*(fieldh + fieldgap));

	// Being Edited? If so paint cursor
	if (selected == actual_game && cursor != -1)
		gwin->get_win()->fill8(0, 1, gwin->get_text_height(2),
			x + fieldx + textx + gwin->get_text_width(2, text, cursor),
				y + fieldy + texty + line*(fieldh + fieldgap));

	// If selected, show selected icon
	if (selected == actual_game) gwin->paint_exult_shape (
					x+fieldx+iconx,
					y+fieldy+icony+line*(fieldh+fieldgap),
					NEWFILE_SHAPE_SELECTED,
					0);

}


/*
 *	Paint on screen.
 */

void Newfile_gump::paint
	(
	Game_window *gwin
	)
{
	Gump::paint(gwin);

	// Paint text objects.
	int i;

	for (i = 0; i < fieldcount; i++)
		PaintSaveName (i);

	// Paint Buttons
	for (i = 0; i < 8; i++) if (buttons[i])
		paint_button(gwin, buttons[i]);

	// Paint scroller

	// First thing, work out number of positions that the scroller can be in
	int num_pos = (2+num_games)-fieldcount;
	if (num_pos < 1) num_pos = 1;

	// Now work out the position
	int pos = ((scrollh-sliderh)*(list_position+2))/num_pos;

	gwin->paint_exult_shape(x+scrollx , y+scrolly+pos, NEWFILE_SHAPE_SLIDER, 0);

	// Now paint the savegame details
	if (screenshot) gwin->paint_shape(x + 222, y + 2, screenshot->get_frame(0));

	if (details && party)
	{
		int	i;

		for (i=0; i<4 && i<details->party_size; i++)
			gwin->paint_shape(x + 249 + i*23, y + 169, party[i].shape, 16);

		for (i=4; i<8 && i<details->party_size; i++)
			gwin->paint_shape(x + 249 + (i-4)*23, y + 198, party[i].shape, 16);

		char	info[256];

		char	*suffix = "th";

		if ((details->real_day%10) == 1 && details->real_day != 11)
			suffix = "st";
		else if ((details->real_day%10) == 2 && details->real_day != 12)
			suffix = "nd";
		else if ((details->real_day%10) == 3 && details->real_day != 13)
			suffix = "rd";

		snprintf (info, 256, infostring, party[0].name,
			party[0].exp, party[0].health,
			party[0].str, party[0].dext,
			party[0].intel, party[0].training,
			details->game_day, details->game_hour, details->game_minute,
			details->save_count,
			details->real_day, suffix, months[details->real_month-1], details->real_year,
			details->real_hour, details->real_minute);

		gwin->paint_text_box (4, info, x+infox, y+infoy, infow, infoh);
	}
	else
	{
		gwin->paint_text (4, "No Info", x+infox+(infow-gwin->get_text_width(4, "No Info"))/2, y+infoy+(infoh-gwin->get_text_height(4))/2);
	}
	gwin->set_painted();
}

/*
 *	Handle mouse-down events.
 */

void Newfile_gump::mouse_down
	(
	int mx, int my			// Position in window.
	)
{
	slide_start = -1;

	Game_window *gwin = Game_window::get_game_window();
	pushed = Gump::on_button(gwin, mx, my);
				// Try buttons at bottom.
	if (!pushed) for (size_t i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
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

	int gx = mx - x;
	int gy = my - y;

	// Check for scroller
	if (gx >= scrollx && gx < scrollx+sliderw && gy >= scrolly && gy < scrolly+scrollh)
	{
		int num_pos = (2+num_games)-fieldcount;
		if (num_pos < 1) num_pos = 1;

		// Now work out the position
		int pos = ((scrollh-sliderh)*(list_position+2))/num_pos;

		// Pressed above it
		if (gy < pos+scrolly)
		{
			scroll_page(-1);
			paint (gwin);
			return;
		}
		// Pressed below it
		else if (gy >= pos+scrolly+sliderh)
		{
			scroll_page(1);
			paint (gwin);
			return;
		}
		// Pressed on it
		else
		{
			slide_start = gy;
			return;
		}
	}


	// Now check for text fields
	if (gx < fieldx || gx >= fieldx+fieldw)
		return;

	int	hit = -1;
	int	i;
	for (i = 0; i < fieldcount; i++)
	{
		int fy = fieldy + i*(fieldh + fieldgap);
		if (gy >= fy && gy < fy+fieldh)
		{
			hit = i;
			break;
		}
	}

	if (hit == -1) return;

	if (hit+list_position >= num_games || hit+list_position < -2 || selected == hit+list_position) return;

#ifdef DEBUG
	cout << "Hit a save game field" << endl;
#endif
	selected = hit+list_position;

	int	want_load = true;
	int	want_delete = true;
	int	want_save = true;

	if (selected == -2)
	{
		want_load = false;
		want_delete = false;
		want_save = false;
		screenshot = cur_shot;
		details = cur_details;
		party = cur_party;
		newname[0] = 0;
		cursor = 0;
	}
	else if (selected == -1)
	{
		want_delete = false;
		screenshot = gd_shot;
		details = gd_details;
		party = gd_party;
		strcpy (newname, "Quick Save");
		cursor = -1; // No cursor
	}
	else
	{
		screenshot = games[selected].screenshot;
		details = games[selected].details;
		party = games[selected].party;
		strcpy (newname, games[selected].savename);
		cursor = strlen (newname);
	}

	if (!buttons[0] && want_load) buttons[0] = new Newfile_button(this,
							btn_cols[1],
							btn_rows[0],
							NEWFILE_SHAPE_LOAD);
	else if (buttons[0] && !want_load)
	{
		delete buttons[0];
		buttons[0] = 0;
	}

	if (!buttons[1] && want_save) buttons[1] = new Newfile_button(this,
							btn_cols[0],
							btn_rows[0],
							NEWFILE_SHAPE_SAVE);
	else if (buttons[1] && !want_save)
	{
		delete buttons[1];
		buttons[1] = 0;
	}

	if (!buttons[2] && want_delete) buttons[2] = new Newfile_button(this,
							btn_cols[2],
							btn_rows[0],
							NEWFILE_SHAPE_DELETE);
	else if (buttons[2] && !want_delete)
	{
		delete buttons[2];
		buttons[2] = 0;
	}

	paint(gwin);			// Repaint.
	gwin->set_painted();
					// See if on text field.
}

/*
 *	Handle mouse-up events.
 */

void Newfile_gump::mouse_up
	(
	int mx, int my			// Position in window.
	)
{
	slide_start = -1;

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
 *	Mouse was dragged with left button down.
 */

void Newfile_gump::mouse_drag
	(
	int mx, int my			// Where mouse is.
	)
{
	// If not sliding don't do anything
	if (slide_start == -1) return;

	int gx = mx - x;
	int gy = my - y;

	// First if the position is too far away from the slider 
	// We'll put it back to the start
	int sy = gy - scrolly;
	if (gx < scrollx-20 || gx > scrollx+sliderw+20)
		sy = slide_start - scrolly;

	if (sy < sliderh/2) sy = sliderh/2;
	if (sy > scrollh-sliderh/2) sy = scrollh-sliderh/2;
	sy -= sliderh/2;

	// Now work out the number of positions
	int num_pos = (2+num_games)-fieldcount;

	// Can't scroll if there is less than 1 pos
	if (num_pos < 1) return;

	// Now work out the closest position to here position
	int new_pos = ((sy*num_pos*2)/(scrollh-sliderh)+1)/2-2;

	if (new_pos != list_position)
	{
		list_position = new_pos;
		paint(Game_window::get_game_window());
	}
}

/*
 *	Handle character that was typed.
 */

void Newfile_gump::key_down
	(
	int chr
	)
{
	bool update_details = false;
	int repaint = false;

	// Are we selected on some text?
	if (selected == -3)
		return;

	Game_window *gwin = Game_window::get_game_window();

	switch (chr) {

	case SDLK_RETURN:		// If only 'Save', do it.
		if (!buttons[0] && buttons[1])
		{
			buttons[1]->push(gwin);
			gwin->show(1);
			buttons[1]->unpush(gwin);
			gwin->show(1);
			buttons[1]->activate(gwin);
		}
		update_details = true;
		break;

	case SDLK_BACKSPACE:
		if (BackspacePressed())
		{	
			// Can't restore/delete now.
			delete buttons[0];
			delete buttons[2];
			buttons[0] = buttons[2] = 0;

			// If no chars cant save either
			if (!newname[0])
			{	
				delete buttons[1];
				buttons[1] = 0;
			}
			update_details = true;
		}
		break;

	case SDLK_DELETE:
		if (DeletePressed())
		{	
			// Can't restore/delete now.
			delete buttons[0];
			delete buttons[2];
			buttons[0] = buttons[2] = 0;

			// If no chars cant save either
			if (!newname[0])
			{	
				delete buttons[1];
				buttons[1] = 0;
			}
			update_details = true;
		}
		break;

	case SDLK_LEFT:
		repaint = MoveCursor(-1);
		break;

	case SDLK_RIGHT:
		repaint = MoveCursor(1);
		break;

	case SDLK_HOME:
		repaint = MoveCursor(-MAX_SAVEGAME_NAME_LEN);
		break;

	case SDLK_END:
		repaint = MoveCursor(MAX_SAVEGAME_NAME_LEN);
		break;

	default:
		if (chr < ' ')
			return;			// Ignore other special chars.

		if (chr < 256 && isascii(chr))
		{
			if (AddCharacter (chr))
			{
				// Added first character?  Need 'Save' button.
				if (newname[0] && !buttons[1])
				{
					buttons[1] = new Newfile_button(this, btn_cols[0], btn_rows[0], NEWFILE_SHAPE_SAVE);
					paint_button(gwin, buttons[1]);
				}

				// Remove Load and Delete Button
				if (buttons[0] || buttons[2])
				{
					delete buttons[0];
					delete buttons[2];
					buttons[0] = buttons[2] = 0;
				}
				update_details = true;
			}
		}
		break;
	}

	// This sets the game details to the cur set
	if (update_details)
	{
		screenshot = cur_shot;
		details = cur_details;
		party = cur_party;
		repaint = true;
	}
	if (repaint)
	{
		paint (gwin);
		gwin->set_painted();
	}
}

int Newfile_gump::BackspacePressed()
{
	if (cursor == -1 || cursor == 0) return 0;
	cursor--;
	return DeletePressed();
}
int Newfile_gump::DeletePressed()
{
	if (cursor == -1 || cursor == strlen (newname)) return 0;
	for (int i = cursor; i < strlen (newname); i++)
		newname[i] = newname[i+1];

	return 1;
}
int Newfile_gump::MoveCursor(int count)
{
	if (cursor == -1) return 0;

	cursor += count;
	if (cursor < 0) cursor = 0;
	if (cursor > strlen (newname)) cursor = strlen (newname);

	return 1;
}
int Newfile_gump::AddCharacter(char c)
{
	if (cursor == -1 || cursor == MAX_SAVEGAME_NAME_LEN-1) return 0;

	char	text[MAX_SAVEGAME_NAME_LEN];

	strcpy (text, newname);
	text[cursor+1] = 0;
	text[cursor] = c;
	strcat (text, newname+cursor);

	//Now check the width of the text
	if (Game_window::get_game_window()->get_text_width(2, text) >= textw)
		return 0;

	cursor++;
	strcpy (newname, text);
	return 1;
}

void Newfile_gump::LoadSaveGameDetails()
{
	Game_window *gwin = Game_window::get_game_window();

	// Gamedat Details
	gwin->get_saveinfo(gd_shot, gd_details, gd_party);

	// Current screenshot
	cur_shot = gwin->create_mini_screenshot();

	// Current Details
	cur_details = new SaveGame_Details;
	memset (cur_details, 0, sizeof(SaveGame_Details));

	gwin->get_win()->put(back, 0, 0);

	Usecode_machine *uc = gwin->get_usecode();

	if (gd_details) cur_details->save_count = gd_details->save_count;
	else cur_details->save_count = 0;

	cur_details->party_size = uc->get_party_count()+1;
	cur_details->game_day = gwin->get_total_hours() / 24;;
	cur_details->game_hour = gwin->get_hour();
	cur_details->game_minute = gwin->get_minute();
	
	time_t t = std::time(0);
	struct tm *timeinfo = std::localtime (&t);	

	cur_details->real_day = timeinfo->tm_mday;
	cur_details->real_hour = timeinfo->tm_hour;
	cur_details->real_minute = timeinfo->tm_min;
	cur_details->real_month = timeinfo->tm_mon+1;
	cur_details->real_year = timeinfo->tm_year + 1900;
	cur_details->real_second = timeinfo->tm_sec;

	// Current Party
	cur_party = new SaveGame_Party[cur_details->party_size];
	for (int i=0; i<cur_details->party_size ; i++)
	{
		Actor *npc;
		if (i == 0)
			npc = gwin->get_main_actor();
		else
			npc = (Npc_actor *) gwin->get_npc( uc->get_party_member(i-1));

		strncpy (cur_party[i].name, npc->get_npc_name().c_str(), 18);
		cur_party[i].shape = npc->get_shapenum();

		cur_party[i].dext = npc->get_property(Actor::dexterity);
		cur_party[i].str = npc->get_property(Actor::strength);
		cur_party[i].intel = npc->get_property(Actor::intelligence);
		cur_party[i].health = npc->get_property(Actor::health);
		cur_party[i].combat = npc->get_property(Actor::combat);
		cur_party[i].mana = npc->get_property(Actor::mana);
		cur_party[i].magic = npc->get_property(Actor::magic);
		cur_party[i].training = npc->get_property(Actor::training);
		cur_party[i].exp = npc->get_property(Actor::exp);
		cur_party[i].food = npc->get_property(Actor::food_level);
		cur_party[i].flags = npc->get_flags();
		cur_party[i].flags2 = npc->get_flags2();
	}

	party = cur_party;
	screenshot = cur_shot;
	details = cur_details;

	// Now read save game details
	char	**filenames = 0;
	char	mask[256];
	int	i;

	snprintf(mask, 256, SAVENAME2, Game::get_game_type() == BLACK_GATE ? "bg" : "si");
	U7ListFiles (mask, filenames, num_games);

	games = new SaveInfo[num_games];

	// Setup basic details
	for (i = 0; i<num_games; i++)
	{
		games[i].filename = new char[strlen(filenames[i])+1];
		strcpy (games[i].filename, filenames[i]);
		games[i].SetSeqNumber();
	}

	// Delete the filenames array cause it's not needed any more
	U7FreeFileList(filenames, num_games);

	// First sort thet games so the will be sorted by number
	// This is so I can work out the first free game
	if (num_games) qsort(games, num_games, sizeof(SaveInfo), SaveInfo::CompareGames);

	// Reand and cache all details
	first_free = -1;
	for (i = 0; i<num_games; i++)
	{
		gwin->get_saveinfo(games[i].num, games[i].savename, games[i].screenshot,
			games[i].details, games[i].party);

		if (first_free == -1 && i != games[i].num) first_free = i;
	}

	if (first_free == -1) first_free = num_games;

	// Now sort it again, with all the details so it can be done by date
	if (num_games) qsort(games, num_games, sizeof(SaveInfo), SaveInfo::CompareGames);



	// We'll now output the info if debugging
#ifdef DEBUG
	cout << "Listing " << num_games << " Save games" << endl;
	for (i = 0; i<num_games; i++)
		cout << i << " = " << games[i].num << " : " << games[i].filename << " : " << games[i].savename << endl;

	cout << "First Free Game " << first_free << endl;
#endif
}

void Newfile_gump::FreeSaveGameDetails()
{
	delete cur_shot;
	cur_shot = 0;
	delete cur_details;
	cur_details = 0;
	delete [] cur_party;
	cur_party = 0;

	delete gd_shot;
	gd_shot = 0;
	delete gd_details;
	gd_details = 0;
	delete [] gd_party;
	gd_party = 0;

	// The SaveInfo struct will delete everything that it's got allocated
	// So we don't need to worry about that
	delete [] games;
	games = 0;
}

// Save Info Methods

// Constructor
Newfile_gump::SaveInfo::SaveInfo() : num(0), filename(0), savename(0), details(0),
			party(0), screenshot(0)
{

}

// Destructor
Newfile_gump::SaveInfo::~SaveInfo()
{
	delete [] filename;
	delete [] savename;
	delete details;
	delete [] party;
	delete screenshot;
}

// Set Sequence Number
void Newfile_gump::SaveInfo::SetSeqNumber()
{
        int i;

        for (i = strlen(filename) - 1; !isdigit(filename[i]); i--);
        for (; isdigit(filename[i]); i--);

	num = atoi(filename+i+1);
}

// Compare This
int Newfile_gump::SaveInfo::CompareThis(const SaveInfo *other) const
{
	// Check by time first, if possible
	if (details && other->details)
	{
		if (details->real_year < other->details->real_year)
			return 1;
		if (details->real_year > other->details->real_year)
			return -1;

		if (details->real_month < other->details->real_month)
			return 1;
		if (details->real_month > other->details->real_month)
			return -1;

		if (details->real_day < other->details->real_day)
			return 1;
		if (details->real_day > other->details->real_day)
			return -1;

		if (details->real_hour < other->details->real_hour)
			return 1;
		if (details->real_hour > other->details->real_hour)
			return -1;

		if (details->real_minute < other->details->real_minute)
			return 1;
		if (details->real_minute > other->details->real_minute)
			return -1;

		if (details->real_second < other->details->real_second)
			return 1;
		if (details->real_second > other->details->real_second)
			return -1;
	}
	else if (details)	// If the other doesn't have time we are first
	{
		return -1;
	}
	else if (other->details)	// If we don't have time we are last
	{
		return 1;
	}
	
	return num - other->num;
}

static int _U7SaveSeqNr(const char *a)
{
        int i;


        for (i = strlen((char*)a) - 1; !isdigit(((char*)a)[i]); i--);
        for (; isdigit(((char*)a)[i]); i--);


        return atoi(&a[i+1]);
}

// Compare Games Static
int Newfile_gump::SaveInfo::CompareGames(const void *a, const void *b)
{
	return ((Newfile_gump::SaveInfo*)a)->CompareThis((Newfile_gump::SaveInfo*)b);
}
