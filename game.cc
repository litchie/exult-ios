/*
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef ALPHA_LINUX_CXX
#  include <cstdlib>
#  include <unistd.h>
#endif
#include "menulist.h"
#include "Audio.h"
#include "Configuration.h"
#include "databuf.h"
#include "exult.h"
#include "exult_flx.h"
#include "files/U7file.h"
#include "files/utils.h"
#include "flic/playfli.h"
#include "font.h"
#include "game.h"
#include "bggame.h"
#include "sigame.h"
#include "devgame.h"
#include "gamewin.h"
#include "keys.h"
#include "mouse.h"
#include "palette.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::strcmp;
using std::strcpy;
using std::strncmp;
using std::string;
using std::snprintf;

bool Game::new_game_flag = false;
bool Game::editing_flag = false;
Game *game = 0;
Exult_Game Game::game_type = BLACK_GATE;

static char av_name[17] = "";
static int av_sex = -1;
static int av_skin = -1;

std::string Game::gametitle;

unsigned int Game::ticks = 0;

Game::Game() : menushapes()
{
	try {				// Okay to fail if development game.
		menushapes.load(MAINSHP_FLX);
	} catch (const exult_exception &e) {
		if (!is_editing())
			throw e;
	}
	jive = false;
	gwin = Game_window::get_game_window();
	win = gwin->get_win();
	ibuf = win->get_ib8();
	topx = (gwin->get_width()-320)/2;
	topy = (gwin->get_height()-200)/2;
	centerx = gwin->get_width()/2;
	centery = gwin->get_height()/2;
}

Game::~Game()
{
}

char *Game::get_game_identity(const char *savename)
{
    ifstream in;
    try {
        U7open(in, savename);		// Open file.
    } catch (const exult_exception &e) {
	if (is_editing())		// Okay if creating a new game.
		return std::strdup(gametitle.c_str());
	throw e;
    }
    in.seekg(0x54);			// Get to where file count sits.
    int numfiles = Read4(in);
    char *game_identity = 0;
    in.seekg(0x80);			// Get to file info.
    // Read pos., length of each file.
    sint32 *finfo = new sint32[2*numfiles];
    int i;
    for (i = 0; i < numfiles; i++)
      {
	finfo[2*i] = Read4(in);	// The position, then the length.
	finfo[2*i + 1] = Read4(in);
      }
    for (i = 0; i < numfiles; i++)	// Now read each file.
      {
	// Get file length.
	int len = finfo[2*i + 1] - 13;
	if (len <= 0)
	  continue;
	in.seekg(finfo[2*i]);	// Get to it.
	char fname[50];		// Set up name.
	in.read(fname, 13);
	if (!strcmp("identity",fname))
	    {
      	      game_identity = new char[len];
	      in.read(game_identity, len);
	      // Truncate identity
	      char *ptr = game_identity;
	      for(; (*ptr!=0x1a && *ptr!=0x0d); ptr++)
	      	;
	      *ptr = 0;
	      break;
	    }
      }
    delete [] finfo;
    return game_identity;
}


Game *Game::create_game(Exult_Game mygame, const char *title)
{
	// Choose the startup path
	string data_directory, keyfilename;
	
	switch(mygame) {
	case EXULT_DEVEL_GAME:
		assert(title != 0);
		gametitle = title;
		game_type = mygame;
		break;
	case SERPENT_ISLE:
		gametitle = "serpentisle";
		break;
	case BLACK_GATE:
	default:
		gametitle = "blackgate";
		break;
	}
	
	string d = "config/disk/game/"+gametitle+"/path";
	config->value(d.c_str(),data_directory,".");
	if(data_directory==".") config->set(d.c_str(),data_directory,true);

	cout << "setting game directories to: " << data_directory << endl;
	string static_dir = data_directory + "/static";
	string gamedat_dir = data_directory + "/gamedat";
	
	add_system_path("<STATIC>", static_dir.c_str());
	add_system_path("<GAMEDAT>", gamedat_dir.c_str());
	add_system_path("<SAVEGAME>", data_directory.c_str());
					// See if map-editing.
	d = "config/disk/game/"+gametitle+"/editing";
	config->value(d.c_str(), editing_flag, false);
					// A patch directory is optional.
	d = "config/disk/game/"+gametitle+"/patch";
	string patch_directory;
	config->value(d.c_str(), patch_directory, "");
	if (patch_directory != "")
		add_system_path("<PATCH>", patch_directory.c_str());

	// Discover the game we are running (BG, SI, ...)
	// We do this, because we don't really trust config :-)
	if (game_type != EXULT_DEVEL_GAME) {
		char *static_identity = get_game_identity(INITGAME);

		if((!strcmp(static_identity,"ULTIMA7"))||
		   (!strcmp(static_identity,"FORGE")))
                	game_type = BLACK_GATE;
        	else if((!strcmp(static_identity,"SERPENT ISLE"))||
			(!strcmp(static_identity,"SILVER SEED")))
	                game_type = SERPENT_ISLE;
		delete[] static_identity;
	}
	switch(game_type) {
	case BLACK_GATE:
		cout << "Starting a BLACK GATE game" << endl;
		game = new BG_Game();
		break;
	case SERPENT_ISLE:
		cout << "Starting a SERPENT ISLE game" << endl;
		game = new SI_Game();
		break;
	case EXULT_DEVEL_GAME:
		cout << "Starting '" << gametitle << "' game" << endl;
		game = new DEV_Game();
		break;
	default:
		game = 0;
	}

	return game;
}


void Game::play_flic(const char *archive, int index) 
{
	char *fli_buf;
	size_t len;
	U7object flic(archive, index);
	fli_buf = flic.retrieve(len);
	playfli fli(fli_buf);
	fli.play(win);
	delete [] fli_buf;
}

void Game::play_audio(const char *archive, int index) 
{
	U7object speech(archive, index);
	// FIXME: should use a DataBuffer
	speech.retrieve("speech.voc");
	Audio::get_ptr()->playfile("speech.voc", false);
}

void Game::play_midi(int track,bool repeat)
{
	if (game_type == BLACK_GATE) Audio::get_ptr()->start_music(track,repeat,1);
	else if (game_type == SERPENT_ISLE) Audio::get_ptr()->start_music(track,repeat,2);
}

void Game::add_shape(const char *name, int shapenum) 
{
	shapes[name] = shapenum;
}

int Game::get_shape(const char *name)
{
	return shapes[name];
}

void Game::add_resource(const char *name, const char *str, int num) 
{
	resources[name].str = str;
	resources[name].num = num;
}

str_int_pair Game::get_resource(const char *name)
{
	return resources[name];
}


bool Game::show_menu()
{
	int menuy = topy+120;
					// Brand-new game in development?
	if (is_editing() && !U7exists(MAINSHP_FLX))
		{
		bool first = !U7exists(IDENTITY);
		if (first)
			set_avname("Newbie");
		if (!gwin->init_gamedat(first))
			return false;
		return true;
		}
	ExultDataSource mouse_data(MAINSHP_FLX, 19);
	menu_mouse = new Mouse(gwin, mouse_data);
	
	top_menu();
	MenuList *menu = 0;

		
	int menuchoices[] = { 0x04, 0x05, 0x08, 0x06, 0x11, 0x12, 0x07 };
	int num_choices = sizeof(menuchoices)/sizeof(int);
	int *menuentries = new int[num_choices];
	
	Vga_file exult_flx("<DATA>/exult.flx");
	char npc_name[16];
	snprintf(npc_name, 16, "Exult");
	bool play = false;
	bool fadeout = true;
	bool exitmenu = false;
	
	do {
		int entries = 0;
		if(!menu) {
			entries = 0;
			menu = new MenuList();	
			for(int i=0; i<num_choices; i++) {
				if((i!=4 && i!=5) || (i==4 && U7exists("<SAVEGAME>/quotes.flg")) || (i==5 && U7exists("<SAVEGAME>/endgame.flg"))) {
					menu->add_entry(new MenuEntry(menushapes.get_shape(menuchoices[i],1),
						      menushapes.get_shape(menuchoices[i],0),
						      centerx, menuy+entries*11));
					menuentries[entries++]=i;
				}
			}
			menu->set_selection(2);
		}
	
		bool created = false;
		int choice = menu->handle_events(gwin, menu_mouse);
		switch(choice<0?choice:menuentries[choice]) {
		case -1: // Exit
			pal.fade_out(c_fade_out_time);
			Audio::get_ptr()->stop_music();
			throw quit_exception();
		case 0: // Intro
			pal.fade_out(c_fade_out_time);
			play_intro();
			gwin->clear_screen(true);
			top_menu();
			break;
		case 2: // Journey Onwards
			created = gwin->init_gamedat(false);
			if(!created) {
				show_journey_failed();
				top_menu();
				menu->set_selection(1);
				break;
			}
			exitmenu = true;
			fadeout = true;
			play = true;
			break;
		case 1: // New Game
			if(!created) {
				if(new_game(menushapes))
					exitmenu = true;
				else
					break;
			} else
				exitmenu = true;
			fadeout = false;
			play = true;
			break;
		case 3: // Credits
			pal.fade_out(c_fade_out_time);
			show_credits();
			delete menu;
			menu = 0;
			top_menu();
			break;
		case 4: // Quotes
			pal.fade_out(c_fade_out_time);
			show_quotes();
			top_menu();
			break;
		case 5: // End Game
			pal.fade_out(c_fade_out_time);
			end_game(true);
			top_menu();
			break;
		case 6: // Return to Menu
			play = false;
			exitmenu = true;
			fadeout = true;
			break;
		default:
			break;
		}
	} while(!exitmenu);

	if (fadeout) {
		pal.fade_out(c_fade_out_time);
		gwin->clear_screen(true);
	}
	delete menu;
	delete[] menuentries;
	Audio::get_ptr()->stop_music();
	delete menu_mouse;
	return play;
}
	
void Game::journey_failed_text()
{
	Font *font = fontManager.get_font("MENU_FONT");
	font->center_text(ibuf, centerx, centery+30,  "You must start a new game first.");
	pal.fade_in(50);
	while (!wait_delay(10))
		;
	pal.fade_out(50);
}
	
const char *Game::get_avname ()
{
	if (av_name[0])
		return av_name;
	else
		return NULL;
}

int Game::get_avsex ()
{
	return av_sex;
}
int Game::get_avskin ()
{
	return av_skin;
}

// Assume safe
void Game::set_avname (char *name)
{
	strcpy (av_name, name);
}

void Game::set_avsex (int sex)
{
	av_sex = sex;
}

void Game::set_avskin (int skin)
{
	av_skin = skin;
}

void Game::clear_avname ()
{
	av_name[0] = 0;
	new_game_flag = false;
}

void Game::clear_avsex ()
{
	av_sex = -1;
}

void Game::clear_avskin ()
{
	av_skin = -1;
}


// wait ms milliseconds, while cycling colours startcol to startcol+ncol-1
// return 0 if time passed completly, 1 if user pressed any key or mouse button,
// and 2 if user pressed Return/Enter
int wait_delay(int ms, int startcol, int ncol)
{
	SDL_Event event;
	int delay;
	int loops;
	bool mouse_down = false;

	int loopinterval = (ncol == 0) ? 50 : 10;
	if (!ms) ms = 1;
	if(ms <= 2*loopinterval) {
		delay = ms;
		loops = 1;
	} else {
		delay = loopinterval;
		loops = ms/delay;
	}

	for(int i=0; i<loops; i++) {
		unsigned long ticks1 = SDL_GetTicks();
	        // this may be a bit risky... How fast can events be generated?
		while(SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_RSHIFT: case SDLK_LSHIFT:
				case SDLK_RCTRL: case SDLK_LCTRL:
				case SDLK_RALT: case SDLK_LALT:
				case SDLK_RMETA: case SDLK_LMETA:
				case SDLK_RSUPER: case SDLK_LSUPER:
				case SDLK_NUMLOCK: case SDLK_CAPSLOCK:
				case SDLK_SCROLLOCK:
					break;
				case SDLK_s:
					if ((event.key.keysym.mod&KMOD_ALT) &&
					    (event.key.keysym.mod&KMOD_CTRL))
						make_screenshot(true);
					break;
				case SDLK_RETURN:
				case SDLK_KP_ENTER:
					return 2;
					break;
				default:
					return 1;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouse_down = true;
				break;
			case SDL_MOUSEBUTTONUP:
				//if (mouse_down)
					return 1;
				break;
			default:
				break;
			}
		}
		if (ncol > 0) {
			Game_window::get_game_window()->get_win()
				->rotate_colors(startcol, ncol, 1);
			if (ms > 250)
			  Game_window::get_game_window()->get_win()->show();
		}
		unsigned long ticks2 = SDL_GetTicks();
		if (ticks2 - ticks1 > delay)
			i+= (ticks2 - ticks1) / delay - 1;
		else
			SDL_Delay(delay - (ticks2 - ticks1));
	}
	
	return 0;
}
