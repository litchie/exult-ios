/*
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

#include "alpha_kludges.h"

#ifndef ALPHA_LINUX_CXX
#  include <cstdlib>
#  include <unistd.h>
#endif
#include "files/U7file.h"
#include "flic/playfli.h"
#include "gamewin.h"
#include "Audio.h"
#include "Configuration.h"
#include "game.h"
#include "palette.h"
#include "databuf.h"
#include "font.h"
#include "txtscroll.h"
#include "menulist.h"
#include "cheat.h"
#include "mouse.h"

using std::cout;
using std::endl;
using std::exit;
using std::ifstream;
using std::strcmp;
using std::strcpy;
using std::strncmp;
using std::string;

Game *game = 0;
extern Configuration *config;
extern Cheat cheat;
extern bool get_play_intro(void);
extern void set_play_intro(bool);
extern bool get_play_1st_scene(void);
extern void set_play_1st_scene(bool);
static Exult_Game game_type = BLACK_GATE;
extern void make_screenshot(bool silent = false);

static char av_name[17] = "";
static int av_sex = -1;
static int av_skin = -1;

Game::Game() : menushapes(MAINSHP_FLX)
{	
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

Exult_Game Game::get_game_type()
{
	return game_type;
}

char *Game::get_game_identity(const char *savename)
{
    ifstream in;
    U7open(in, savename);		// Open file; throws an exception in case of an error.
    in.seekg(0x54);			// Get to where file count sits.
    int numfiles = Read4(in);
    char *game_identity = 0;
    in.seekg(0x80);			// Get to file info.
    // Read pos., length of each file.
    long *finfo = new long[2*numfiles];
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


Game *Game::create_game(Exult_Game mygame)
{
	// Choose the startup path
	string data_directory;
	string gametitle;
	
	switch(mygame) {
	case BLACK_GATE:
		gametitle = "blackgate";
		break;
	case SERPENT_ISLE:
		gametitle = "serpentisle";
		break;
	default:
		gametitle = "blackgate";
		break;
	}
	
	string d = "config/disk/game/"+gametitle+"/path";
	config->value(d.c_str(),data_directory,".");
	if(data_directory==".")
		config->set(d.c_str(),data_directory,true);
	cout << "chdir to " << data_directory << endl;
	chdir(data_directory.c_str());
	
	add_system_path("<STATIC>", "static");
	// Discover the game we are running (BG, SI, ...)
	// We do this, because we don't really trust config :-)
	char *static_identity = get_game_identity(INITGAME);

	if((!strcmp(static_identity,"ULTIMA7"))||(!strcmp(static_identity,"FORGE")))
                game_type = BLACK_GATE;
        else if((!strcmp(static_identity,"SERPENT ISLE"))||(!strcmp(static_identity,"SILVER SEED")))
                game_type = SERPENT_ISLE;
	
	switch(game_type) {
	case BLACK_GATE:
		game = new BG_Game();
		break;
	case SERPENT_ISLE:
		game = new SI_Game();
		break;
	default:
		game = 0;
	}

	delete[] static_identity;
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


void Game::show_menu()
{
	int menuy = topy+120;
	ExultDataSource mouse_data(MAINSHP_FLX, 19);
	menu_mouse = new Mouse(gwin, mouse_data);
	
	top_menu();
	MenuList *menu = new MenuList();
		
	int menuchoices[] = { 0x04, 0x05, 0x08, 0x06, 0x11, 0x12};
	int num_choices = sizeof(menuchoices)/sizeof(int);
		
	for(int i=0; i<num_choices; i++) {
		menu->add_entry(new MenuEntry(menushapes.get_shape(menuchoices[i],1),
					      menushapes.get_shape(menuchoices[i],0),
					      centerx, menuy+i*11));
	}
		
	menu->set_selection(2);
	char npc_name[16];
	sprintf(npc_name, "Exult");
	do {
		bool created = false;
		
		switch(menu->handle_events(gwin,menu_mouse)) {
		case -1: // Exit
			pal.fade_out(30);
			Audio::get_ptr()->stop_music();
			exit(0);
		case 0: // Intro
			pal.fade_out(30);
			play_intro();
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
			// else fall through
		case 1: // New Game
			if(!created) {
				if(new_game(menushapes))
					menu->set_selection(2);
			} else
				menu->set_selection(2); // This will start the game
			break;
		case 3: // Credits
			pal.fade_out(30);
			show_credits();
			top_menu();
			break;
		case 4: // Quotes
			pal.fade_out(30);
			show_quotes();
			top_menu();
			break;
		case 5: // End Game
			pal.fade_out(30);
			end_game(true);
			top_menu();
			break;
		default:
			break;
		}
	} while(menu->get_selection()!=2);
	pal.fade_out(30);
	delete menu;
	gwin->clear_screen();
	Audio::get_ptr()->stop_music();
	delete menu_mouse;
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
bool wait_delay(int ms, int startcol, int ncol)
{
	SDL_Event event;
	int delay;
	int loops;

	int loopinterval = (ncol == 0) ? 50 : 10;

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
				default:
					return true;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				return true;
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
	return false;
}

ExultMenu::ExultMenu(Game_window *gw)
{
	gwin = gw;
	ibuf = gwin->get_win()->get_ib8();
	calc_win();
	fontManager.add_font("CREDITS_FONT", "<DATA>/exult.flx", 9, 1);
	exult_flx.load("<DATA>/exult.flx");
}

ExultMenu::~ExultMenu()
{
}

void ExultMenu::calc_win()
{
	topx = (gwin->get_width()-320)/2;
	topy = (gwin->get_height()-200)/2;
	centerx = gwin->get_width()/2;
	centery = gwin->get_height()/2;
	menuy = topy+120;
}


void ExultMenu::setup()
{
	Font *font = fontManager.get_font("CREDITS_FONT");
	MenuList menu;
	
	MenuChoice *palfades = new MenuChoice(exult_flx.get_shape(0x16,1),
			      exult_flx.get_shape(0x16,0),
			      centerx, menuy-33, font);
	palfades->add_choice("Off");
	palfades->add_choice("On");
	palfades->set_choice(gwin->get_fades_enabled()?1:0);
	menu.add_entry(palfades);
	
	MenuChoice *midiconv = new MenuChoice(exult_flx.get_shape(0x14,1),
			      exult_flx.get_shape(0x14,0),
			      centerx, menuy-22, font);
	midiconv->add_choice("None");
	midiconv->add_choice("GM");
	midiconv->add_choice("GS");
	midiconv->add_choice("GS127");
	midiconv->add_choice("GS127DRUM");
	midiconv->set_choice(Audio::get_ptr()->get_midi()->get_music_conversion());
	menu.add_entry(midiconv);
	
	MenuChoice *sfxconv = new MenuChoice(exult_flx.get_shape(0x15,1),
			      exult_flx.get_shape(0x15,0),
			      centerx, menuy-11, font);
	sfxconv->add_choice("None");
	sfxconv->add_choice("GS");
	sfxconv->set_choice(Audio::get_ptr()->get_midi()->get_effects_conversion()==XMIDI_CONVERT_GS127_TO_GS?1:0);
	menu.add_entry(sfxconv);
	
	MenuChoice *playintro = new MenuChoice(exult_flx.get_shape(0x0B,1),
			      exult_flx.get_shape(0x0B,0),
			      centerx, menuy, font);
	playintro->add_choice("Off");
	playintro->add_choice("On");
	playintro->set_choice(get_play_intro()?1:0);
	menu.add_entry(playintro);
	
	MenuChoice *playscene = new MenuChoice(exult_flx.get_shape(0x12,1),
			      exult_flx.get_shape(0x12,0),
			      centerx, menuy+11, font);
	playscene->add_choice("Off");
	playscene->add_choice("On");
	playscene->set_choice(get_play_1st_scene()?1:0);
	menu.add_entry(playscene);

	MenuChoice *fullscreen = new MenuChoice(exult_flx.get_shape(0x0C,1),
			      exult_flx.get_shape(0x0C,0),
			      centerx, menuy+22, font);
	fullscreen->add_choice("Off");
	fullscreen->add_choice("On");
	fullscreen->set_choice(gwin->get_win()->is_fullscreen()?1:0);
	menu.add_entry(fullscreen);
	
	MenuChoice *cheating = new MenuChoice(exult_flx.get_shape(0x0D,1),
				      exult_flx.get_shape(0x0D,0),
				      centerx, menuy+33, font);
	cheating->add_choice("Off");
	cheating->add_choice("On");
	cheating->set_choice(cheat()?1:0);
	menu.add_entry(cheating);
	
	MenuEntry *ok = new MenuEntry(exult_flx.get_shape(0x0E,1),
		      exult_flx.get_shape(0x0E,0),
		      centerx-64, menuy+55);
	int ok_button = menu.add_entry(ok);
	
	MenuEntry *cancel = new MenuEntry(exult_flx.get_shape(0x0F,1),
			 exult_flx.get_shape(0x0F,0),
			 centerx+64, menuy+55);
	int cancel_button = menu.add_entry(cancel);
	
	menu.set_selection(0);
	gwin->clear_screen();
	for(;;) {
		pal.apply();
		int entry = menu.handle_events(gwin,menu_mouse);
		if(entry==ok_button) {
			pal.fade_out(30);
			gwin->clear_screen();
			// Palette fades
			gwin->set_fades_enabled(palfades->get_choice()==1);
			config->set("config/video/disable_fades",gwin->get_fades_enabled()?"no":"yes",true);
			// Midi Conversion
			Audio::get_ptr()->get_midi()->set_music_conversion(midiconv->get_choice());
			// SFX Conversion
			Audio::get_ptr()->get_midi()->set_effects_conversion(sfxconv->get_choice()==1?XMIDI_CONVERT_GS127_TO_GS:XMIDI_CONVERT_NOCONVERSION);
			// Play Intro
			set_play_intro(playintro->get_choice()==1);
			// Play 1st scene
			set_play_1st_scene(playscene->get_choice()==1);
			// Full screen
			if(((fullscreen->get_choice()==0)&&(gwin->get_win()->is_fullscreen()))||
			   ((fullscreen->get_choice()==1)&&(!gwin->get_win()->is_fullscreen())))
				gwin->get_win()->toggle_fullscreen();
			config->set("config/video/fullscreen",gwin->get_win()->is_fullscreen()?"yes":"no",true);
			// Cheating
			cheat.set_enabled(cheating->get_choice()==1);
			calc_win();
			return;
		} else if (entry==cancel_button) {
			pal.fade_out(30);
			gwin->clear_screen();
			return;
		}
	}
}

Exult_Game ExultMenu::run()
{
	ExultDataSource *midi_data = new ExultDataSource("<DATA>/exult.flx", 8);
	XMIDI midfile(midi_data, XMIDI_CONVERT_NOCONVERSION);
	Audio::get_ptr()->start_music(&midfile, true);
	
	ExultDataSource mouse_data("<DATA>/exult.flx", 16);
	menu_mouse = new Mouse(gwin, mouse_data);
	
	gwin->paint_shape(topx,topy,exult_flx.get_shape(4, 0));
	pal.load("<DATA>/exult.flx",5);
	pal.fade_in(30);
	wait_delay(2000);
	MenuList *menu = new MenuList();
		
	int menuchoices[] = { 0x06, 0x07, 0x0A, 0x01, 0x00 , 0x11 };
	int num_choices = sizeof(menuchoices)/sizeof(int);
		
	for(int i=0; i<num_choices; i++) {
		menu->add_entry(new MenuEntry(exult_flx.get_shape(menuchoices[i],1),
					      exult_flx.get_shape(menuchoices[i],0),
					      centerx, menuy+i*11+(i<2?0:11)));
	}
	menu->set_selection(0);
	Exult_Game sel_game = NONE;
	do {
		gwin->paint_shape(topx,topy,exult_flx.get_shape(4, 1));
		switch(menu->handle_events(gwin, menu_mouse)) {
		case 5:
		case -1: // Exit
			pal.fade_out(30);
			Audio::get_ptr()->stop_music();
			exit(0);
		case 0: // Black Gate
			pal.fade_out(30);
			sel_game = BLACK_GATE;
			break;
		case 1: // Serpent Isle
			pal.fade_out(30);
			sel_game = SERPENT_ISLE;
			break;
		case 2: // Setup
			pal.fade_out(30);
			setup();
			pal.apply();
			break;
		case 3: // Exult Credits
			{
				pal.fade_out(30);
				TextScroller credits("<DATA>/exult.flx", 0x03, 
						     fontManager.get_font("CREDITS_FONT"),
						     exult_flx.extract_shape(0x13));
				credits.run(gwin,pal);
				gwin->clear_screen();
				pal.apply();
			}
			break;
		case 4: // Exult Quotes
			{
				pal.fade_out(30);
				TextScroller quotes("<DATA>/exult.flx", 0x02, 
						    fontManager.get_font("CREDITS_FONT"),
			     			    exult_flx.extract_shape(0x13));
				quotes.run(gwin,pal);
				gwin->clear_screen();
				pal.apply();
			}
			break;
		default:
			break;
		}
	} while(sel_game==NONE);
	delete menu;
	
	gwin->clear_screen();
	Audio::get_ptr()->stop_music();
	delete menu_mouse;
	delete midi_data;
	return sel_game;
}
