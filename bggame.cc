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

#include "files/U7file.h"
#include "flic/playfli.h"
#include "gamewin.h"
#include "Audio.h"
#include "game.h"
#include "palette.h"
#include "databuf.h"
#include "font.h"
#include "txtscroll.h"

#ifndef ALPHA_LINUX_CXX
#  include <cctype>
#  include <cstring>
#endif

using std::toupper;

void create_static(Image_buffer8* ib, int w, int h, int x, int y,
		   int black, int gray, int white) {
	for (int i = x; i < x+w; i++) {
		for (int j = y; j < y+h; j++) {
			unsigned char c;
			switch (rand()%5) {
 				case 0: case 1: c = black; break;
				case 2: case 3: c = gray; break;
				case 4: c = white; break;
			}
			ib->put_pixel8(c, i, j);
		}
	}
}

bool wait_delay_cycle(int ms, int startcol, int ncol)
{
	SDL_Event event;
	int delay;
	int loops;
	if(ms<=20) {
		delay = ms;
		loops = 1;
	} else {
		delay = 10;
		loops = ms/delay;
	}
	for(int i=0; i<loops; i++) {
		unsigned long ticks1 = SDL_GetTicks();
	        // this may be a bit risky... How fast can events be generated?
		while(SDL_PollEvent(&event)) {
			if ((event.type==SDL_KEYDOWN) || 
			    (event.type==SDL_MOUSEBUTTONUP))
				return true;
		}
		Game_window::get_game_window()->get_win()
		  ->rotate_colors(startcol, ncol, 1);
		if (ms > 250)
			Game_window::get_game_window()->get_win()->show();
		unsigned long ticks2 = SDL_GetTicks();
		if (ticks2 - ticks1 > delay)
			i+= (ticks2 - ticks1) / delay - 1;
		else
			SDL_Delay(delay - (ticks2 - ticks1));
	}
	return false;
}


BG_Game::BG_Game()
{
	add_shape("gumps/check",2);
	add_shape("gumps/fileio",3);
	add_shape("gumps/fntext",4);
	add_shape("gumps/loadbtn",5);
	add_shape("gumps/savebtn",6);
	add_shape("gumps/halo",7);
	add_shape("gumps/disk",24);
	add_shape("gumps/heart",25);
	add_shape("gumps/statatts",28);
	add_shape("gumps/musicbtn",29);
	add_shape("gumps/speechbtn",30);
	add_shape("gumps/soundbtn",31);	
	add_shape("gumps/spellbook",43);
	add_shape("gumps/statsdisplay",47);
	add_shape("gumps/combat",46);
	add_shape("gumps/quitbtn",56);
	add_shape("gumps/yesnobox",69);
	add_shape("gumps/yesbtn",70);
	add_shape("gumps/nobtn",71);
	add_shape("gumps/book",32);
	add_shape("gumps/scroll",55);
	add_shape("gumps/combatmode",12);
	add_shape("gumps/slider",14);
	add_shape("gumps/slider_diamond",15);
	add_shape("gumps/slider_right",16);
	add_shape("gumps/slider_left",17);

	add_shape("gumps/box", 0);
	add_shape("gumps/crate", 1);
	add_shape("gumps/barrel", 8);
	add_shape("gumps/bag", 9);
	add_shape("gumps/backpack", 10);
	add_shape("gumps/basket", 11);
	add_shape("gumps/chest", 22);
	add_shape("gumps/shipshold", 26);
	add_shape("gumps/drawer", 27);
	add_shape("gumps/woodsign", 49);
	add_shape("gumps/tombstone", 50);
	add_shape("gumps/goldsign", 51);
	add_shape("gumps/body", 53);

	add_resource("files/shapes/count", 0, 7);
	add_resource("files/shapes/0", "<STATIC>/shapes.vga", 0);
	add_resource("files/shapes/1", "<STATIC>/faces.vga", 0);
	add_resource("files/shapes/2", "<STATIC>/gumps.vga", 0);
	add_resource("files/shapes/3", "<STATIC>/sprites.vga", 0);
	add_resource("files/shapes/4", "<STATIC>/mainshp.flx", 0);
	add_resource("files/shapes/5", "<STATIC>/endshape.flx", 0);
	add_resource("files/shapes/6", "<STATIC>/fonts.vga", 0);

	add_resource("palettes/count", 0, 18);
	add_resource("palettes/0", "<STATIC>/palettes.flx", 0);
	add_resource("palettes/1", "<STATIC>/palettes.flx", 1);
	add_resource("palettes/2", "<STATIC>/palettes.flx", 2);
	add_resource("palettes/3", "<STATIC>/palettes.flx", 3);
	add_resource("palettes/4", "<STATIC>/palettes.flx", 4);
	add_resource("palettes/5", "<STATIC>/palettes.flx", 5);
	add_resource("palettes/6", "<STATIC>/palettes.flx", 6);
	add_resource("palettes/7", "<STATIC>/palettes.flx", 7);
	add_resource("palettes/8", "<STATIC>/palettes.flx", 8);
	add_resource("palettes/9", "<STATIC>/palettes.flx", 10);
	add_resource("palettes/10", "<STATIC>/palettes.flx", 11);
	add_resource("palettes/11", "<STATIC>/palettes.flx", 12);
	add_resource("palettes/12", "<STATIC>/intropal.dat", 0);
	add_resource("palettes/13", "<STATIC>/intropal.dat", 1);
	add_resource("palettes/14", "<STATIC>/intropal.dat", 2);
	add_resource("palettes/15", "<STATIC>/intropal.dat", 3);
	add_resource("palettes/16", "<STATIC>/intropal.dat", 4);
	add_resource("palettes/17", "<STATIC>/intropal.dat", 5);
	
	add_resource("xforms/count", 0, 20);
	add_resource("xforms/0", "<STATIC>/xform.tbl", 0);
	add_resource("xforms/1", "<STATIC>/xform.tbl", 1);
	add_resource("xforms/2", "<STATIC>/xform.tbl", 2);
	add_resource("xforms/3", "<STATIC>/xform.tbl", 3);
	add_resource("xforms/4", "<STATIC>/xform.tbl", 4);
	add_resource("xforms/5", "<STATIC>/xform.tbl", 5);
	add_resource("xforms/6", "<STATIC>/xform.tbl", 6);
	add_resource("xforms/7", "<STATIC>/xform.tbl", 7);
	add_resource("xforms/8", "<STATIC>/xform.tbl", 8);
	add_resource("xforms/9", "<STATIC>/xform.tbl", 9);
	add_resource("xforms/10", "<STATIC>/xform.tbl", 10);
	add_resource("xforms/11", "<STATIC>/xform.tbl", 11);
	add_resource("xforms/12", "<STATIC>/xform.tbl", 12);
	add_resource("xforms/13", "<STATIC>/xform.tbl", 13);
	add_resource("xforms/14", "<STATIC>/xform.tbl", 14);
	add_resource("xforms/15", "<STATIC>/xform.tbl", 15);
	add_resource("xforms/16", "<STATIC>/xform.tbl", 16);
	add_resource("xforms/17", "<STATIC>/xform.tbl", 17);
	add_resource("xforms/18", "<STATIC>/xform.tbl", 18);
	add_resource("xforms/19", "<STATIC>/xform.tbl", 19);
	
	fontManager.add_font("MENU_FONT", "<STATIC>/mainshp.flx", 9, 1);
	fontManager.add_font("END2_FONT", "<STATIC>/endgame.dat", 4, -1);
	fontManager.add_font("END3_FONT", "<STATIC>/endgame.dat", 5, -2);
}

BG_Game::~BG_Game()
{
}

#define WAITDELAY(x) if (wait_delay(x)) { \
			pal.fade_out(30); \
			delete backup; delete backup2; delete backup3; \
			delete cbackup; delete cbackup2; delete cbackup3; \
			delete noise; delete plasma; \
			return; \
		     }

#define WAITDELAYCYCLE(x) if (wait_delay_cycle((x), 16, 95)) { \
			pal.fade_out(30); \
			delete backup; delete backup2; delete backup3; \
			delete cbackup; delete cbackup2; delete cbackup3; \
			return; \
		     }

#define WAITDELAYCYCLE2(x) if (wait_delay_cycle((x), 250, 5)) { \
			pal.fade_out(30); \
			return; \
		     }

#define WAITDELAYCYCLE3(x) if (wait_delay_cycle((x), 240, 15)) { \
			pal.fade_out(30); \
			return; \
		     }

void BG_Game::play_intro()
{
	Vga_file shapes(ENDSHAPE_FLX);
	bool skip = false;
	Palette pal;
	Font *font = fontManager.get_font("END2_FONT");

	const char *txt_msg[] = { "with help from",
			"The Exult Team", 
			"Driven by the Exult game engine V" VERSION };

	Audio::get_ptr()->stop_music();

	// TODO: check/fix other resolutions

	// these backups store the area under the guardian shapes being drawn
	// the cbackups are 'clean' backups, ie. just background
	// the others may have other shapes on them, if those are static
	Image_buffer *backup, *backup2, *backup3;
	Image_buffer *cbackup, *cbackup2, *cbackup3;
	Image_buffer *noise, *plasma;
	Shape_frame *s, *s2, *s3;

	backup = backup2 = backup3 = 0;
	cbackup = cbackup2 = cbackup3 = 0;
	noise = plasma = 0;

	// Lord British presents...  (sh. 0x11)
	pal.load("<STATIC>/intropal.dat",3);
	gwin->paint_shape(topx,topy,shapes.get_shape(0x11,0));
	font->center_text(ibuf, centerx, centery+50, txt_msg[0]);
	font->center_text(ibuf, centerx, centery+65, txt_msg[1]);
	pal.fade_in(30);
	skip = wait_delay(2000);
	play_midi(0);	// Start the birdsongs just before we fade
	pal.fade_out(30);
	if(skip)
		return;


	// Ultima VII logo w/Trees
	// sh. 0x12 = trees, sh. 0x0D = text, sh. 0x0E = butterfly

	s = shapes.get_shape(0x0E,0);
	backup = win->create_buffer(s->get_width(), s->get_height());

	gwin->paint_shape(topx,topy,shapes.get_shape(0x12,0));
	gwin->paint_shape(topx+160,topy+50,shapes.get_shape(0x0D,0));
	font->center_text(ibuf, centerx, centery+50, txt_msg[2]);
	pal.load("<STATIC>/intropal.dat",4);
	pal.fade_in(30);

	WAITDELAY(10000);

	// clear 'Exult' text
	gwin->paint_shape(topx,topy,shapes.get_shape(0x12,0));
	gwin->paint_shape(topx+160,topy+50,shapes.get_shape(0x0D,0));
	win->show();

	// TODO: butterfly path is wrong and too slow

	// Butterfly, fast entrance
	// Aim is to be at topx+130,centery-130/5
	// But to get there quickly
	for(int i=0; i<10; i++) {
		win->get(backup,topx+(i*3) - s->get_xleft(), 
			 centery-(i*3)/5 - s->get_yabove());
		gwin->paint_shape(topx+(i*3), centery-(i*3)/5,
				  shapes.get_shape(0x0E, i%4));
		win->show();
		WAITDELAY(50);
		win->put(backup,topx+(i*3) - s->get_xleft(),
			 centery-(i*3)/5 - s->get_yabove());		
	}
	// And wait.....
	for(int i=10; i<30; i++) {
		win->get(backup,topx+(50) - s->get_xleft(),
			 centery-(50)/5 - s->get_yabove());
		gwin->paint_shape(topx+(50), centery-(50)/5,
				  shapes.get_shape(0x0E, i%4));
		win->show();
		WAITDELAY(50);
		win->put(backup,topx+(50) - s->get_xleft(),
			 centery-(50)/5 - s->get_yabove());
	}
	// Butterfly, final flight
	for(int i=25; i<135; i++) {
		win->get(backup, topx+(i*2) - s->get_xleft(),
			 centery-(i*2)/5 - s->get_yabove());
		gwin->paint_shape(topx+(i*2), centery-(i*2)/5,
				  shapes.get_shape(0x0E, i%4));
		win->show();
		WAITDELAY(50);
		win->put(backup, topx+(i*2) - s->get_xleft(),
			 centery-(i*2)/5 - s->get_yabove());
	}
	for(int i=1; i<13; i++) {
		win->get(backup, 270 - s->get_xleft(),
			 centery-54 - s->get_yabove());
		gwin->paint_shape(270,centery-54, shapes.get_shape(0x0E, i%4));
		win->show();
		WAITDELAY(50*i);
		win->put(backup, 270 - s->get_xleft(),
			 centery-54 - s->get_yabove());
	}
	WAITDELAY(2000);
	delete backup; backup = 0;

	// Enter guardian
	//TODO: reduce sudden facial movements in speech

	play_midi(2);

	// create buffers containing a blue 'plasma' screen and noise
	noise = win->create_buffer(gwin->get_width(),
						gwin->get_height());
	plasma = win->create_buffer(gwin->get_width(),
						gwin->get_height());
	gwin->plasma(gwin->get_width(), gwin->get_height(), 0, 0, 16, 110);
	win->get(plasma, 0, 0);
	create_static((Image_buffer8*)noise, gwin->get_width(), 
		      gwin->get_height(), 0, 0, 0, 7, 15);


	pal.load("<STATIC>/intropal.dat",2);
	pal.set_color(1,0,0,0); //UGLY hack... set font background to black
	pal.apply();

	//TODO: sound effects here!
	//TODO: timing?
	win->show();
       	WAITDELAY(100);
	win->put(noise,0,0); win->show();
	//	WAITDELAY(25);
	win->put(plasma,0,0); win->show();
	//	WAITDELAY(50);
	win->put(noise,0,0); win->show();
	//	WAITDELAY(25);
	win->put(plasma,0,0); win->show();
	//	WAITDELAY(50);
	win->put(noise,0,0); win->show();
	WAITDELAY(100);
	win->put(plasma,0,0); win->show();
	delete plasma; plasma = 0;
	delete noise; noise = 0;

	// First 'popup' (sh. 0x21)
	s = shapes.get_shape(0x21, 0);
	backup = win->create_buffer(s->get_width(), s->get_height());
	win->get(backup, centerx- s->get_xleft(), centery-45- s->get_yabove());
	for(int i=8; i>=-8; i--) {
		gwin->paint_shape(centerx,centery-45,
				  shapes.get_shape(0x21,1+abs(i)));
		win->show();
		WAITDELAYCYCLE(70);
		win->put(backup, centerx - s->get_xleft(), 
			 centery-45 - s->get_yabove());
	}
	delete backup; backup = 0;

	// Second 'popup' (sh. 0x22)
	s = shapes.get_shape(0x22, 0);
	backup = win->create_buffer(s->get_width(), s->get_height());
	win->get(backup, centerx- s->get_xleft(), centery-45- s->get_yabove());
	for(int i=9; i>=-9; i--) {
		gwin->paint_shape(centerx,centery-45,
				  shapes.get_shape(0x22,9-abs(i)));
		win->show();
		WAITDELAYCYCLE(70);
		win->put(backup, centerx - s->get_xleft(),
			 centery-45 - s->get_yabove());
	}
	delete backup; backup = 0;

	// Successful 'popup' (sh. 0x23)
	s = shapes.get_shape(0x23, 0);
	backup = win->create_buffer(s->get_width(), s->get_height());
	cbackup = win->create_buffer(s->get_width(), s->get_height());

	win->get(cbackup, centerx- s->get_xleft(), centery-1- s->get_yabove());
	gwin->paint_shape(centerx,centery-1,s); // frame 0 is static background
	win->get(backup, centerx- s->get_xleft(), centery-1- s->get_yabove());
	for(int i=1; i<16; i++) {
		gwin->paint_shape(centerx,centery-1,shapes.get_shape(0x23,i));
		win->show();
		WAITDELAYCYCLE(70);
		win->put(backup, centerx - s->get_xleft(),
			 centery-1 - s->get_yabove());
	}
	win->put(cbackup, centerx- s->get_xleft(), centery-1- s->get_yabove());
	delete backup; backup = 0;
	delete cbackup; cbackup = 0;

	// Actual appearance
	// sh. 0x1E = mouth, sh. 0x1F = forehead, sh. 0x20 = eyes

	// prepare Guardian speech
	font = fontManager.get_font("END3_FONT");
	U7object textobj(MAINSHP_FLX, 0x0D);
	char * txt, *txt_ptr, *txt_end, *next_txt;
	size_t txt_len;
	next_txt = txt_ptr = txt = textobj.retrieve(txt_len);
	Audio::get_ptr()->playfile(INTROSND,false);
	int txt_height = font->get_text_height();
	int txt_ypos = gwin->get_height()-txt_height-16;

	// backup text area
	backup3 = win->create_buffer(gwin->get_width(),txt_height);
	win->get(backup3, 0, txt_ypos);
	// mouth
	s = shapes.get_shape(0x1E,0);
	backup = win->create_buffer(s->get_width(), s->get_height());
	cbackup = win->create_buffer(s->get_width(), s->get_height());
	win->get(cbackup, centerx - s->get_xleft(), centery - s->get_yabove());
	gwin->paint_shape(centerx,centery,s); // frame 0 is background
	win->get(backup, centerx - s->get_xleft(), centery - s->get_yabove());
	// eyes
	s2 = shapes.get_shape(0x20,0);
	backup2 = win->create_buffer(s2->get_width(), s2->get_height());
	cbackup2 = win->create_buffer(s2->get_width(), s2->get_height());
	win->get(cbackup2, centerx - s2->get_xleft(),
		 centery-12 - s2->get_yabove());
	gwin->paint_shape(centerx,centery-12,s2); // frame 0 is background
	win->get(backup2, centerx - s2->get_xleft(),
		 centery-12 - s2->get_yabove());
	// forehead
	s3 = shapes.get_shape(0x1F,0);
	cbackup3 = win->create_buffer(s3->get_width(), s3->get_height());
	win->get(cbackup3, centerx - s3->get_xleft(),
		 centery-49 - s3->get_yabove());
       	gwin->paint_shape(centerx,centery-49,s3); // forehead isn't animated

	// start speech
	for(int i=0; i<14*20; i++) {
		gwin->paint_shape(centerx,centery-12,
				  shapes.get_shape(0x20,1 + i % 9));
		gwin->paint_shape(centerx,centery,
				  shapes.get_shape(0x1E,1 + i % 14));

		if ((i % 20) == 0) {
			txt_ptr = next_txt;
			txt_end = std::strchr(txt_ptr, '\r');
			*txt_end = '\0';

			next_txt = txt_end+2;
		}

		font->center_text(win->get_ib8(), centerx, txt_ypos, txt_ptr);

		win->show();
		if(wait_delay_cycle(50, 16, 95)) {
			pal.fade_out(30);
			delete [] txt;
			delete backup; delete backup2; delete backup3;
			delete cbackup; delete cbackup2; delete cbackup3;
			Audio::get_ptr()->cancel_streams();
			return;	
		}

		win->put(backup3, 0, txt_ypos);
		win->put(backup, centerx - s->get_xleft(),
				     centery - s->get_yabove());
		win->put(backup2, centerx - s2->get_xleft(),
				     centery-12 - s2->get_yabove());
	}
	win->put(backup3, 0, txt_ypos);
	win->put(cbackup, centerx - s->get_xleft(), centery - s->get_yabove());
	win->put(cbackup2, centerx - s2->get_xleft(), 
		 centery-12 - s2->get_yabove());
	win->put(cbackup3, centerx - s3->get_xleft(),
		 centery-49 - s3->get_yabove());
	
	delete [] txt;
	delete backup; backup = 0;
	delete backup2; backup2 = 0;
	delete backup3; backup3 = 0;
	delete cbackup; cbackup = 0;
	delete cbackup2; cbackup2 = 0;
	delete cbackup3; cbackup3 = 0;

	// G. disappears again (sp. 0x23 again)
	s = shapes.get_shape(0x23, 0);
	backup = win->create_buffer(s->get_width(), s->get_height());
	cbackup = win->create_buffer(s->get_width(), s->get_height());
	win->get(cbackup, centerx- s->get_xleft(), centery-1- s->get_yabove());
	gwin->paint_shape(centerx,centery-1,s); // frame 0 is background
	win->get(backup, centerx- s->get_xleft(), centery-1- s->get_yabove());
	for(int i=15; i>0; i--) {
		gwin->paint_shape(centerx,centery-1,shapes.get_shape(0x23,i));
		win->show();
		if(wait_delay_cycle(70, 16, 95)) {
			pal.fade_out(30);
			Audio::get_ptr()->cancel_streams();
			delete backup; delete cbackup;
			return;	
		}
		win->put(backup, centerx - s->get_xleft(),
			 centery-1 - s->get_yabove());
	}
	win->put(cbackup, centerx- s->get_xleft(), centery-1- s->get_yabove());
	delete backup; backup = 0;
	delete cbackup; cbackup = 0;

	win->show();
	WAITDELAYCYCLE(1000);

	// PC screen
	// TODO: transition (zoom out to PC) scene missing

	play_midi(1);
	
	gwin->clear_screen();
	pal.load("<STATIC>/intropal.dat",1);
	pal.apply();

	// draw monitor (sh. 0x07, 0x08, 0x09, 0x0A: various parts of monitor)
	gwin->paint_shape(centerx, centery, shapes.get_shape(0x07,0));
	gwin->paint_shape(centerx, centery, shapes.get_shape(0x09,0));
	gwin->paint_shape(centerx, centery, shapes.get_shape(0x08,0));
	gwin->paint_shape(centerx, centery, shapes.get_shape(0x0A,0));

	// draw white dot in center of monitor (sh. 0x14)
	gwin->paint_shape(centerx+12, centery-22, shapes.get_shape(0x14,0));

	// draw arm hitting pc (sh. 0x0C)
	s = shapes.get_shape(0x0C, 0);
	backup = win->create_buffer(s->get_width(), s->get_height());
	for (int i=0; i<9; i++) {
		win->get(backup, centerx-96-30*abs(i%4-2) - s->get_xleft(),
			 centery+100 - s->get_yabove());
		gwin->paint_shape(centerx-96-30*abs(i%4-2), centery+100, s);
		win->show();
		win->put(backup, centerx-96-30*abs(i%4-2) - s->get_xleft(),
			 centery+100 - s->get_yabove());
		WAITDELAY(0); //just to catch events
	}

	// screen comes back up (sh. 0x1D)
	gwin->paint_shape(centerx+12, centery-22, shapes.get_shape(0x1D,0));
	win->show();
	delete backup; backup = 0;

	// "Something is obviously amiss"
	gwin->paint_shape(centerx, centery+50, shapes.get_shape(0x15,0));
	win->show();
	WAITDELAY(4000);

	// TODO: misaligned?

	// scroll right (sh. 0x06: map to the right of the monitor)
	for(int i=0;i<194;i+=4) {
		gwin->paint_shape(centerx-i,centery, shapes.get_shape(0x07,0));
		gwin->paint_shape(centerx-i,centery, shapes.get_shape(0x09,0));
		gwin->paint_shape(centerx-i,centery, shapes.get_shape(0x08,0));
		gwin->paint_shape(centerx-i,centery, shapes.get_shape(0x0A,0));
		gwin->paint_shape(centerx-i+12, centery-22, 
				  shapes.get_shape(0x1D,0));
		gwin->paint_shape(topx+320-i,topy, shapes.get_shape(0x06,0));

		if (i > 20 && i < 175) {
			// "It has been a long time..."
			gwin->paint_shape(centerx, centery+50, 
					  shapes.get_shape(0x16,0));
		}
		win->show();
		WAITDELAY(30);
	}

	// scroll down (sh. 0x0B: mouse + orb of moons, below map)
	for(int i=0;i<=50;i+=2) {
		gwin->paint_shape(centerx-194, centery-i,
				  shapes.get_shape(0x07,0));
		gwin->paint_shape(centerx-194, centery-i,
				  shapes.get_shape(0x09,0));
		gwin->paint_shape(centerx-194, centery-i,
				  shapes.get_shape(0x08,0));
		gwin->paint_shape(centerx-194, centery-i,
				  shapes.get_shape(0x0A,0));
		gwin->paint_shape(centerx-194+12, centery-22-i,
				  shapes.get_shape(0x1D,0));
		gwin->paint_shape(topx+319-194, topy-i,
				  shapes.get_shape(0x06,0));
		gwin->paint_shape(topx+319, topy+199-i,
				  shapes.get_shape(0x0B,0));
		// "The mystical Orb beckons you"
		gwin->paint_shape(centerx, topy, shapes.get_shape(0x17,0));

		win->show();
		WAITDELAYCYCLE2(50);
	}
	WAITDELAYCYCLE2(1000);

	gwin->paint_shape(centerx-194, centery-50, shapes.get_shape(0x07,0));
	gwin->paint_shape(centerx-194, centery-50, shapes.get_shape(0x09,0));
	gwin->paint_shape(centerx-194, centery-50, shapes.get_shape(0x08,0));
	gwin->paint_shape(centerx-194, centery-50, shapes.get_shape(0x0A,0));
	gwin->paint_shape(centerx-182, centery-72, shapes.get_shape(0x1D,0));
	gwin->paint_shape(topx+319-194, topy-50, shapes.get_shape(0x06,0));
	gwin->paint_shape(topx+319, topy+149, shapes.get_shape(0x0B,0));
	// "It has opened gateways to Britannia in the past"
	gwin->paint_shape(centerx, topy, shapes.get_shape(0x18,0));
	win->show();

	WAITDELAYCYCLE2(3000);

	// The Moongate
	// sh. 0x02, 0x03, 0x04, 0x05: various parts of moongate
	// sh. 0x00, 0x01: parting trees before clearing

	gwin->clear_screen();
	pal.load("<STATIC>/intropal.dat",5);
	pal.apply();

	// "Behind your house is the circle of stones"
	gwin->paint_shape(centerx, centery+50, shapes.get_shape(0x19,0));
	win->show();

	// TODO: fade in screen while text is onscreen

	WAITDELAY(3000);

	// TODO: misaligned?
	for(int i=120;i>=-170;i-=6) {
		gwin->paint_shape(centerx+1,centery+1,
				  shapes.get_shape(0x02,0));
		gwin->paint_shape(centerx+1,centery+1,
				  shapes.get_shape(0x03,0));
		gwin->paint_shape(centerx+1,centery+1,
				  shapes.get_shape(0x04,0));
		gwin->paint_shape(centerx+1,centery+1,
				  shapes.get_shape(0x05,0));

		gwin->paint_shape(centerx+i,topy, shapes.get_shape(0x00,0));
		gwin->paint_shape(centerx-i,topy, shapes.get_shape(0x01,0));

		// "Why is a moongate already there?"
		gwin->paint_shape(centerx,centery+50,shapes.get_shape(0x1A,0));
		win->show();
		WAITDELAYCYCLE3(50);
	}

	gwin->paint_shape(centerx+1, centery+1, shapes.get_shape(0x02,0));
	gwin->paint_shape(centerx+1, centery+1, shapes.get_shape(0x03,0));
	gwin->paint_shape(centerx+1, centery+1, shapes.get_shape(0x04,0));
	gwin->paint_shape(centerx+1, centery+1, shapes.get_shape(0x05,0));

	// "You have but one path to the answer"
	gwin->paint_shape(centerx, centery+50, shapes.get_shape(0x1C,0));
	win->show();
	
	WAITDELAYCYCLE3(3000);

	// TODO: zoom (run) into moongate

	gwin->clear_screen();
}
	
void BG_Game::top_menu()
{
	play_midi(3, true);
		
	gwin->paint_shape(topx,topy,menushapes.get_shape(0x2,0));
	pal.load("<STATIC>/intropal.dat",0);
	pal.fade_in(60);	
}

void BG_Game::show_journey_failed()
{
	pal.fade_out(50);
	gwin->paint_shape(topx,topy,menushapes.get_shape(0x2,0));
	journey_failed_text();
}
	
void BG_Game::end_game(bool success) 
{
	int	i, j, next = 0;
	int	starty;
	int	centerx = gwin->get_width() /2;
	int 	topy = (gwin->get_height()-200)/2;
	Font *font = fontManager.get_font("MENU_FONT");

	if(!success) {
		TextScroller text("<STATIC>/mainshp.flx", 0x15,
				  font,0);
		gwin->clear_screen();
		pal.load("<STATIC>/intropal.dat",0);
		for(uint32 i=0; i<text.get_count(); i++) {
			text.show_line(gwin, topx, topx+320, topy+20+i*12, i);
		}
		
		pal.fade_in(30);
		wait_delay(10000);
		pal.fade_out(30);
		
		gwin->clear_screen();
		font->center_text(ibuf, centerx, centery-10, "The end of Ultima VII");
		pal.fade_in(30);
		wait_delay(4000);
		pal.fade_out(30);
		
		gwin->clear_screen();
		font->center_text(ibuf, centerx, centery-10, "The end of Britannia as you know it!");
		pal.fade_in(30);
		wait_delay(4000);
		pal.fade_out(30);

		return;
	}

	// Audio buffer
	size_t	size;
	uint8	*buffer;
	
	// Fli Buffers
	size_t	flisize;
	char	*fli_b[3];

	// Clear screen
	gwin->clear_screen();
	win->show();

	U7object flic1(ENDGAME, 0);
	U7object flic2(ENDGAME, 1);
	U7object flic3(ENDGAME, 2);
	U7object speech1(ENDGAME, 7);
	U7object speech2(ENDGAME, 8);
	U7object speech3(ENDGAME, 9);

	/* There seems to be something wrong with the shapes. Needs investigating
	U7object shapes(ENDGAME, 10);
	shapes.retrieve("endgame.shp");
	Shape_file sf("endgame.shp");
	int x = get_width()/2-160;
	int y = get_height()/2-100;
	cout << "Shape in Endgame.dat has " << sf.get_num_frames() << endl;
	*/

	fli_b[0] = flic1.retrieve(flisize);
	playfli fli1(fli_b[0]+8, flisize-8);

	fli_b[1] = flic2.retrieve(flisize);
	playfli fli2(fli_b[1]+8, flisize-8);

	fli_b[2] = flic3.retrieve(flisize);
	playfli fli3(fli_b[2]+8, flisize-8);

	buffer = (uint8 *) speech1.retrieve(size);
	
	fli1.play(win, 0, 0, 0);
	
	// Start endgame music.
	Audio::get_ptr()->start_music(ENDSCORE_XMI,1,false);
	
	for (i = 0; i < 240; i++)
	{
		next = fli1.play(win, 0, 1, next);
		if (wait_delay (10))
		{
			gwin->clear_screen();
			delete [] fli_b[0];
			delete [] fli_b[1];
			delete [] fli_b[2];
			return;
		}
	}
	
	for (i = 1; i < 150; i++)
	{
		next = fli1.play(win, i, i+1, next);
		if (wait_delay (10))
		{
			gwin->clear_screen();
			delete [] buffer;
			delete [] fli_b[0];
			delete [] fli_b[1];
			delete [] fli_b[2];
			return;
		}
	}

	Audio::get_ptr()->play (buffer+8, size-8, false);
	delete [] buffer;
	Font *endfont2 = fontManager.get_font("END2_FONT");
	Font *endfont3 = fontManager.get_font("END3_FONT");

	const char 	*message = "No. You cannot do that! You must not!";
	int	height = topy+200 - endfont2->get_text_height()*2;
	int	width = (gwin->get_width() - endfont2->get_text_width(message)) / 2;

	for (i = 150; i < 204; i++)
	{
		next = fli1.play(win, i, i, next);
		endfont2->draw_text(ibuf, width, height, message);
		
		win->show();
		if (wait_delay (10))
		{
			gwin->clear_screen();
			delete [] fli_b[0];
			delete [] fli_b[1];
			delete [] fli_b[2];
			return;
		}
	}

	// Set new music
	Audio::get_ptr()->start_music(ENDSCORE_XMI,2,false);
	
	// Set speech
	
	buffer = (uint8 *) speech2.retrieve(size);
	Audio::get_ptr()->play (buffer+8, size-8, false);
	delete [] buffer;

	message = "Damn you Avatar!  Damn you!";
	width = (gwin->get_width() - endfont2->get_text_width(message)) / 2;

	for (i = 0; i < 320; i++)
	{
		next = fli2.play(win, i, i, next);
		endfont2->draw_text(ibuf, width, height, message);
		
		win->show();
		if (wait_delay (10))
		{
			gwin->clear_screen();
			delete [] fli_b[0];
			delete [] fli_b[1];
			delete [] fli_b[2];
			return;
		}
	}

	for (i = 1000 + next; i > next; )
	{
		next = fli2.play(win, -1, -1, next, (next-i)/-10);
		win->show ();
		if (wait_delay (10))
		{
			gwin->clear_screen();
			delete [] fli_b[0];
			delete [] fli_b[1];
			delete [] fli_b[2];
			return;
		}
	}

	// Text message 1

	// Paint backgound black
	win->fill8(0,gwin->get_width(),gwin->get_height(),0,0);

	// Paint text
	message = "The Black Gate is destroyed.";
	width = (gwin->get_width() - gwin->get_text_width(0,message)) / 2;
	height = (gwin->get_height() - gwin->get_text_height(0)) / 2;
	
	gwin->paint_text (0, message, width, height);

	// Fade in for 1 sec (50 cycles)
	gwin->fade_palette (50, 1, 0);

	// Display text for 3 seconds
	for (i = 0; i < 30; i++)
	{
		if (wait_delay (100))
		{
			gwin->clear_screen();
			delete [] fli_b[0];
			delete [] fli_b[1];
			delete [] fli_b[2];
			return;
		}
	}

	// Fade out for 1 sec (50 cycles)
	gwin->fade_palette (50, 0, 0);

	
	// Now the second text message


	// Paint backgound black
	win->fill8(0,gwin->get_width(),gwin->get_height(),0,0);

	// Paint text
	message = "The Guardian has been stopped.";
	width = (gwin->get_width() - gwin->get_text_width(0,message)) / 2;

	gwin->paint_text (0, message, width, height);

	// Fade in for 1 sec (50 cycles)
	gwin->fade_palette (50, 1, 0);

	// Display text for 3 seonds
	for (i = 0; i < 30; i++)
	{
		if (wait_delay (100))
		{
			gwin->clear_screen();
			delete [] fli_b[0];
			delete [] fli_b[1];
			delete [] fli_b[2];
			return;
		}
	}

	// Fade out for 1 sec (50 cycles)
	gwin->fade_palette (50, 0, 0);
	
	// Now for the final flic

	next = 0;
	for (i = 0; i <= 200; i+=7)
	{
		next = fli3.play(win, 0, 1, next, i/2);
		if (wait_delay (10))
		{
			gwin->clear_screen();
			delete [] fli_b[0];
			delete [] fli_b[1];
			delete [] fli_b[2];
			return;
		}
	}

	buffer = (uint8 *) speech3.retrieve(size);
	Audio::get_ptr()->play (buffer+8, size-8, false);
	delete [] buffer;

	playfli::fliinfo finfo;
	fli3.info (&finfo);
	
	int	m;
	const char *txt_screen0[] = {
		"Avatar! You think you have won>",
		"Think again! You are unable to",
		"leave britannia, whereas I am free",
		"to enter other worlds",
		"Perhaps your puny Earth shall be",
		"my NEXT target!."
	};

	starty = (gwin->get_height() - endfont3->get_text_height()*8)/2;

	for (i = next+29000; i > next; )
	{
		for (j = 0; j < finfo.frames; j++)
		{
			next = fli3.play(win, j, j, next);
			for(m=0; m<6; m++)
				endfont3->center_text(ibuf, centerx, starty+endfont3->get_text_height()*m, txt_screen0[m]);

			win->show ();
			if (wait_delay (10))
			{
				gwin->clear_screen();
				delete [] fli_b[0];
				delete [] fli_b[1];
				delete [] fli_b[2];
				return;
			}
		}
	}

	
	for (i = 200; i > 0; i-=7)
	{
		next = fli3.play(win, 0, 0, next, i/2);
		if (wait_delay (10))
		{
			gwin->clear_screen();
			delete [] fli_b[0];
			delete [] fli_b[1];
			delete [] fli_b[2];
			return;
		}
	}

	// Text Screen 1

	// Paint backgound black
	win->fill8(0,gwin->get_width(),gwin->get_height(),0,0);


	const char *txt_screen1[] = {
		"In the months following the climactic",
		"battle at The Black Gate, Britannia",
		"is set upon the long road to recovery",
		"from its various plights.",
		" ",
		"Upon your return to Britain,",
		"Lord British decreed that",
		"The Fellowship be outlawed",
		"and all of the branches were",
		"soon destroyed."
	};

	starty = (gwin->get_height() - gwin->get_text_height(0)*10)/2;
	
	for(i=0; i<10; i++)
		gwin->paint_text(0, txt_screen1[i], centerx-gwin->get_text_width(0, txt_screen1[i])/2, starty+gwin->get_text_height(0)*i);

	// Fade in for 1 sec (50 cycles)
	gwin->fade_palette (50, 1, 0);

	// Display text for 20 seonds (only 10 at the moment)
	for (i = 0; i < 100; i++)
	{
		if (wait_delay (100))
		{
			gwin->clear_screen();
			delete [] fli_b[0];
			delete [] fli_b[1];
			delete [] fli_b[2];
			return;
		}
	}

	// Fade out for 1 sec (50 cycles)
	gwin->fade_palette (50, 0, 0);

	if (wait_delay (10))
	{
		gwin->clear_screen();
		delete [] fli_b[0];
		delete [] fli_b[1];
		delete [] fli_b[2];
		return;
	}

	// Text Screen 2

	// Paint backgound black
	win->fill8(0,gwin->get_width(),gwin->get_height(),0,0);

	const char *txt_screen2[] = {
		"The frustration you feel at having been",
		"stranded in Britannia is somewhat",
		"alleviated by the satisfaction that you",
		"solved the gruesome murders committed",
		"by The Fellowship and even avenged the",
		"death of Spark's father."
	};

	starty = (gwin->get_height() - gwin->get_text_height(0)*6)/2;
	
	for(i=0; i<6; i++)
		gwin->paint_text(0, txt_screen2[i], centerx-gwin->get_text_width(0, txt_screen2[i])/2, starty+gwin->get_text_height(0)*i);


	// Fade in for 1 sec (50 cycles)
	gwin->fade_palette (50, 1, 0);

	// Display text for 20 seonds (only 8 at the moment)
	for (i = 0; i < 80; i++)
	{
		if (wait_delay (100))
		{
			gwin->clear_screen();
			delete [] fli_b[0];
			delete [] fli_b[1];
			delete [] fli_b[2];
			return;
		}
	}

	// Fade out for 1 sec (50 cycles)
	gwin->fade_palette (50, 0, 0);


	if (wait_delay (10))
	{
		gwin->clear_screen();
		delete [] fli_b[0];
		delete [] fli_b[1];
		delete [] fli_b[2];
		return;
	}


	// Text Screen 3 

	// Paint backgound black
	win->fill8(0,gwin->get_width(),gwin->get_height(),0,0);

	const char *txt_screen3[] = {
		"And although you are, at the moment,",
		"helpless to do anything about",
		"The Guardian's final threat,",
		"another thought nags at you...",
		"what became of Batlin, the fiend",
		"who got away?"
	};

	starty = (gwin->get_height() - gwin->get_text_height(0)*6)/2;
	
	for(i=0; i<6; i++)
		gwin->paint_text(0, txt_screen3[i], centerx-gwin->get_text_width(0, txt_screen3[i])/2, starty+gwin->get_text_height(0)*i);

	// Fade in for 1 sec (50 cycles)
	gwin->fade_palette (50, 1, 0);

	// Display text for 20 seonds (only 8 at the moment)
	for (i = 0; i < 80; i++)
	{
		if (wait_delay (100))
		{
			gwin->clear_screen();
			delete [] fli_b[0];
			delete [] fli_b[1];
			delete [] fli_b[2];
			return;
		}
	}

	// Fade out for 1 sec (50 cycles)
	gwin->fade_palette (50, 0, 0);


	if (wait_delay (10))
	{
		gwin->clear_screen();
		delete [] fli_b[0];
		delete [] fli_b[1];
		delete [] fli_b[2];
		return;
	}


	// Text Screen 4

	// Paint backgound black
	win->fill8(0,gwin->get_width(),gwin->get_height(),0,0);

	const char *txt_screen4[] = {
		"That is another story...", 
		"one that will take you",
		"to a place called",
		"The Serpent Isle..."
	};

	starty = (gwin->get_height() - gwin->get_text_height(0)*4)/2;
	
	for(i=0; i<4; i++)
		gwin->paint_text(0, txt_screen4[i], centerx-gwin->get_text_width(0, txt_screen4[i])/2, starty+gwin->get_text_height(0)*i);


	// Fade in for 1 sec (50 cycles)
	gwin->fade_palette (50, 1, 0);

	// Display text for 10 seonds (only 5 at the moment)
	for (i = 0; i < 50; i++)
	{
		if (wait_delay (100))
		{
			gwin->clear_screen();
			delete [] fli_b[0];
			delete [] fli_b[1];
			delete [] fli_b[2];
			return;
		}
	}

	// Fade out for 1 sec (50 cycles)
	gwin->fade_palette (50, 0, 0);


	gwin->clear_screen();
	delete [] fli_b[0];
	delete [] fli_b[1];
	delete [] fli_b[2];
}

void BG_Game::show_quotes()
{
	play_midi(5);
	TextScroller quotes("<STATIC>/mainshp.flx", 0x10, 
			fontManager.get_font("MENU_FONT"),
			menushapes.extract_shape(0x14)
			);
	pal.load("<STATIC>/intropal.dat",6);
	quotes.run(gwin,pal);
	pal.load("<STATIC>/intropal.dat",0);
}

void BG_Game::show_credits()
{
	
	play_midi(4);
	TextScroller credits("<STATIC>/mainshp.flx", 0x0E, 
			fontManager.get_font("MENU_FONT"),
			menushapes.extract_shape(0x14)
			);
	pal.load("<STATIC>/intropal.dat",6);
	credits.run(gwin,pal);
	pal.load("<STATIC>/intropal.dat",0);
}

bool BG_Game::new_game(Vga_file &shapes)
{
	int menuy = topy+110;
	Font *font = fontManager.get_font("MENU_FONT");
	
	char npc_name[17];
	char disp_name[18];
	int max_len = 16;
	npc_name[0] = 0;
	int sex = 0;
	int selected = 0;
	int num_choices = 4;
	pal.load("<STATIC>/intropal.dat",6);
	SDL_Event event;
	bool editing = true;
	bool redraw = true;
	bool ok = true;
	do
	{
		if (redraw)
		{
			win->fill8(0,gwin->get_width(),90,0,menuy);
			gwin->paint_shape(topx+10,menuy+10,shapes.get_shape(0xC, selected==0?1:0));
			gwin->paint_shape(topx+10,menuy+25,shapes.get_shape(0xA, selected==1?1:0));
			gwin->paint_shape(topx+50,menuy+25,shapes.get_shape(0xB,sex));
			gwin->paint_shape(topx+250,menuy+10,shapes.get_shape(sex,0));
			gwin->paint_shape(topx+10,topy+180,shapes.get_shape(0x8,selected==2?1:0));
			gwin->paint_shape(centerx+10,topy+180,shapes.get_shape(0x7,selected==3?1:0));
			if(selected==0)
				sprintf(disp_name, "%s_", npc_name);
			else
				sprintf(disp_name, "%s", npc_name);
			font->draw_text(ibuf, topx+50, menuy+10, disp_name);
			pal.apply();
			redraw = false;
		}
		SDL_WaitEvent(&event);
		if(event.type==SDL_KEYDOWN)
		{
			redraw = true;
			switch(event.key.keysym.sym)
			{
			case SDLK_SPACE:
				if(selected==0)
				{
					int len = std::strlen(npc_name);
					if(len<max_len)
					{
						npc_name[len] = ' ';
						npc_name[len+1] = 0;
					}
				}
				else if(selected==1)
					sex = !sex;
				else if(selected==2)
				{
					editing=false;
					ok = true;
				}
				else if(selected==3)
				{
					editing = false;
					ok = false;
				}
				break;
			case SDLK_LEFT:
			case SDLK_RIGHT:
				if(selected==1)
					sex = !sex;
				break;
			case SDLK_ESCAPE:
				editing = false;
				ok = false;
				break;
			case SDLK_TAB:
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
				else if(selected==2)
				{
					editing=false;
					ok = true;
				}
				else
				{
					editing = false;
					ok = false;
				}
				break;
			case SDLK_BACKSPACE:
				if(selected==0)
				{
					if(std::strlen(npc_name)>0)
						npc_name[std::strlen(npc_name)-1] = 0;
				}
				break;
			default:
				{
					int c = event.key.keysym.sym;
					if(selected==0 && c>=SDLK_0 && c<=SDLK_z)
					{
						int len = std::strlen(npc_name);
						char chr = (event.key.keysym.mod & KMOD_SHIFT) ? toupper(c) : c;
						if(len<max_len)
						{
							npc_name[len] = chr;
							npc_name[len+1] = 0;
						}
					}
					else
					{
						redraw = false;
					}
				}
				break;
			}
		}
	}
	while(editing);

	if(ok)
	{
		set_avname (npc_name);
		set_avsex (sex);
		ok =gwin->init_gamedat(true);
	}
	win->fill8(0,gwin->get_width(),90,0,menuy);

	return ok;
}
