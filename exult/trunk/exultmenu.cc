/*
 *  Copyright (C) 2000-2011  The Exult Team
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

#include "menulist.h"
#include "exultmenu.h"
#include "exult.h"
#include "mouse.h"
#include "gamewin.h"
#include "ibuf8.h"
#include "game.h"
#include "bggame.h"
#include "sigame.h"
#include "Audio.h"
#include "font.h"
#include "cheat.h"
#include "Configuration.h"
#include "txtscroll.h"
#include "data/exult_flx.h"
#include "palette.h"
#include "shapeid.h"
#include "XMidiFile.h"
#include "databuf.h"
#include "fnames.h"
#include "gamemgr/modmgr.h"
#include "gumps/Gamemenu_gump.h"
#include "shapeid.h"

static bool get_play_intro(void);
static void set_play_intro(bool);
static bool get_play_1st_scene(void);
static void set_play_1st_scene(bool);

#define MAX_GAMES 100

static inline bool handle_menu_click
	(
	int id,
	int& first,
	int last_page,
	int pagesize
	)
{
	switch (id)
	{
		case -8:
			first = 0;
			return true;
		case -7:
			first -= pagesize;
			return true;
		case -6:
			first += pagesize;
			return true;
		case -5:
			first = last_page;
			return true;
		default:
			return false;
	}
}

int maximum_size(Font *font, const char *options[], int num_choices, int centerx)
{
	int max_width = 0, width;
	for(int i=0; i<num_choices; i++) {
		width = font->get_text_width(options[i]);
		if (width > max_width)
			max_width = width;
	}
	max_width += 16;
	return max_width;
}

void create_scroller_menu(MenuList *menu, Font *fonton, Font *font, int first, int pagesize, int num_choices, int xpos, int ypos)
{
	const char *menuscroller[] = {
		"FIRST",
		"PREVIOUS",
		"NEXT",
		"LAST"
	};
	int ncount = sizeof(menuscroller)/sizeof(menuscroller[0]);
	assert(ncount==4);
	int max_width = maximum_size(font, menuscroller, ncount, xpos);
	xpos = xpos - max_width*3/2;
	
	num_choices--;
	int lastpage = num_choices - num_choices%pagesize;

	for(int i=0; i<ncount; i++) {
		//Check to see if this entry is needed at all:
		if (!((i<2 && first == 0) ||
			(i==0 && first == pagesize) ||
			(i>=2 && lastpage == first) ||
			(i==3 && lastpage == first+pagesize)))
		{
			MenuTextEntry *entry = new MenuTextEntry(fonton, font, menuscroller[i],
								xpos, ypos);
			//These commands have negative ids:
			entry->set_id(i-8);
			menu->add_entry(entry);
		}
		xpos += max_width;
	}

}

ExultMenu::ExultMenu(Game_window *gw)
	: font(0), fonton(0), navfont(0), navfonton(0)
{
	gwin = gw;
	ibuf = gwin->get_win()->get_ib8();
	const char *fname = BUNDLE_CHECK(BUNDLE_EXULT_FLX, EXULT_FLX);
	fontManager.add_font("CREDITS_FONT", fname, EXULT_FLX_FONT_SHP, 1);
	fontManager.add_font("HOT_FONT", fname, EXULT_FLX_FONTON_SHP, 1);
	fontManager.add_font("NAV_FONT", fname, EXULT_FLX_NAVFONT_SHP, 1);
	fontManager.add_font("HOT_NAV_FONT", fname, EXULT_FLX_NAVFONTON_SHP, 1);
	calc_win();
	exult_flx.load(fname);
}

ExultMenu::~ExultMenu()
{
}

void ExultMenu::calc_win()
{
	centerx = gwin->get_width()/2;
	centery = gwin->get_height()/2;
	Font *fnt = font ? font : fontManager.get_font("CREDITS_FONT");
	pagesize = 2 * ((gwin->get_win()->get_full_height() - 5 * fnt->get_text_height() - 15) / 45);
}


void ExultMenu::setup()
{
	ModManager*mm = gamemanager->get_bg();
	if (!mm) gamemanager->get_si();
	if (!mm) gamemanager->get_game(0);
	if (!mm) {
		std::cerr << "No games found. Unable to show gumps in Exult menu." << std::endl;
		return;
	}
	//ModManager mm_exult_menu_game (*mm);
	//mm_exult_menu_game.set_game_type(EXULT_MENU_GAME);

	Game *exult_menu_game = Game::create_game(mm);
	
	if (!Shape_manager::get_instance()->load_gumps_minimal())
	{
		std::cerr << "Unable to show gumps in Exult menu." << std::endl;
		return;
	}

	Mouse::mouse = menu_mouse;

	gwin->clear_screen(true);

	Palette *gpal = gwin->get_pal();
	gpal->fade(0,1,0);

	gwin->set_in_exult_menu(true);
	Gamemenu_gump::do_exult_menu();
	gwin->set_in_exult_menu(false);

	Mouse::mouse = 0;
	delete exult_menu_game; 
	game = exult_menu_game = 0;

	gwin->clear_screen(true);
	gpal->load(BUNDLE_CHECK(BUNDLE_EXULT_FLX, EXULT_FLX), EXULT_FLX_EXULT0_PAL);
	gpal->apply();

#if 0
	Palette *gpal = gwin->get_pal();
	Font *font = fontManager.get_font("CREDITS_FONT");
	Font *fonton = fontManager.get_font("HOT_FONT");
	MenuList menu;
#ifdef HAVE_OPENGL
	Shape_frame *setupbg = 0;
	if (GL_manager::get_instance())
	{
		int w = gwin->get_win()->get_full_width(), h = gwin->get_win()->get_full_height();
		Image_buffer8 *buf = dynamic_cast<Image_buffer8 *>
				(gwin->get_win()->get_ib8()->create_another(w, h));
		assert(buf);
		buf->fill8(0);
		setupbg = new Shape_frame(buf->get_bits(), w, h, 0, 0, true);
		delete buf;
	}
	menu.set_background(setupbg);
#endif

	int menuypos = centery-44;
	
	MenuTextChoice *palfades = new MenuTextChoice(fonton, font, "PALETTE FADES",
					centerx, menuypos);
	palfades->add_choice("Off");
	palfades->add_choice("On");
	palfades->set_choice(gwin->get_pal()->get_fades_enabled()?1:0);
	menu.add_entry(palfades);
	menuypos+=11;

	MenuTextChoice *playintro = new MenuTextChoice(fonton, font, "PLAY INTRODUCTION",
					centerx, menuypos);
	playintro->add_choice("Off");
	playintro->add_choice("On");
	playintro->set_choice(get_play_intro()?1:0);
	menu.add_entry(playintro);
	menuypos+=11;

	
	MenuTextChoice *playscene = new MenuTextChoice(fonton, font, "PLAY FIRST SCENE",
					centerx, menuypos);
	playscene->add_choice("Off");
	playscene->add_choice("On");
	playscene->set_choice(get_play_1st_scene()?1:0);
	menu.add_entry(playscene);
	menuypos+=11;

	MenuTextChoice *cheating = new MenuTextChoice(fonton, font, "CHEATING",
					centerx, menuypos);
	cheating->add_choice("Off");
	cheating->add_choice("On");
	cheating->set_choice(cheat()?1:0);
	menu.add_entry(cheating);
	menuypos+=11;

	MenuTextEntry *ok = new MenuTextEntry(fonton, font, "OK",
					centerx-64, centery+55);
	int ok_button = menu.add_entry(ok);
	
	MenuTextEntry *cancel = new MenuTextEntry(fonton, font, "CANCEL",
					centerx+64, centery+55);
	int cancel_button = menu.add_entry(cancel);
	
	menu.set_selection(0);
	gwin->clear_screen(true);
	while(1)
		{
		gpal->apply();
		int entry = menu.handle_events(gwin,menu_mouse);
		if(entry==ok_button)
			{
			gpal->fade_out(c_fade_out_time);
			gwin->clear_screen(true);
			// Scaling Method			
			// Palette fades
			gpal->set_fades_enabled(palfades->get_choice()==1);
			config->set("config/video/disable_fades",
					gpal->get_fades_enabled()?"no":"yes", true);

			// Play Intro
			set_play_intro(playintro->get_choice()==1);
			// Play 1st scene
			set_play_1st_scene(playscene->get_choice()==1);
			// Cheating
			cheat.set_enabled(cheating->get_choice()==1);
			calc_win();
			break;
			}
		else if (entry==cancel_button)
			{
			gpal->fade_out(c_fade_out_time);
			gwin->clear_screen(true);
			break;
			}
		}
#ifdef HAVE_OPENGL
	delete setupbg;
#endif
#endif
}

MenuList *ExultMenu::create_main_menu(Shape_frame *bg, int first)
{
	MenuList *menu = new MenuList();

	int ypos = 15 + gwin->get_win()->get_start_y();
	int xpos = (gwin->get_win()->get_full_width()/2+exult_flx.get_shape(EXULT_FLX_SFX_ICON_SHP,0)->get_width())/2;
	std::vector<ModManager>& game_list = gamemanager->get_game_list();
	int num_choices = game_list.size();
	int last = num_choices > first + pagesize ? first + pagesize : num_choices;
	for(int i=first; i<last; i++) {
		int menux = xpos+(i%2)*gwin->get_win()->get_full_width()/2 + gwin->get_win()->get_start_x();
		ModManager& exultgame = game_list[i];
		bool have_sfx = Audio::have_config_sfx(exultgame.get_cfgname()) ||
				Audio::have_roland_sfx(exultgame.get_game_type()) ||
				Audio::have_sblaster_sfx(exultgame.get_game_type()) ||
				Audio::have_midi_sfx();

		Shape_frame *sfxicon = exult_flx.get_shape(EXULT_FLX_SFX_ICON_SHP,
				have_sfx ? 1 : 0);
		MenuGameEntry *entry = new MenuGameEntry(fonton, font,
							exultgame.get_menu_string().c_str(),
							sfxicon, menux, ypos);
		entry->set_id(i);
		menu->add_entry(entry);
		if (exultgame.has_mods())
		{
			MenuTextEntry *mod_entry = new MenuTextEntry(navfonton, navfont, "SHOW MODS",
								menux, ypos+entry->get_height()+4);
			mod_entry->set_id(i+MAX_GAMES);
			menu->add_entry(mod_entry);
		}
		if (i%2)
			ypos += 45;
	}

	create_scroller_menu(menu, navfonton, navfont, first, pagesize, num_choices,
			centerx, ypos = gwin->get_win()->get_end_y()-5*font->get_text_height());

	const char *menuchoices[] = { 
		"SETUP",
		"CREDITS",
		"QUOTES",
		"EXIT"
	};
	int num_entries = sizeof(menuchoices)/sizeof(menuchoices[0]);
	int max_width = maximum_size(font, menuchoices, num_entries, centerx);
	xpos = centerx - max_width*(num_entries-1)/2;
	ypos = gwin->get_win()->get_end_y()-3*font->get_text_height();
	for(int i=0; i<4; i++) {
		MenuTextEntry *entry = new MenuTextEntry(fonton, font, menuchoices[i],
							xpos, ypos);
		//These commands have negative ids:
		entry->set_id(i-4);
		menu->add_entry(entry);
		xpos += max_width;
	}

	menu->set_background(bg);
	return menu;
}

MenuList *ExultMenu::create_mods_menu(ModManager *selgame, Shape_frame *bg, int first)
{
	MenuList *menu = new MenuList();

	int ypos = 15 + gwin->get_win()->get_start_y();
	int xpos = gwin->get_win()->get_full_width()/4;
	
	std::vector<ModInfo>& mod_list = selgame->get_mod_list();
	int num_choices = mod_list.size();
	int last = num_choices > first + pagesize ? first + pagesize : num_choices;
	for(int i=first; i<last; i++) {
		int menux = xpos+(i%2)*gwin->get_win()->get_full_width()/2 + gwin->get_win()->get_start_x();
		ModInfo& exultmod = mod_list[i];
		MenuGameEntry *entry = new MenuGameEntry(fonton, font,
							exultmod.get_menu_string().c_str(),
							0, menux, ypos);
		entry->set_id(i);
		entry->set_enabled(exultmod.is_mod_compatible());
		menu->add_entry(entry);

		if (!exultmod.is_mod_compatible())
		{
			MenuGameEntry *incentry = new MenuGameEntry(navfonton, navfont, "WRONG EXULT VERSION",
								0, menux, ypos+entry->get_height()+4);
			// Accept no clicks:
			incentry->set_enabled(false);
			menu->add_entry(incentry);
		}
		if (i%2)
			ypos += 45;
	}
	
	create_scroller_menu(menu, navfonton, navfont, first, pagesize, num_choices,
			centerx, ypos = gwin->get_win()->get_end_y()-5*font->get_text_height());

	const char *menuchoices[] = { 
		"RETURN TO MAIN MENU"
	};
	int num_entries = sizeof(menuchoices)/sizeof(menuchoices[0]);
	int max_width = maximum_size(font, menuchoices, num_entries, centerx);
	xpos = centerx - max_width*(num_entries-1)/2;
	ypos = gwin->get_win()->get_end_y()-3*font->get_text_height();
	for(int i=0; i<num_entries; i++) {
		MenuTextEntry *entry = new MenuTextEntry(fonton, font, menuchoices[i],
							xpos, ypos);
		//These commands have negative ids:
		entry->set_id(i-4);
		menu->add_entry(entry);
		xpos += max_width;
	}

	menu->set_background(bg);
	return menu;
}

BaseGameInfo *ExultMenu::show_mods_menu(ModManager *selgame, Shape_frame *logobg)
{
	Palette *gpal = gwin->get_pal();
	Shape_manager *sman = Shape_manager::get_instance();
	
	gwin->clear_screen(true);
	gpal->load(BUNDLE_CHECK(BUNDLE_EXULT_FLX, EXULT_FLX), EXULT_FLX_EXULT0_PAL);
	gpal->apply();

	int first_mod = 0, num_choices = selgame->get_mod_list().size()-1,
		last_page = num_choices - num_choices % pagesize;
	MenuList *menu = create_mods_menu(selgame, logobg, first_mod);
	menu->set_selection(0);
	BaseGameInfo *sel_mod = 0;
	
	Shape_frame *exultlogo = 0;
	exultlogo = exult_flx.get_shape(EXULT_FLX_EXULT_LOGO_SHP, 1);
	int logox, logoy;
	logox = centerx - exultlogo->get_width()/2;
	logoy = centery - exultlogo->get_height()/2;
	
	do {
		// Interferes with the menu.
#ifdef HAVE_OPENGL
		if (!GL_manager::get_instance())
#endif
			sman->paint_shape(logox,logoy,exultlogo);
#ifdef HAVE_OPENGL
		if (!GL_manager::get_instance())
#endif
			font->draw_text(gwin->get_win()->get_ib8(), 
						gwin->get_win()->get_end_x()-font->get_text_width(VERSION),
						gwin->get_win()->get_end_y()-font->get_text_height()-5, VERSION);
		int choice = menu->handle_events(gwin, menu_mouse);
		switch(choice) {
			case -10: // The incompatibility notice; do nothing
				break;
			case -4: // Return to main menu
				gpal->fade_out(c_fade_out_time/2);
				wait_delay(c_fade_out_time/2);
				gwin->clear_screen(true);
				delete menu;
				return 0;
			default:
				if (choice>=0)
				{
					// Load the game:
					gpal->fade_out(c_fade_out_time);
					sel_mod = selgame->get_mod(choice);
					break;
				}
				else if (handle_menu_click(choice, first_mod, last_page, pagesize))
				{
					delete menu;
					menu = create_mods_menu(selgame, logobg, first_mod);
					gwin->clear_screen(true);
				}
		}
	} while(sel_mod==0);
	delete menu;
	
	gwin->clear_screen(true);
	return sel_mod;
}

static Shape_frame *create_exultlogo(int logox, int logoy, Vga_file& exult_flx, Font *font)
	{
#ifdef HAVE_OPENGL
	if (GL_manager::get_instance())
		{
		Game_window *gwin = Game_window::get_instance();
		int w = gwin->get_win()->get_full_width(), h = gwin->get_win()->get_full_height();
		Shape_frame *logo = exult_flx.get_shape(EXULT_FLX_EXULT_LOGO_SHP, 1);
		Image_buffer8 *buf = dynamic_cast<Image_buffer8 *>
				(gwin->get_win()->get_ib8()->create_another(w, h));
		assert(buf);
		buf->fill8(0);
		logo->paint(buf, logox, logoy);
			// Disable OpenGL rendering for a bit.
		Shape_frame::set_to_render(buf, 0);
		font->draw_text(buf, w-font->get_text_width(VERSION),
					h-font->get_text_height()-5, VERSION);
		Shape_frame::set_to_render(gwin->get_win()->get_ib8(),
					GL_manager::get_instance());
		Shape_frame *exultlogo = new Shape_frame(buf->get_bits(), w, h, 0, 0, true);
		delete buf;
		return exultlogo;
		}
#endif
	return 0;
	}

BaseGameInfo *ExultMenu::run()
{
	Palette *gpal = gwin->get_pal();
	Shape_manager *sman = Shape_manager::get_instance();

	gwin->clear_screen(true);
	gpal->load(BUNDLE_CHECK(BUNDLE_EXULT_FLX, EXULT_FLX), EXULT_FLX_EXULT0_PAL);
	font = fontManager.get_font("CREDITS_FONT");
	fonton = fontManager.get_font("HOT_FONT");
	navfont = fontManager.get_font("NAV_FONT");
	navfonton = fontManager.get_font("HOT_NAV_FONT");

	if(!gamemanager->get_game_count()) {
		int topy = centery-25;
		font->center_text(gwin->get_win()->get_ib8(),
					centerx, topy+20, "WARNING");
		font->center_text(gwin->get_win()->get_ib8(),
					centerx, topy+40, "Could not find the static data for either");
		font->center_text(gwin->get_win()->get_ib8(),
					centerx, topy+50, "\"The Black Gate\" or \"Serpent Isle\".");
		font->center_text(gwin->get_win()->get_ib8(),
					centerx, topy+60, "Please edit the configuration file");
		font->center_text(gwin->get_win()->get_ib8(),
					centerx, topy+70, "and restart Exult");
		gpal->apply();
		while(!wait_delay(200))
			;	
		throw quit_exception(1);

	}
	ExultDataSource mouse_data(BUNDLE_CHECK(BUNDLE_EXULT_FLX, EXULT_FLX),
	                           EXULT_FLX_POINTERS_SHP);
	menu_mouse = new Mouse(gwin, mouse_data);

	//Must check this or it will crash as midi 
	//may not be initialised
	if(Audio::get_ptr()->audio_enabled)
	{
		// Make sure timbre library is correct!
		//Audio::get_ptr()->get_midi()->set_timbre_lib(MyMidiPlayer::TIMBRE_LIB_GM);
		Audio::get_ptr()->start_music(EXULT_FLX_MEDITOWN_MID, true, EXULT_FLX);
	}

#ifdef HAVE_OPENGL
	if (GL_manager::get_instance())
		gwin->get_win()->fill8(0);
#endif
	Shape_frame *exultlogo = exult_flx.get_shape(EXULT_FLX_EXULT_LOGO_SHP, 0);
	int logox = centerx-exultlogo->get_width()/2,
		logoy = centery-exultlogo->get_height()/2;
	sman->paint_shape(logox,logoy,exultlogo);
	gpal->fade_in(c_fade_in_time);
	wait_delay(2000);

	exultlogo = exult_flx.get_shape(EXULT_FLX_EXULT_LOGO_SHP, 1);

	int first_game = 0, num_choices = gamemanager->get_game_count()-1,
		last_page = num_choices - num_choices % pagesize;
	// Erase the old logo.
	gwin->clear_screen(true);

	Shape_frame *logobg = create_exultlogo(logox, logoy, exult_flx, font);
	MenuList *menu = create_main_menu(logobg, first_game);;
	BaseGameInfo *sel_game = 0;
	menu->set_selection(0);

	do {
		// Interferes with the menu.
#ifdef HAVE_OPENGL
		if (GL_manager::get_instance() && !logobg)
			{
			logobg = create_exultlogo(logox, logoy, exult_flx, font);
			menu->set_background(logobg);
			}
		else if (!GL_manager::get_instance())
#endif
			sman->paint_shape(logox,logoy,exultlogo);
#ifdef HAVE_OPENGL
		if (!GL_manager::get_instance())
#endif
			font->draw_text(gwin->get_win()->get_ib8(), 
						gwin->get_win()->get_end_x()-font->get_text_width(VERSION),
						gwin->get_win()->get_end_y()-font->get_text_height()-5, VERSION);
		int choice = menu->handle_events(gwin, menu_mouse);
		switch(choice) {
			case -4: // Setup
				gpal->fade_out(c_fade_out_time);
				setup();
				delete menu;
				delete logobg;
				if(Audio::get_ptr()->audio_enabled)
				{
					// Make sure timbre library is correct!
					//Audio::get_ptr()->get_midi()->set_timbre_lib(MyMidiPlayer::TIMBRE_LIB_GM);
					Audio::get_ptr()->start_music(EXULT_FLX_MEDITOWN_MID, true, EXULT_FLX);
				}

				calc_win();
				logox = centerx-exultlogo->get_width()/2;
				logoy = centery-exultlogo->get_height()/2;
				logobg = create_exultlogo(logox, logoy, exult_flx, font);
				first_game = 0;
				menu = create_main_menu(logobg, first_game);
				menu->set_selection(0);
				break;
			case -3: // Exult Credits
				{
					gpal->fade_out(c_fade_out_time);
					TextScroller credits(
			                     BUNDLE_CHECK(BUNDLE_EXULT_FLX, EXULT_FLX),
			                     EXULT_FLX_CREDITS_TXT,
			                     fontManager.get_font("CREDITS_FONT"),
			                     exult_flx.extract_shape(EXULT_FLX_EXTRAS_SHP));
					credits.run(gwin);
					gwin->clear_screen(true);
					gpal->apply();
				}
				break;
			case -2: // Exult Quotes
				{
					gpal->fade_out(c_fade_out_time);
					TextScroller quotes(BUNDLE_CHECK(
								BUNDLE_EXULT_FLX, EXULT_FLX),
			                    EXULT_FLX_QUOTES_TXT,
			                    fontManager.get_font("CREDITS_FONT"),
			                    exult_flx.extract_shape(EXULT_FLX_EXTRAS_SHP));
					quotes.run(gwin);
					gwin->clear_screen(true);
					gpal->apply();
				}
				break;
			case -1: // Exit
				gpal->fade_out(c_fade_out_time);
				Audio::get_ptr()->stop_music();
				delete menu;
				delete logobg;
				delete menu_mouse;
				throw quit_exception();
			default:
				if (choice>=0 && choice<MAX_GAMES)
				{
					// Load the game:
					gpal->fade_out(c_fade_out_time);
					sel_game = gamemanager->get_game(choice);
				}
				else if (choice>=MAX_GAMES && choice<2*MAX_GAMES)
				{
					// Show the mods for the game:
					gpal->fade_out(c_fade_out_time/2);
					sel_game = show_mods_menu(
							gamemanager->get_game(choice-MAX_GAMES), logobg);
					gwin->clear_screen(true);
					gpal->apply();
				}
				else if (handle_menu_click(choice, first_game, last_page, pagesize))
				{
					delete menu;
					menu = create_main_menu(logobg, first_game);
					gwin->clear_screen(true);
				}
				break;
		}
	} while(sel_game==0);
	delete menu;
	delete logobg;
	gwin->clear_screen(true);
	Audio::get_ptr()->stop_music();
	delete menu_mouse;
	return sel_game;
}

bool get_play_intro (void)
{
	std::string yn;
	config->value("config/gameplay/skip_splash", yn, "no");
	return(yn=="no");
}

void set_play_intro (bool play)
{
	config->set("config/gameplay/skip_splash", play?"no":"yes", true);
}

bool get_play_1st_scene (void)
{
	std::string yn;
	config->value("config/gameplay/skip_intro", yn, "no");
	return(yn=="no");
}

void set_play_1st_scene (bool play)
{
	config->set("config/gameplay/skip_intro", play?"no":"yes", true);
}
