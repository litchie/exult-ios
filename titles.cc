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
#include "titles.h"
#include "palette.h"

Titles::Titles()
	{
		gwin = Game_window::get_game_window();
		win = gwin->get_win();
	}

Titles::~Titles()
	{
	}

bool Titles::wait_delay(int ms)
	{
		SDL_Event event;
		if(SDL_PollEvent(&event)) {
			if((event.type==SDL_KEYDOWN)||(event.type==SDL_MOUSEBUTTONDOWN))
				return true;
		}
		SDL_Delay(ms);
		return false;
	}

void Titles::play_intro()
	{
		Vga_file shapes(ENDSHAPE_FLX);
		bool skip = false;
		Palette pal;

		int topx = (gwin->get_width()-320)/2;
		int topy = (gwin->get_height()-200)/2;
		int centerx = gwin->get_width()/2;
		int centery = gwin->get_height()/2;

		// Lord British presents...
		pal.load("static/intropal.dat",3);
		gwin->paint_shape(topx,topy,shapes.get_shape(0x11,0));
		char *txt_msg[] = { "& Jeff Freedman, Dancer Vesperman,", 
				"Willem Jan Palenstijn, Tristan Tarrant,", 
				"Max Horn, Coder Infidel",
				"Driven by the Exult game engine V" VERSION };
		for(int i=0; i<3; i++) {
			gwin->paint_text(0, txt_msg[i], centerx-gwin->get_text_width(0, txt_msg[i])/2, centery+50+15*i);
		}
		pal.fade_in(30);
		skip = wait_delay(2000);
		pal.fade_out(30);
		if(skip)
			return;
		// Ultima VII logo w/Trees
		play_midi(0);
		gwin->paint_shape(topx,topy,shapes.get_shape(0x12,0));
		gwin->paint_shape(topx+160,topy+30,shapes.get_shape(0x0D,0));
		gwin->paint_text(0, txt_msg[3], centerx-gwin->get_text_width(0, txt_msg[3])/2, centery+50);
		pal.load("static/intropal.dat",4);
		pal.fade_in(30);
		if(wait_delay(1500)) {
			pal.fade_out(30);
			return;
		}
		for(int i=0; i<270; i++) {
			gwin->paint_shape(topx,topy,shapes.get_shape(0x12,0));
			gwin->paint_shape(topx+160,topy+30,shapes.get_shape(0x0D,0));
			if(i>20) {
				gwin->paint_shape(i, centery-i/5, shapes.get_shape(0x0E, i%4));
			}
			win->show();
			if(wait_delay(50)) {
				pal.fade_out(30);
				return;	
			}
		}
		for(int i=1; i<13; i++) {
			gwin->paint_shape(topx,topy,shapes.get_shape(0x12,0));
			gwin->paint_shape(topx+160,topy+30,shapes.get_shape(0x0D,0));
			gwin->paint_shape(270, centery-54, shapes.get_shape(0x0E, i%4));
			win->show();
			if(wait_delay(50*i)) {
				pal.fade_out(30);
				return;	
			}
		}
		if(wait_delay(2000)) {
			pal.fade_out(30);
			return;	
		}

		// The main man :)
		play_midi(2);
		pal.load("static/intropal.dat",2);
		pal.apply();
		// First
		for(int i=9; i>0; i--) {
			clear_screen();
			gwin->paint_shape(centerx,centery-45,shapes.get_shape(0x21,i));
			win->show();
			if(wait_delay(70)) {
				pal.fade_out(30);
				return;	
			}
		}
		for(int i=1; i<10; i++) {
			clear_screen();
			gwin->paint_shape(centerx,centery-45,shapes.get_shape(0x21,i));
			win->show();
			if(wait_delay(70)) {
				pal.fade_out(30);
				return;	
			}
		}
		// Second 
		for(int i=0; i<10; i++) {
			clear_screen();
			gwin->paint_shape(centerx,centery-45,shapes.get_shape(0x22,i));
			win->show();
			if(wait_delay(70)) {
				pal.fade_out(30);
				return;	
			}
		}
		for(int i=9; i>=0; i--) {
			clear_screen();
			gwin->paint_shape(centerx,centery-45,shapes.get_shape(0x22,i));
			win->show();
			if(wait_delay(70)) {
				pal.fade_out(30);
				return;	
			}
		}
		for(int i=0; i<16; i++) {
			clear_screen();
			gwin->paint_shape(centerx,centery-20,shapes.get_shape(0x23,i));
			win->show();
			if(wait_delay(70)) {
				pal.fade_out(30);
				return;	
			}
		}
		gwin->paint_shape(centerx,centery-30,shapes.get_shape(0x20,1));
		win->show();
		
		U7object textobj(MAINSHP_FLX, 0x0D);
		char * txt, *txt_ptr;
		size_t txt_len;
		textobj.retrieve(&txt,txt_len);
		txt_ptr = txt;
		// Guardian speech
		audio->playfile(INTROSND,false);
		int txt_ypos = gwin->get_height()-gwin->get_text_height(0);
		for(int i=0; i<14*40; i++) {
			gwin->paint_shape(centerx,centery-30,shapes.get_shape(0x20,i % 10));
			gwin->paint_shape(centerx,centery-20,shapes.get_shape(0x1E,i % 15));
			if(i % 40 ==0) {
				char *txt_end = strchr(txt_ptr, '\r');
				*txt_end = 0;
				win->fill8(0,gwin->get_width(),txt_ypos,0,txt_ypos);
				gwin->paint_text(0, txt_ptr, centerx-gwin->get_text_width(0, txt_ptr)/2, txt_ypos);
				txt_ptr = txt_end+2;
			}
			win->show();
			if(wait_delay(50)) {
				pal.fade_out(30);
				return;	
			}
		}
		delete [] txt;
		for(int i=15; i>=0; i--) {
			clear_screen();
			gwin->paint_shape(centerx,centery-20,shapes.get_shape(0x23,i));
			win->show();
			if(wait_delay(70)) {
				pal.fade_out(30);
				return;	
			}
		}

		// PC screen
		play_midi(1);
		
		pal.load("static/intropal.dat",1);
		pal.apply();

		for(int i=0;i<194;i+=2) {
			gwin->paint_shape(centerx-i, centery, shapes.get_shape(0x07,0));
			gwin->paint_shape(centerx-i, centery, shapes.get_shape(0x09,0));
			gwin->paint_shape(centerx-i, centery, shapes.get_shape(0x08,0));
			gwin->paint_shape(centerx-i, centery, shapes.get_shape(0x0A,0));
			gwin->paint_shape(centerx-i+12, centery-22, shapes.get_shape(0x1D,0));
			gwin->paint_shape(topx+320-i, topy, shapes.get_shape(0x06,0));
			if(i<75)
				gwin->paint_shape(centerx, centery+50, shapes.get_shape(0x15,0));
			else
				gwin->paint_shape(centerx, centery+50, shapes.get_shape(0x16,0));
			win->show();
			if(wait_delay(50)) {
				pal.fade_out(30);
				return;	
			}
		}
		for(int i=0;i<50;i++) {
			gwin->paint_shape(centerx-194, centery-i, shapes.get_shape(0x07,0));
			gwin->paint_shape(centerx-194, centery-i, shapes.get_shape(0x09,0));
			gwin->paint_shape(centerx-194, centery-i, shapes.get_shape(0x08,0));
			gwin->paint_shape(centerx-194, centery-i, shapes.get_shape(0x0A,0));
			gwin->paint_shape(centerx-194+12, centery-22-i, shapes.get_shape(0x1D,0));
			gwin->paint_shape(topx+320-194, topy-i, shapes.get_shape(0x06,0));
			gwin->paint_shape(topx+320, topy+200-i, shapes.get_shape(0x0B,0));
			if(i>48) {
				gwin->paint_shape(centerx, topy, shapes.get_shape(0x18,0));
			} else if(i>15)
				gwin->paint_shape(centerx, topy, shapes.get_shape(0x17,0));
			win->show();
			if(wait_delay(50)) {
				pal.fade_out(30);
				return;	
			}
		}
		if(wait_delay(2000)) {
			pal.fade_out(30);
			return;	
		}

		clear_screen();

		// The Moongate
		pal.load("static/intropal.dat",5);
		pal.apply();
		for(int i=120;i>=0;i-=2) {
			gwin->paint_shape(centerx, centery, shapes.get_shape(0x02,0));
			gwin->paint_shape(centerx, centery, shapes.get_shape(0x03,0));
			gwin->paint_shape(centerx, centery, shapes.get_shape(0x04,0));
			gwin->paint_shape(centerx, centery, shapes.get_shape(0x05,0));

			gwin->paint_shape(centerx+i, topy, shapes.get_shape(0x00,0));
			gwin->paint_shape(centerx-i, topy, shapes.get_shape(0x01,0));
			if(i>60)
				gwin->paint_shape(centerx, centery+50, shapes.get_shape(0x19,0));
			else
				gwin->paint_shape(centerx, centery+50, shapes.get_shape(0x1A,0));
			win->show();
			if(wait_delay(50)) {
				pal.fade_out(30);
				return;	
			}
		}
		wait_delay(2000);
		clear_screen();
	}
	
void Titles::clear_screen()
	{
		win->fill8(0,gwin->get_width(),gwin->get_height(),0,0);
	}

void Titles::show_menu()
	{
		Vga_file menushapes(MAINSHP_FLX);
		Palette pal;

		int topx = (gwin->get_width()-320)/2;
		int topy = (gwin->get_height()-200)/2;
		int centerx = gwin->get_width()/2;
		
		play_midi(3);
		
		gwin->paint_shape(topx,topy,menushapes.get_shape(0x2,0));
		pal.load("static/intropal.dat",0);
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

void Titles::play_flic(const char *archive, int index) 
	{
		U7object flic(archive, index);
		flic.retrieve("flic.fli");
		playfli fli("flic.fli");
		fli.play(win);
	}

void Titles::play_audio(const char *archive, int index) 
	{
		U7object speech(archive, index);
		speech.retrieve("speech.voc");
		audio->playfile("speech.voc", false);
	}

void Titles::play_midi(int track)
	{
		audio->start_music(track,0,1);
	}

void Titles::end_game(bool success) 
	{
		// Clear screen
		clear_screen();
		// Start endgame music.
		// It should actually play endscore.xmi, but we can't
		// handle XMIs yet... :-(
		play_midi(40);
		play_flic(ENDGAME,0);
		play_audio(ENDGAME,7);
		play_flic(ENDGAME,1);
		play_audio(ENDGAME,8);
		play_flic(ENDGAME,2);
		play_audio(ENDGAME,9);
	}

void Titles::show_quotes()
	{
	}

void Titles::show_credits()
	{
		U7object mainshp("static/mainshp.flx", 0x0E);
		size_t len;
		
		char * credits, *ptr, *end;
		mainshp.retrieve(&credits, len);
		ptr = credits;
		end = ptr+len;
		int lines = 0;
		while(ptr<end) {
			ptr = strchr(ptr, '\r');
			ptr += 2;
			++lines;
		}
		printf("Credits lines: %d\n", lines);
#if defined(MACOS)
		char **text = new char*[lines] ;
#else
		char **text = new (char*)[lines];
#endif
		ptr = credits;
		while(ptr<end) {
			text[lines] = ptr;
			ptr = strchr(ptr, '\r');
			*ptr = 0;
			ptr += 2;
			++lines;
		}
		for(int i=0; i<lines; i++)
			printf("%d - %s\n", i, text[i]);
		delete [] text;
		delete [] credits;
	}
