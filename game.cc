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

#include "files/U7file.h"
#include "flic/playfli.h"
#include "gamewin.h"
#include "Audio.h"
#include "game.h"
#include "palette.h"
#include "databuf.h"

Game *game = 0;
Exult_Game game_type = BLACK_GATE;

Game::Game()
{
	gwin = Game_window::get_game_window();
	win = gwin->get_win();
	topx = (gwin->get_width()-320)/2;
	topy = (gwin->get_height()-200)/2;
	centerx = gwin->get_width()/2;
	centery = gwin->get_height()/2;
	
	if (!gwin->setup_mainshp_fonts())
			gwin->abort ("Unable to setup fonts from 'mainshp.flx' file.");
}

Game::~Game()
{
}

Game *Game::get_game()
{
	return game;
}
Exult_Game Game::get_game_type()
{
	return game_type;
}

Game *Game::create_game(const char *static_identity)
{

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
	return game;
}

bool Game::wait_delay(int ms)
{
	SDL_Event event;
	if(SDL_PollEvent(&event)) {
		if((event.type==SDL_KEYDOWN)||(event.type==SDL_MOUSEBUTTONDOWN))
			return true;
	}
	SDL_Delay(ms);
	return false;
}

void Game::clear_screen()
{
	win->fill8(0,gwin->get_width(),gwin->get_height(),0,0);
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

void Game::play_midi(int track)
{
	audio->start_music(track,0,1);
}

void Game::refresh_screen ()
{
	clear_screen();
	gwin->set_palette(0);
	gwin->paint();
	gwin->fade_palette (50, 1, 0);
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

void Game::show_text_line(int x, int y, const char *s)
{
	// FIXME:
	//The texts used in the main menu contains backslashed sequences that
	//indicates the output format of the lines:
	// \Px   include picture number x (frame of MAINSHP.FLX shape 14h)
	// \C    center line
	// \L    left-aligned line

	char *ptr = s;
	int xpos = x;
	
	if(!strncmp(s,"\\C",2)) {
		ptr += 2;
	}
}

vector<char *> *Game::load_text(const char *archive, int index)
{
	U7object txtobj(archive, index);
	size_t len;
		
	char *txt, *ptr, *end;
	txtobj.retrieve(&txt, len);
	ptr = txt;
	end = ptr+len;

	vector<char *> *text = new vector<char *>();
	while(ptr<end) {
		char *start = ptr;
		ptr = strchr(ptr, '\r');
		*ptr = 0;
		text->push_back(strdup(start));
		ptr += 2;
	}
	delete [] txt;
	for(int i=0; i<text->size(); i++)
		printf("%d - %s\n", i, (*text)[i]);
	return text;
}

void Game::destroy_text(vector<char *> *text)
{
	for(int i=0; i<text->size(); i++)
		delete [] (*text)[i];
	delete text;
}

void Game::show_menu()
	{
		Vga_file menushapes(MAINSHP_FLX);

		int menuy = topy+110;

		top_menu(menushapes);
		
		int menuchoices[] = { 0x04, 0x05, 0x08, 0x06, 0x11, 0x12 };
		int num_choices = sizeof(menuchoices)/sizeof(int);
		int selected = 2;
		SDL_Event event;
		char npc_name[16];
		sprintf(npc_name, "God");
		do {
			bool exit_loop = false;
			do {
				for(int i=0; i<6; i++) {
					Shape_frame *shape = menushapes.get_shape(menuchoices[i],i==selected);
					gwin->paint_shape(centerx-shape->get_width()/2,menuy+i*10,shape);
				}		
				win->show();
				SDL_WaitEvent(&event);
				if(event.type==SDL_KEYDOWN) {
					switch(event.key.keysym.sym) {
					case SDLK_ESCAPE:
						exit(0);
						break;
					case SDLK_UP:
						--selected;
						if(selected<0)
							selected = num_choices-1;
						continue;
					case SDLK_DOWN:
						++selected;
						if(selected==num_choices)
							selected = 0;
						continue;
					case SDLK_RETURN:
						exit_loop = true;
						break;
					default:
						break;
					}
				}
			} while(!exit_loop);
			bool created = false;
			switch(selected) {
			case 0: // Intro
				play_intro();
				top_menu(menushapes);
				break;
			case 2: // Journey Onwards
				created = gwin->init_gamedat(false);
				if(!created)
					break;
				// else fall through
			case 1: // New Game
				if(!created) {
					if(new_game(menushapes))
						selected = 2;
				} else
					selected = 2; // This will start the game
				break;
			case 3: // Credits
				show_credits();
				top_menu(menushapes);
				break;
			case 4: // Quotes
				show_quotes();
				top_menu(menushapes);
				break;
			case 5: // End Game
				end_game(true);
				top_menu(menushapes);
				break;
			default:
				break;
			}
		} while(selected!=2);
		pal.fade_out(30);
		
		clear_screen();
		audio->stop_music();
	}
