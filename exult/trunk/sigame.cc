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

SI_Game::SI_Game()
	{
		add_shape("gumps/check",2);
		add_shape("gumps/fileio",3);
		add_shape("gumps/fntext",4);
		add_shape("gumps/loadbtn",5);
		add_shape("gumps/savebtn",6);
		add_shape("gumps/disk",19);
		add_shape("gumps/heart",20);
		add_shape("gumps/statatts",23);
		add_shape("gumps/musicbtn",24);
		add_shape("gumps/speechbtn",25);
		add_shape("gumps/soundbtn",26);
		add_shape("gumps/spellbook",38);
		add_shape("gumps/combat",41);
		add_shape("gumps/statsdisplay",42);
		add_shape("gumps/quitbtn",50);
		add_shape("gumps/yesnobox",51);
		add_shape("gumps/yesbtn",52);
		add_shape("gumps/nobtn",53);
	}

SI_Game::~SI_Game()
	{
	}

void SI_Game::play_intro()
	{
		bool skip = false;
		size_t	flisize;
		char	*fli_b;

		// Lord British presents...
		//pal.load("static/lblogo.pal",0);
		U7object lbflic("static/intro.dat", 0);
		lbflic.retrieve(&fli_b, flisize);
		playfli fli0(fli_b+8, flisize-8);
		fli0.info();
		fli0.play(win);
		const char *txt_msg[] = { "& Jeff Freedman, Dancer Vesperman,", 
				"Willem Jan Palenstijn, Tristan Tarrant,", 
				"Max Horn, Luke Dunstan, Ryan Nunn",
				"Driven by the Exult game engine V" VERSION };
		for(int i=0; i<3; i++) {
			gwin->paint_text(0, txt_msg[i], centerx-gwin->get_text_width(0, txt_msg[i])/2, centery+50+15*i);
		}
		skip = wait_delay(2000);
		if(skip)
			return;

		

		// Flic 0: Lord British presents
		// Flic 1: Castle with lightning
		// Flic 2:
		// Flic 3:
		for(int i=1; i<9; i++) {
			U7object flic("static/intro.dat", i);
			flic.retrieve(&fli_b, flisize);
			playfli fli1(fli_b+8, flisize-8);
			fli1.info();
			fli1.play(win);
			delete [] fli_b;
		}
	}

void SI_Game::top_menu()
{
	gwin->paint_shape(topx,topy,menushapes.get_shape(0x2,0));
	pal.load("static/mainshp.flx",26);
	pal.fade_in(60);	
}
	
void SI_Game::end_game(bool success) 
	{
		size_t	flisize;
		char	*fli_b;
		
		for(int i=9; i<14; i++) {
			U7object flic("static/intro.dat", i);
			flic.retrieve(&fli_b, flisize);
			playfli fli1(fli_b+8, flisize-8);
			fli1.play(win);
			delete [] fli_b;
		}
	}

void SI_Game::show_quotes()
	{
		vector<char *> *text = load_text("static/mainshp.flx", 0x10);
		scroll_text(text);
		destroy_text(text);
	}

void SI_Game::show_credits()
	{
		vector<char *> *text = load_text("static/mainshp.flx", 0x0E);
		scroll_text(text);
		destroy_text(text);
	}

bool SI_Game::new_game(Vga_file &shapes)
	{
		int menuy = topy+110;
		
		char npc_name[9];
		char disp_name[10];
		int max_len = 8;
		npc_name[0] = 0;
		int sex = 0;
		int selected = 0;
		int num_choices = 4;
		//pal.load("static/intropal.dat",6);
		SDL_Event event;
		bool editing = true;
		bool ok = true;
		do {
			win->fill8(0,gwin->get_width(),90,0,menuy);
			gwin->paint_shape(topx+10,menuy+10,shapes.get_shape(0xC, selected==0?1:0));
			gwin->paint_shape(topx+10,menuy+25,shapes.get_shape(0x19, selected==1?1:0));
			gwin->paint_shape(topx+250,menuy+10,shapes.get_shape(0x16+sex,0));
			gwin->paint_shape(topx+10,topy+180,shapes.get_shape(0x8,selected==2?1:0));
			gwin->paint_shape(centerx+10,topy+180,shapes.get_shape(0x7,selected==3?1:0));
			if(selected==0)
				sprintf(disp_name, "%s_", npc_name);
			else
				sprintf(disp_name, "%s", npc_name);
			gwin->paint_text(MAINSHP_FONT1, disp_name, topx+50,menuy+10);
			pal.apply();
			SDL_WaitEvent(&event);
			if(event.type==SDL_KEYDOWN) {
				switch(event.key.keysym.sym) {
				case SDLK_SPACE:
					if(selected==1)
						++sex;
					if(sex>4)
						sex = 0;
					break;
				case SDLK_ESCAPE:
					editing = false;
					ok = false;
					break;
				case SDLK_DOWN:
					++selected;
					if(selected==num_choices)
						selected = 0;
					break;
				case SDLK_UP:
					--selected;
					if(selected<0)
						selected = num_choices-1;
					break;
				case SDLK_RETURN:
					if(selected<2) 
						++selected;
					else if(selected==2) {
						editing=false;
						ok = true;
					} else {
						editing = false;
						ok = false;
					}
					break;
				case SDLK_BACKSPACE:
					if(selected==0) {
						if(strlen(npc_name)>0)
							npc_name[strlen(npc_name)-1] = 0;
					}
					break;
				default:
					{
					int c = event.key.keysym.sym;
					if(selected==0 && c>=SDLK_0 && c<=SDLK_z) {
						int len = strlen(npc_name);
						char chr = (event.key.keysym.mod & KMOD_SHIFT) ? toupper(c) : c;
						if(len<max_len) {
							npc_name[len] = chr;
							npc_name[len+1] = 0;
						}
					}
					}
					break;
				}
			}
		} while(editing);
		if(ok)
			gwin->init_gamedat(true);  // FIXME: we need to set the player's name/sex
		win->fill8(0,gwin->get_width(),90,0,menuy);
		return ok;
	}
