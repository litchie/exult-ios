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
		Palette pal;
		size_t	flisize;
		char	*fli_b;

		int topx = (gwin->get_width()-320)/2;
		int topy = (gwin->get_height()-200)/2;
		int centerx = gwin->get_width()/2;
		int centery = gwin->get_height()/2;

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
	
void SI_Game::show_menu()
	{
		Vga_file menushapes(MAINSHP_FLX);
		Palette pal;

		int topx = (gwin->get_width()-320)/2;
		int topy = (gwin->get_height()-200)/2;
		int centerx = gwin->get_width()/2;
		
		gwin->paint_shape(topx,topy,menushapes.get_shape(0x2,0));
		pal.load("static/u72_logo.pal",0);
		pal.fade_in(60);
		
		int menuchoices[] = { 0x04, 0x05, 0x08, 0x06, 0x11, 0x12 };
		int selected = 2;
		for(int i=0; i<6; i++) {
			Shape_frame *shape = menushapes.get_shape(menuchoices[i],i==selected);
			gwin->paint_shape(centerx-shape->get_width()/2,topy+110+i*10,shape);
		}		
		win->show();
		while(!wait_delay(100))
			;
		pal.fade_out(30);
		clear_screen();
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
	}

void SI_Game::show_credits()
	{
	}
