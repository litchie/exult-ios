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

#include <unistd.h>
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

Game *game = 0;
extern Configuration *config;
static Exult_Game game_type = BLACK_GATE;

static char av_name[17] = "";
static int av_sex = -1;
static int av_skin = -1;

Game::Game() : menushapes(MAINSHP_FLX)
{	
	jive = false;
	gwin = Game_window::get_game_window();
	win = gwin->get_win();
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
    U7open(in, savename);		// Open file & abort if error.
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
	flic.retrieve(&fli_buf, len);
	playfli fli(fli_buf);
	fli.play(win);
	delete [] fli_buf;
}

void Game::play_audio(const char *archive, int index) 
{
	U7object speech(archive, index);
	// FIXME: should use a DataBuffer
	speech.retrieve("speech.voc");
	audio->playfile("speech.voc", false);
}

void Game::play_midi(int track,bool repeat)
{
	if (game_type == BLACK_GATE) audio->start_music(track,repeat,1);
	else if (game_type == SERPENT_ISLE) audio->start_music(track,repeat,2);
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
	resources[name].str = (char *)str;
	resources[name].num = num;
}

str_int_pair Game::get_resource(const char *name)
{
	return resources[name];
}


void Game::show_menu()
{
	int menuy = topy+120;

	top_menu();
	MenuList *menu = new MenuList();
		
	int menuchoices[] = { 0x04, 0x05, 0x08, 0x06, 0x11, 0x12};
	int num_choices = sizeof(menuchoices)/sizeof(int);
		
	for(int i=0; i<num_choices; i++) {
		menu->add_entry(new MenuEntry(menushapes.get_shape(menuchoices[i],1),
					      menushapes.get_shape(menuchoices[i],0),
					      centerx, menuy+i*11));
	}
		
	menu->set_selected(2);
	char npc_name[16];
	sprintf(npc_name, "Exult");
	do {
		bool created = false;
		
		switch(menu->handle_events(gwin)) {
		case -1: // Exit
			pal.fade_out(30);
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
				menu->set_selected(1);
				break;
			}
			// else fall through
		case 1: // New Game
			if(!created) {
				if(new_game(menushapes))
					menu->set_selected(2);
			} else
				menu->set_selected(2); // This will start the game
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
	} while(menu->get_selected()!=2);
	pal.fade_out(30);
	delete menu;
	gwin->clear_screen();
	audio->stop_music();
}
	
void Game::journey_failed_text()
{
	Font *font = fontManager.get_font("MENU_FONT");
	font->center_text(gwin, centerx, centery+30,  "You must start a new game first.");
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


bool wait_delay(int ms)
{
	SDL_Event event;
	int delay;
	int loops;
	if(ms<=100) {
		delay = ms;
		loops = 1;
	} else {
		delay = 50;
		loops = ms/delay;
	}
	for(int i=0; i<loops; i++) {
		if(SDL_PollEvent(&event)) {
			if((event.type==SDL_KEYDOWN)||(event.type==SDL_MOUSEBUTTONDOWN))
				return true;
		}
		SDL_Delay(delay);
	}
	return false;
}


Exult_Game exult_menu(Game_window *gwin)
{
	Image_window8 *win = gwin->get_win();
	int topx = (gwin->get_width()-320)/2;
	int topy = (gwin->get_height()-200)/2;
	int centerx = gwin->get_width()/2;
	int centery = gwin->get_height()/2;
	int menuy = topy+120;
	Palette pal;
	char *mid_buf;
	size_t len;
	fontManager.add_font("CREDITS_FONT", "<DATA>/exult.flx", 9, 1);
	U7object banner_midi("<DATA>/exult.flx", 8);
	banner_midi.retrieve(&mid_buf, len);
	BufferDataSource *midi_data = new BufferDataSource(mid_buf, len);
	XMIDI midfile(midi_data, false);
	audio->start_music(&midfile, true);
	Vga_file exult_flx("<DATA>/exult.flx");
	
	gwin->paint_shape(topx,topy,exult_flx.get_shape(4, 0));
	pal.load("<DATA>/exult.flx",5);
	pal.fade_in(30);
	wait_delay(2000);
	MenuList *menu = new MenuList();
		
	int menuchoices[] = { 0x06, 0x07, 0x01, 0x00 };
	int num_choices = sizeof(menuchoices)/sizeof(int);
		
	for(int i=0; i<num_choices; i++) {
		menu->add_entry(new MenuEntry(exult_flx.get_shape(menuchoices[i],1),
					      exult_flx.get_shape(menuchoices[i],0),
					      centerx, menuy+i*11));
	}
	menu->set_selected(0);
	Exult_Game sel_game = NONE;
	do {
		gwin->paint_shape(topx,topy,exult_flx.get_shape(4, 1));
		switch(menu->handle_events(gwin)) {
		case -1: // Exit
			pal.fade_out(30);
			exit(0);
		case 0: // Black Gate
			pal.fade_out(30);
			sel_game = BLACK_GATE;
			break;
		case 1: // Serpent Isle
			pal.fade_out(30);
			sel_game = SERPENT_ISLE;
			break;
		case 2: // Exult Credits
			{
				pal.fade_out(30);
				TextScroller credits("<DATA>/exult.flx", 0x03, 
						     fontManager.get_font("CREDITS_FONT"),
						     0);
				credits.run(gwin,pal);
				gwin->clear_screen();
				pal.apply();
			}
			break;
		case 3: // Exult Quotes
			{
				pal.fade_out(30);
				TextScroller quotes("<DATA>/exult.flx", 0x02, 
						    fontManager.get_font("CREDITS_FONT"),
			     			    0);
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
	audio->stop_music();
	
	return sel_game;
}
