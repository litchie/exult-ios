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

#include "SDL_events.h"

#include <typeinfo>
#include "files/U7file.h"
#include "files/utils.h"
#include "flic/playfli.h"
#include "gamewin.h"
#include "Audio.h"
#include "bggame.h"
#include "sigame.h"
#include "palette.h"
#include "databuf.h"
#include "font.h"
#include "txtscroll.h"
#include "data/exult_bg_flx.h"
#include "exult.h"
#include "Configuration.h"
#include "shapeid.h"

#ifndef ALPHA_LINUX_CXX
#  include <cctype>
#  include <cstring>
#endif

using std::abs;
using std::rand;
using std::strchr;
using std::strlen;
using std::toupper;
using std::snprintf;

enum
{
	ultima_text_shp = 0x0D,
	butterfly_shp = 0x0E,
	lord_british_shp = 0x11,
	trees_shp = 0x12,
	
	guardian_mouth_shp = 0x1E,
	guardian_forehead_shp = 0x1F,
	guardian_eyes_shp = 0x20
};

enum
{
	bird_song_midi = 0,
	home_song_midi = 1,
	guardian_midi = 2,
	menu_midi = 3,
	credits_midi = 4,
	quotes_midi = 5
};

BG_Game::BG_Game()
	: shapes(ENDSHAPE_FLX)
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

	add_shape("sprites/map", 22);
	add_shape("sprites/cheatmap", 12);

	add_resource("files/shapes/count", 0, 9);
	add_resource("files/shapes/0", "<STATIC>/shapes.vga", 0);
	add_resource("files/shapes/1", "<STATIC>/faces.vga", 0);
	add_resource("files/shapes/2", "<STATIC>/gumps.vga", 0);
	add_resource("files/shapes/3", "<STATIC>/sprites.vga", 0);
	add_resource("files/shapes/4", MAINSHP_FLX, 0);
	add_resource("files/shapes/5", "<STATIC>/endshape.flx", 0);
	add_resource("files/shapes/6", "<STATIC>/fonts.vga", 0);
	add_resource("files/shapes/7", "<DATA>/exult.flx", 0);
	add_resource("files/shapes/8", "<DATA>/exult_bg.flx", 0);

	add_resource("files/gameflx", "<DATA>/exult_bg.flx", 0);

	add_resource("config/defaultkeys", "<DATA>/exult_bg.flx", 13);

	add_resource("palettes/count", 0, 18);
	add_resource("palettes/0", PALETTES_FLX, 0);
	add_resource("palettes/1", PALETTES_FLX, 1);
	add_resource("palettes/2", PALETTES_FLX, 2);
	add_resource("palettes/3", PALETTES_FLX, 3);
	add_resource("palettes/4", PALETTES_FLX, 4);
	add_resource("palettes/5", PALETTES_FLX, 5);
	add_resource("palettes/6", PALETTES_FLX, 6);
	add_resource("palettes/7", PALETTES_FLX, 7);
	add_resource("palettes/8", PALETTES_FLX, 8);
	add_resource("palettes/9", PALETTES_FLX, 10);
	add_resource("palettes/10", PALETTES_FLX, 11);
	add_resource("palettes/11", PALETTES_FLX, 12);
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
	
	fontManager.add_font("MENU_FONT", MAINSHP_FLX, 9, 1);
	fontManager.add_font("END2_FONT", ENDGAME, 4, -1);
	fontManager.add_font("END3_FONT", ENDGAME, 5, -2);
	fontManager.add_font("NORMAL_FONT", FONTS_VGA, 0, -1);
}

BG_Game::~BG_Game()
{
}

class UserSkipException : public UserBreakException
{
};


#define WAITDELAY(x) switch(wait_delay(x)) { \
			case 1: throw UserBreakException(); break; \
			case 2: throw UserSkipException(); break; \
			}

#define WAITDELAYCYCLE(x) switch (wait_delay((x), 16, 78)) { \
			case 1: throw UserBreakException(); break; \
			case 2: throw UserSkipException(); break; \
			}

#define WAITDELAYCYCLE2(x) switch (wait_delay((x), 250, 5)) { \
			case 1: throw UserBreakException(); break; \
			case 2: throw UserSkipException(); break; \
			}

#define WAITDELAYCYCLE3(x) switch (wait_delay((x), 240, 15)) { \
			case 1: throw UserBreakException(); break; \
			case 2: throw UserSkipException(); break; \
			}

void BG_Game::play_intro()
{
	gwin->clear_screen(true);

	Audio::get_ptr()->stop_music();

	// TODO: check/fix other resolutions

	// these backups store the area under the guardian shapes being drawn
	// the cbackups are 'clean' backups, ie. just background
	// the others may have other shapes on them, if those are static
	Image_buffer *backup, *backup2, *backup3;
	Image_buffer *cbackup, *cbackup2, *cbackup3;

	backup = backup2 = backup3 = 0;
	cbackup = cbackup2 = cbackup3 = 0;

	try
	{
		/********************************************************************
		 Lord British Presents
		********************************************************************/
		
		scene_lord_british();

		/********************************************************************
		 Ultima VII logo w/Trees
		********************************************************************/

		scene_butterfly();

		/********************************************************************
		 Enter guardian
		 TODO: reduce sudden facial movements in speech
		********************************************************************/
		
		scene_guardian();

		// TODO: transition (zoom out to PC) scene missing

		/********************************************************************
		 PC screen
		********************************************************************/

		scene_desk();

		/********************************************************************
		 The Moongate
		********************************************************************/
		
		scene_moongate();
	}
	catch(const UserBreakException &x)
	{
		// Waste disposal
		FORGET_OBJECT(backup); FORGET_OBJECT(backup2); FORGET_OBJECT(backup3);
		FORGET_OBJECT(cbackup); FORGET_OBJECT(cbackup2); FORGET_OBJECT(cbackup3);
	}
	
	// Fade out the palette...
	pal.fade_out(c_fade_out_time);
	
	// ... and clean the screen.
	gwin->clear_screen(true);
	
	// Stop all audio output
	Audio::get_ptr()->cancel_streams();
}

void BG_Game::scene_lord_british()
{
	Font *font = fontManager.get_font("END2_FONT");

	const char *txt_msg[] = { "with help from",
			"The Exult Team"};

	// Lord British presents...  (sh. 0x11)
	pal.load("<STATIC>/intropal.dat",3);
	gwin->paint_shape(topx,topy,shapes.get_shape(lord_british_shp,0));
	
	// insert our own intro text
	font->center_text(ibuf, centerx, centery+50, txt_msg[0]);
	font->center_text(ibuf, centerx, centery+65, txt_msg[1]);

	pal.fade_in(c_fade_in_time);
	if(1 == wait_delay(2000))
		throw UserBreakException();
	pal.fade_out(c_fade_out_time);
	gwin->clear_screen(true);
}


#define	BUTTERFLY_FRAME_DURATION	16

#define	BUTTERFLY_SUB_FRAMES	3

#define	BUTTERFLY(x,y,frame,delay)	do {	\
		win->get(backup, topx + (x) - butterfly->get_xleft(),	\
				topy + (y) - butterfly->get_yabove());	\
		gwin->paint_shape(topx + x, topy + y, shapes.get_shape(butterfly_shp, frame));	\
		win->show();	\
		WAITDELAY(delay);	\
		win->put(backup, topx + (x) - butterfly->get_xleft(),	\
				topy + (y) - butterfly->get_yabove());	\
		} while(0)

#define	BUTTERFLY_FLAP()	do {	\
			if ((rand() % 5)<4) {	\
				if (frame == 3)	\
					dir = -1;	\
				else if (frame == 0)	\
					dir = +1;	\
				frame += dir;	\
			} } while(0)

static int butterfly_x[] =
{
	6,18,30,41,52,62,70,78,86,95,
	104,113,122,132,139,146,151,155,157,158,
	157,155,151,146,139,132,124,116,108,102,
	96,93,93,93,95,99,109,111,118,125,
	132,140,148,157,164,171,178,184,190,196,
	203,211,219,228,237,246,254,259,262,264,
	265,265,263,260,256,251,245,239,232,226,
	219,212,208,206,206,209,212,216,220,224,
	227,234,231,232,233,233,233,233,234,236,
	239,243,247,250,258,265
};

static int butterfly_y[] =
{
	155,153,151,150,149,148,148,148,148,149,
	150,150,150,149,147,142,137,131,125,118,
	110,103,98,94,92,91,91,91,92,95,
	99,104,110,117,123,127,131,134,135,135,
	135,135,135,134,132,129,127,123,119,115,
	112,109,104,102,101,102,109,109,114,119,
	125,131,138,144,149,152,156,158,159,159,
	158,155,150,144,137,130,124,118,112,105,
	99,93,86,80,73,66,59,53,47,42,
	38,35,32,29,26,25
};

static const int butterfly_num_coords = sizeof(butterfly_x)/sizeof(int);

static int butterfly_end_frames[] = { 3, 4, 3, 4, 3, 2, 1, 0 };
static int butterfly_end_delay[] = { 167, 416, 250, 416, 416, 416, 416, 333 };


void BG_Game::scene_butterfly()
{
	Font *font = fontManager.get_font("END2_FONT");
	Image_buffer *backup = 0;
	Shape_frame *butterfly = 0;
	const char *txt_msg = "Driven by Exult";
	int	i, j, frame, dir;
	
	try
	{
		pal.load("<STATIC>/intropal.dat",4);

		// Load the butterfly shape
		butterfly = shapes.get_shape(butterfly_shp,0);
		backup = win->create_buffer(butterfly->get_width(), butterfly->get_height());
		
		// Start playing the birdsongs while still faded out
		play_midi(bird_song_midi);

		// trees with "Ultima VII" on top of 'em
		gwin->paint_shape(topx,topy,shapes.get_shape(trees_shp,0));
		gwin->paint_shape(topx+160,topy+50,shapes.get_shape(ultima_text_shp,0));
		
		// again display our own text
		font->center_text(ibuf, centerx, centery+50, txt_msg);

		// Keep it dark for some more time, playing the music 
		WAITDELAY(3500);

		// Finally fade in
		pal.fade_in(c_fade_in_time);
		WAITDELAY(12500);

		// clear 'Exult' text
		gwin->paint_shape(topx,topy,shapes.get_shape(trees_shp,0));
		gwin->paint_shape(topx+160,topy+50,shapes.get_shape(ultima_text_shp,0));
		win->show();

		//
		// Move the butterfly along its path
		//
		frame = 0;
		Sint32 delay = BUTTERFLY_FRAME_DURATION;
		Sint32 ticks = SDL_GetTicks();
		for(i=0; i < butterfly_num_coords-1; ++i)
		{
			for(j=0; j < BUTTERFLY_SUB_FRAMES; ++j)
			{

				ticks = SDL_GetTicks();
				int x = butterfly_x[i] + j*(butterfly_x[i+1] - butterfly_x[i])/BUTTERFLY_SUB_FRAMES;
				int y = butterfly_y[i] + j*(butterfly_y[i+1] - butterfly_y[i])/BUTTERFLY_SUB_FRAMES;
				BUTTERFLY(x, y, frame, delay);

				// Flap the wings; but not always, so that the butterfly "glides" from time to time
				BUTTERFLY_FLAP();

				// Calculate the difference between the time we wanted to spent and the time
				// we actually spent; then adjust 'delay' accordingly
				ticks = SDL_GetTicks() - ticks;
				delay = (delay + (2*BUTTERFLY_FRAME_DURATION - ticks)) / 2;
				
				// ... but maybe we also have to skip frames..
				if( delay < 0 )
				{
					// Calculate how many frames we should skip
					int frames_to_skip = (-delay) / BUTTERFLY_FRAME_DURATION + 1;
					int new_index = i*BUTTERFLY_SUB_FRAMES + j + frames_to_skip;
					i = new_index / BUTTERFLY_SUB_FRAMES;
					j = new_index % BUTTERFLY_SUB_FRAMES;
					
					// Did we skip over the end?
					if ( i >= butterfly_num_coords-1 )
						break;

					while(frames_to_skip--)
						BUTTERFLY_FLAP();

					delay = 0;
				}
			}
		}

		// Finally, let it flutter a bit on the end spot
		for(i=0; i<8; i++) {
			BUTTERFLY(butterfly_x[butterfly_num_coords-1],
						butterfly_y[butterfly_num_coords-1],
						butterfly_end_frames[i],
						butterfly_end_delay[i]);
		}
		
		WAITDELAY(2000);
		
		// Wait till the music finished playing
		while(Audio::get_ptr()->is_track_playing(bird_song_midi))
			WAITDELAY(20);
	}
	catch(const UserSkipException &x)
	{
	}
}

#define	FLASH_SHAPE(x,y,shape,frame, delay)	do {	\
		gwin->paint_shape(x,y,shapes.get_shape(shape,frame));	\
		win->show();	\
		WAITDELAYCYCLE(delay);	\
		win->put(backup,(x)-s->get_xleft(),(y)-s->get_yabove());	\
		} while(0)

#define	EYES_DIST		12
#define	FORHEAD_DIST	49

void BG_Game::scene_guardian()
{
	Image_buffer *backup = 0, *backup2 = 0, *backup3 = 0;
	Image_buffer *cbackup = 0, *cbackup2 = 0, *cbackup3 = 0;
	Image_buffer *plasma;
	char *txt = 0;

	try
	{
		char *txt_ptr, *txt_end, *next_txt;
		Shape_frame *s, *s2, *s3;
		Uint32 ticks;
		int i;

		// Start background music
		play_midi(guardian_midi);
		
		// create buffer containing a blue 'plasma' screen
		plasma = win->create_buffer(gwin->get_width(),
							gwin->get_height());
		gwin->plasma(gwin->get_width(), gwin->get_height(), 0, 0, 16, 16+76);
		win->get(plasma, 0, 0);

		pal.load("<STATIC>/intropal.dat",2);
		pal.set_color(1,0,0,0); //UGLY hack... set font background to black
		pal.apply();

		//play static SFX
		Audio::get_ptr()->play_sound_effect(115, MIX_MAX_VOLUME, 0, 0);

		//TODO: timing?
		win->show();
		WAITDELAYCYCLE(100);
		
		//
		// Show some "static" alternating with the blue plasma
		//
		ticks = SDL_GetTicks();
		while(1)
		{
			win->get_ibuf()->fill_static(0, 7, 15);
			win->show();
			WAITDELAY(0);
			if (SDL_GetTicks() > ticks + 400)
				break;
		}

		win->put(plasma,0,0); win->show();
		WAITDELAYCYCLE(100);

		ticks = SDL_GetTicks();
		while(1)
		{
			win->get_ibuf()->fill_static(0, 7, 15);
			win->show();
			WAITDELAY(0);
			if (SDL_GetTicks() > ticks + 200)
				break;
		}

		win->put(plasma,0,0); win->show();
		WAITDELAYCYCLE(200);

		ticks = SDL_GetTicks();
		while(1)
		{
			win->get_ibuf()->fill_static(0, 7, 15);
			win->show();
			WAITDELAY(0);
			if (SDL_GetTicks() > ticks + 100)
				break;
		}
		
		win->put(plasma,0,0); win->show();

		FORGET_OBJECT(plasma);

		
		//
		// First 'popup' (sh. 0x21)
		//
		s = shapes.get_shape(0x21, 0);
		backup = win->create_buffer(s->get_width(), s->get_height());
		win->get(backup, centerx-53-s->get_xleft(), centery-68-s->get_yabove());
		for(i=8; i>=-8; i--)
		{
			FLASH_SHAPE(centerx-53, centery-68, 0x21, 1+abs(i),70);
		}
		FORGET_OBJECT(backup);
		WAITDELAYCYCLE(800);


		//
		// Second 'popup' (sh. 0x22)
		//
		s = shapes.get_shape(0x22, 0);
		backup = win->create_buffer(s->get_width(), s->get_height());
		win->get(backup, centerx - s->get_xleft(), centery-45 - s->get_yabove());
		for(i=9; i>=-9; i--)
		{
			FLASH_SHAPE(centerx, centery-45, 0x22, 9-abs(i),70);
		}
		FORGET_OBJECT(backup);
		WAITDELAYCYCLE(800);


		//
		// Successful 'popup' (sh. 0x23)
		//
		s = shapes.get_shape(0x23, 0);
		backup = win->create_buffer(s->get_width(), s->get_height());
		cbackup = win->create_buffer(s->get_width(), s->get_height());

		win->get(cbackup, centerx - s->get_xleft(), centery - s->get_yabove());
		gwin->paint_shape(centerx,centery,s); // frame 0 is static background
		win->get(backup, centerx- s->get_xleft(), centery- s->get_yabove());
		for(i=1; i<16; i++)
		{
			FLASH_SHAPE(centerx, centery, 0x23, i,70);
		}
		
		win->put(cbackup, centerx - s->get_xleft(), centery - s->get_yabove());
		FORGET_OBJECT(backup);
		FORGET_OBJECT(cbackup);


		//
		// Actual appearance
		//


		// mouth
		s = shapes.get_shape(guardian_mouth_shp,0);
		backup = win->create_buffer(s->get_width(), s->get_height());
		cbackup = win->create_buffer(s->get_width(), s->get_height());
		win->get(cbackup, centerx - s->get_xleft(), centery - s->get_yabove());
		gwin->paint_shape(centerx,centery,s); // frame 0 is background
		win->get(backup, centerx - s->get_xleft(), centery - s->get_yabove());
		// eyes
		s2 = shapes.get_shape(guardian_eyes_shp,0);
		backup2 = win->create_buffer(s2->get_width(), s2->get_height());
		cbackup2 = win->create_buffer(s2->get_width(), s2->get_height());
		win->get(cbackup2, centerx - s2->get_xleft(),
			 centery-EYES_DIST - s2->get_yabove());
		gwin->paint_shape(centerx,centery-EYES_DIST,s2); // frame 0 is background
		win->get(backup2, centerx - s2->get_xleft(),
			 centery-EYES_DIST - s2->get_yabove());
		// forehead
		s3 = shapes.get_shape(guardian_forehead_shp,0);
		cbackup3 = win->create_buffer(s3->get_width(), s3->get_height());
		win->get(cbackup3, centerx - s3->get_xleft(),
			 centery-FORHEAD_DIST - s3->get_yabove());
	       	gwin->paint_shape(centerx,centery-FORHEAD_DIST,s3); // forehead isn't animated

		// prepare Guardian speech
		Font *font = fontManager.get_font("END3_FONT");
		U7object textobj(MAINSHP_FLX, 0x0D);
		size_t txt_len;
		next_txt = txt_ptr = txt = textobj.retrieve(txt_len);
		if (Audio::get_ptr()->is_speech_enabled())
			Audio::get_ptr()->playfile(INTROSND,false);
		int txt_height = font->get_text_height();
		int txt_ypos = gwin->get_height()-txt_height-16;

		// backup text area
		backup3 = win->create_buffer(gwin->get_width(),txt_height);
		win->get(backup3, 0, txt_ypos);

		int speech_delay[] = { 29, 71, 59, 47, 47, 62, 79, 
				       55, 90, 104, 80, 65, 55, 120 };
		// start speech
		for(int speech_item=0; speech_item<14; speech_item++)
		{
			unsigned long ticks = SDL_GetTicks();
			unsigned long end_ticks = ticks+speech_delay[speech_item]*50;
			i=0;
			do
			{
				// convoluted mess to get eye movement acceptable
				int eye_frame = 1 + 3*((i/12) % 4) + ((i%50>47&&(i/12)%4!=3)?i%50-47:0);
				gwin->paint_shape(centerx,centery-EYES_DIST,
						shapes.get_shape(guardian_eyes_shp, eye_frame));
		
				gwin->paint_shape(centerx,centery,
						  shapes.get_shape(guardian_mouth_shp,1 + i % 13));
		
				if (i == 0)
				{
					txt_ptr = next_txt;
					txt_end = strchr(txt_ptr, '\r');
					*txt_end = '\0';
					next_txt = txt_end+2;
				}

				font->center_text(win->get_ib8(), centerx, txt_ypos, txt_ptr);
		
				win->show();
				WAITDELAYCYCLE(50);
		
				win->put(backup3, 0, txt_ypos);
				win->put(backup, centerx - s->get_xleft(),
					     centery - s->get_yabove());
				win->put(backup2, centerx - s2->get_xleft(),
					     centery-EYES_DIST - s2->get_yabove());
				ticks = SDL_GetTicks();
				++i;
			} while (ticks<end_ticks);
		}

		win->put(backup3, 0, txt_ypos);
		win->put(cbackup, centerx - s->get_xleft(), centery - s->get_yabove());
		win->put(cbackup2, centerx - s2->get_xleft(), 
			 centery-EYES_DIST - s2->get_yabove());
		win->put(cbackup3, centerx - s3->get_xleft(),
			 centery-FORHEAD_DIST - s3->get_yabove());
		
		FORGET_ARRAY(txt);
		FORGET_OBJECT(backup);
		FORGET_OBJECT(backup2);
		FORGET_OBJECT(backup3);
		FORGET_OBJECT(cbackup);
		FORGET_OBJECT(cbackup2);
		FORGET_OBJECT(cbackup3);

		// G. disappears again (sp. 0x23 again)
		s = shapes.get_shape(0x23, 0);
		backup = win->create_buffer(s->get_width(), s->get_height());
		cbackup = win->create_buffer(s->get_width(), s->get_height());
		win->get(cbackup, centerx- s->get_xleft(), centery- s->get_yabove());
		gwin->paint_shape(centerx,centery,s); // frame 0 is background
		win->get(backup, centerx - s->get_xleft(), centery - s->get_yabove());
		for(i=15; i>0; i--)
		{
			FLASH_SHAPE(centerx, centery, 0x23, i, 70);
		}
		win->put(cbackup, centerx- s->get_xleft(), centery- s->get_yabove());
		FORGET_OBJECT(backup);
		FORGET_OBJECT(cbackup);

		win->show();
		WAITDELAYCYCLE(1000);
		
		Audio::get_ptr()->stop_music();
		
		//
		// White dot
		//
		s = shapes.get_shape(0x14, 0);
		backup = win->create_buffer(s->get_width()+2, s->get_height()+2);
		win->get(backup, centerx - 1, centery - 1);
		ticks = SDL_GetTicks();
		for(i = 0; i < 100; i++)
		{
			int x = centerx + rand()%3 - 1;
			int y = centery + rand()%3 - 1;
			FLASH_SHAPE(x, y, 0x14, 0, 0);
			if (SDL_GetTicks() - ticks > 400)
				break;
		}
		FORGET_OBJECT(backup);
	}
	catch(const UserBreakException &x)
	{
		// Waste disposal
		FORGET_ARRAY(txt);
		FORGET_OBJECT(backup); FORGET_OBJECT(backup2); FORGET_OBJECT(backup3);
		FORGET_OBJECT(cbackup); FORGET_OBJECT(cbackup2); FORGET_OBJECT(cbackup3);
		FORGET_OBJECT(plasma);
		
		if(typeid(x) != typeid(UserSkipException))
			throw x;
	}
}

void BG_Game::scene_desk()
{
	Shape_frame *s;
	Image_buffer *backup = 0;
	int i;

	try
	{
		play_midi(home_song_midi);
		
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
		for (i=0; i<9; i++) {
			win->get(backup, centerx-96-30*abs(i%4-2) - s->get_xleft(),
				 centery+100 - s->get_yabove());
			gwin->paint_shape(centerx-96-30*abs(i%4-2), centery+100, s);
			win->show();
			win->put(backup, centerx-96-30*abs(i%4-2) - s->get_xleft(),
				 centery+100 - s->get_yabove());
			WAITDELAY(1); //just to catch events
		}

		// screen comes back up (sh. 0x1D)
		gwin->paint_shape(centerx+12, centery-22, shapes.get_shape(0x1D,0));
		win->show();
		FORGET_OBJECT(backup);

		// "Something is obviously amiss"
		gwin->paint_shape(centerx, centery+50, shapes.get_shape(0x15,0));
		win->show();
		WAITDELAY(4000);

		// TODO: misaligned?

		// scroll right (sh. 0x06: map to the right of the monitor)
		for(i=0;i<194;i+=4) {
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
		for(i=0;i<=50;i+=2) {
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
	}
	catch(const UserBreakException &x)
	{
		// Waste disposal
		FORGET_OBJECT(backup);
		
		throw x;
	}
}

void BG_Game::scene_moongate()
{
	// sh. 0x02, 0x03, 0x04, 0x05: various parts of moongate
	// sh. 0x00, 0x01: parting trees before clearing

	int i;
		
	gwin->clear_screen();
	pal.load("<STATIC>/intropal.dat",5);
	pal.apply();

	// "Behind your house is the circle of stones"
	gwin->paint_shape(centerx, centery+50, shapes.get_shape(0x19,0));
	win->show();

	// TODO: fade in screen while text is onscreen

	WAITDELAY(3000);

	// TODO: misaligned?
	for(i=120;i>=-170;i-=6) {
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

	// TODO: zoom (run) into moongate

	WAITDELAYCYCLE3(3000);

	// Wait till the music finished playing
	while(Audio::get_ptr()->is_track_playing(home_song_midi))
		WAITDELAYCYCLE3(50);
}

void BG_Game::top_menu()
{
	play_midi(menu_midi, true);
		
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
		TextScroller text(MAINSHP_FLX, 0x15,
				  font,0);
		gwin->clear_screen();
		pal.load("<STATIC>/intropal.dat",0);
		for(uint32 i=0; i<text.get_count(); i++) {
			text.show_line(gwin, topx, topx+320, topy+20+i*12, i);
		}
		
		pal.fade_in(c_fade_in_time);
		wait_delay(10000);
		pal.fade_out(c_fade_out_time);
		
		gwin->clear_screen();
		font->center_text(ibuf, centerx, centery-10, "The end of Ultima VII");
		pal.fade_in(c_fade_in_time);
		wait_delay(4000);
		pal.fade_out(c_fade_out_time);
		
		gwin->clear_screen();
		font->center_text(ibuf, centerx, centery-10, "The end of Britannia as you know it!");
		pal.fade_in(c_fade_in_time);
		wait_delay(4000);
		pal.fade_out(c_fade_out_time);
		gwin->clear_screen(true);
		return;
	}

	// Audio buffer
	size_t	size;
	uint8	*buffer;
	
	// Fli Buffers
	size_t	flisize;
	char	*fli_b[3];

	// Clear screen
	gwin->clear_screen(true);

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
		if (wait_delay (0))
		{
			gwin->clear_screen(true);
			FORGET_ARRAY(buffer);
			FORGET_ARRAY(fli_b[0]);
			FORGET_ARRAY(fli_b[1]);
			FORGET_ARRAY(fli_b[2]);
			return;
		}
	}
	
	for (i = 1; i < 150; i++)
	{
		next = fli1.play(win, i, i+1, next);
		if (wait_delay (0))
		{
			gwin->clear_screen(true);
			FORGET_ARRAY(buffer);
			FORGET_ARRAY(fli_b[0]);
			FORGET_ARRAY(fli_b[1]);
			FORGET_ARRAY(fli_b[2]);
			return;
		}
	}

	Audio::get_ptr()->play (buffer+8, size-8, false);
	FORGET_ARRAY(buffer);
	Font *endfont2 = fontManager.get_font("END2_FONT");
	Font *endfont3 = fontManager.get_font("END3_FONT");
	Font *normal = fontManager.get_font("NORMAL_FONT");

	const char 	*message = "No. You cannot do that! You must not!";
	int	height = topy+200 - endfont2->get_text_height()*2;
	int	width = (gwin->get_width() - endfont2->get_text_width(message)) / 2;

	for (i = 150; i < 204; i++)
	{
		next = fli1.play(win, i, i, next);
		endfont2->draw_text(ibuf, width, height, message);
		
		win->show();
		if (wait_delay (0))
		{
			gwin->clear_screen(true);
			FORGET_ARRAY(fli_b[0]);
			FORGET_ARRAY(fli_b[1]);
			FORGET_ARRAY(fli_b[2]);
			return;
		}
	}

	// Set new music
	Audio::get_ptr()->start_music(ENDSCORE_XMI,2,false);
	
	// Set speech
	
	buffer = (uint8 *) speech2.retrieve(size);
	Audio::get_ptr()->play (buffer+8, size-8, false);
	FORGET_ARRAY(buffer);

	message = "Damn you Avatar!  Damn you!";
	width = (gwin->get_width() - endfont2->get_text_width(message)) / 2;

	for (i = 0; i < 100; i++)
	{
		next = fli2.play(win, i, i, next);
		endfont2->draw_text(ibuf, width, height, message);
		
		win->show();
		if (wait_delay (0))
		{
			gwin->clear_screen(true);
			FORGET_ARRAY(fli_b[0]);
			FORGET_ARRAY(fli_b[1]);
			FORGET_ARRAY(fli_b[2]);
			return;
		}
	}


	Palette *pal = fli2.get_palette();
	next = SDL_GetTicks();
	for (i = 1000 + next; next < i; next += 10)
	{
		// Speed related frame skipping detection
		int skip_frame = Game_window::get_instance()->get_frame_skipping() && SDL_GetTicks() >= next;
		while (SDL_GetTicks() < next)
			;
		if (!skip_frame)
		{
			pal->set_brightness ((i - next) / 10);
			pal->apply();
		}
		if (wait_delay (0))
		{
			gwin->clear_screen(true);
			FORGET_ARRAY(fli_b[0]);
			FORGET_ARRAY(fli_b[1]);
			FORGET_ARRAY(fli_b[2]);
			return;
		}
	}

	// Text message 1

	// Paint backgound black
	win->fill8(0,gwin->get_width(),gwin->get_height(),0,0);

	// Paint text
	message = "The Black Gate is destroyed.";
	width = (gwin->get_width() - normal->get_text_width(message)) / 2;
	height = (gwin->get_height() - normal->get_text_height()) / 2;
	
	normal->draw_text (ibuf, width, height, message);

	Palette *gpal = gwin->get_pal();
	// Fade in for 1 sec (50 cycles)
	gpal->fade (50, 1, 0);

	// Display text for 3 seconds
	for (i = 0; i < 30; i++)
	{
		if (wait_delay (100))
		{
			gwin->clear_screen(true);
			FORGET_ARRAY(fli_b[0]);
			FORGET_ARRAY(fli_b[1]);
			FORGET_ARRAY(fli_b[2]);
			return;
		}
	}

	// Fade out for 1 sec (50 cycles)
	gpal->fade (50, 0, 0);

	
	// Now the second text message


	// Paint backgound black
	win->fill8(0,gwin->get_width(),gwin->get_height(),0,0);

	// Paint text
	message = "The Guardian has been stopped.";
	width = (gwin->get_width() - normal->get_text_width(message)) / 2;

	normal->draw_text (ibuf, width, height, message);

	// Fade in for 1 sec (50 cycles)
	gpal->fade (50, 1, 0);

	// Display text for approx 3 seonds
	for (i = 0; i < 30; i++)
	{
		if (wait_delay (100))
		{
			gwin->clear_screen(true);
			FORGET_ARRAY(fli_b[0]);
			FORGET_ARRAY(fli_b[1]);
			FORGET_ARRAY(fli_b[2]);
			return;
		}
	}

	// Fade out for 1 sec (50 cycles)
	gpal->fade (50, 0, 0);

	next = fli3.play(win, 0, 0, next);
	pal = fli3.get_palette();
	next = SDL_GetTicks();
	for (i = 1000 + next; next < i; next += 10)
	{
		// Speed related frame skipping detection
		int skip_frame = Game_window::get_instance()->get_frame_skipping() && SDL_GetTicks() >= next;
		while (SDL_GetTicks() < next)
			;
		if (!skip_frame)
		{
			pal->set_brightness (100 - (i-next) / 10);
			pal->apply();
		}
		if (wait_delay (0))
		{
			gwin->clear_screen(true);
			FORGET_ARRAY(fli_b[0]);
			FORGET_ARRAY(fli_b[1]);
			FORGET_ARRAY(fli_b[2]);
			return;
		}
	}
	
	buffer = (uint8 *) speech3.retrieve(size);
	Audio::get_ptr()->play (buffer+8, size-8, false);
	FORGET_ARRAY(buffer);

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

	next = SDL_GetTicks();
	for (i = next+28000; i > next; )
	{
		for (j = 0; j < finfo.frames; j++)
		{
			next = fli3.play(win, j, j, next);
			for(m=0; m<6; m++)
				endfont3->center_text(ibuf, centerx, starty+endfont3->get_text_height()*m, txt_screen0[m]);

			win->show ();
			if (wait_delay (10))
			{
				gwin->clear_screen(true);
				FORGET_ARRAY(fli_b[0]);
				FORGET_ARRAY(fli_b[1]);
				FORGET_ARRAY(fli_b[2]);
				return;
			}
		}
	}

	
	next = SDL_GetTicks();
	for (i = 1000 + next; next < i; next += 10)
	{
		// Speed related frame skipping detection
		int skip_frame = Game_window::get_instance()->get_frame_skipping() && SDL_GetTicks() >= next;
		while (SDL_GetTicks() < next)
			;
		if (!skip_frame)
		{
			pal->set_brightness ((i - next) / 10);
			pal->apply();
		}
		if (wait_delay (0))
		{
			gwin->clear_screen(true);
			FORGET_ARRAY(fli_b[0]);
			FORGET_ARRAY(fli_b[1]);
			FORGET_ARRAY(fli_b[2]);
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

	starty = (gwin->get_height() - normal->get_text_height()*10)/2;
	
	for(i=0; i<10; i++)
		normal->draw_text (ibuf, centerx-normal->get_text_width(txt_screen1[i])/2, starty+normal->get_text_height()*i, txt_screen1[i]);

	// Fade in for 1 sec (50 cycles)
	gpal->fade (50, 1, 0);

	// Display text for 20 seonds (only 10 at the moment)
	for (i = 0; i < 100; i++)
	{
		if (wait_delay (100))
		{
			gwin->clear_screen(true);
			FORGET_ARRAY(fli_b[0]);
			FORGET_ARRAY(fli_b[1]);
			FORGET_ARRAY(fli_b[2]);
			return;
		}
	}

	// Fade out for 1 sec (50 cycles)
	gpal->fade (50, 0, 0);

	if (wait_delay (10))
	{
		gwin->clear_screen(true);
		FORGET_ARRAY(fli_b[0]);
		FORGET_ARRAY(fli_b[1]);
		FORGET_ARRAY(fli_b[2]);
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

	starty = (gwin->get_height() - normal->get_text_height()*6)/2;
	
	for(i=0; i<6; i++)
		normal->draw_text (ibuf, centerx-normal->get_text_width(txt_screen2[i])/2, starty+normal->get_text_height()*i, txt_screen2[i]);


	// Fade in for 1 sec (50 cycles)
	gpal->fade (50, 1, 0);

	// Display text for 20 seonds (only 8 at the moment)
	for (i = 0; i < 80; i++)
	{
		if (wait_delay (100))
		{
			gwin->clear_screen(true);
			FORGET_ARRAY(fli_b[0]);
			FORGET_ARRAY(fli_b[1]);
			FORGET_ARRAY(fli_b[2]);
			return;
		}
	}

	// Fade out for 1 sec (50 cycles)
	gpal->fade (50, 0, 0);


	if (wait_delay (10))
	{
		gwin->clear_screen(true);
		FORGET_ARRAY(fli_b[0]);
		FORGET_ARRAY(fli_b[1]);
		FORGET_ARRAY(fli_b[2]);
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

	starty = (gwin->get_height() - normal->get_text_height()*6)/2;
	
	for(i=0; i<6; i++)
		normal->draw_text (ibuf, centerx-normal->get_text_width(txt_screen3[i])/2, starty+normal->get_text_height()*i, txt_screen3[i]);

	// Fade in for 1 sec (50 cycles)
	gpal->fade (50, 1, 0);

	// Display text for 20 seonds (only 8 at the moment)
	for (i = 0; i < 80; i++)
	{
		if (wait_delay (100))
		{
			gwin->clear_screen(true);
			FORGET_ARRAY(fli_b[0]);
			FORGET_ARRAY(fli_b[1]);
			FORGET_ARRAY(fli_b[2]);
			return;
		}
	}

	// Fade out for 1 sec (50 cycles)
	gpal->fade (50, 0, 0);


	if (wait_delay (10))
	{
		gwin->clear_screen(true);
		FORGET_ARRAY(fli_b[0]);
		FORGET_ARRAY(fli_b[1]);
		FORGET_ARRAY(fli_b[2]);
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

	starty = (gwin->get_height() - normal->get_text_height()*4)/2;
	
	for(i=0; i<4; i++)
		normal->draw_text (ibuf, centerx-normal->get_text_width(txt_screen4[i])/2, starty+normal->get_text_height()*i, txt_screen4[i]);


	// Fade in for 1 sec (50 cycles)
	gpal->fade (50, 1, 0);

	// Display text for 10 seonds (only 5 at the moment)
	for (i = 0; i < 50; i++)
	{
		if (wait_delay (100))
		{
			gwin->clear_screen(true);
			FORGET_ARRAY(fli_b[0]);
			FORGET_ARRAY(fli_b[1]);
			FORGET_ARRAY(fli_b[2]);
			return;
		}
	}

	// Fade out for 1 sec (50 cycles)
	gpal->fade (50, 0, 0);


	gwin->clear_screen(true);
	FORGET_ARRAY(fli_b[0]);
	FORGET_ARRAY(fli_b[1]);
	FORGET_ARRAY(fli_b[2]);
}

void BG_Game::show_quotes()
{
	play_midi(quotes_midi);
	TextScroller quotes(MAINSHP_FLX, 0x10, 
			fontManager.get_font("MENU_FONT"),
			menushapes.extract_shape(0x14)
			);
	pal.load("<STATIC>/intropal.dat",6);
	quotes.run(gwin,pal);
	pal.load("<STATIC>/intropal.dat",0);
}

void BG_Game::show_credits()
{
	
	play_midi(credits_midi);
	TextScroller credits(MAINSHP_FLX, 0x0E, 
			fontManager.get_font("MENU_FONT"),
			menushapes.extract_shape(0x14)
			);
	pal.load("<STATIC>/intropal.dat",6);
	if(credits.run(gwin,pal)) {	// Watched through the entire sequence?
		std::ofstream quotesflg;
		U7open(quotesflg, "<SAVEGAME>/quotes.flg");
		quotesflg.close();
	}
		
	pal.load("<STATIC>/intropal.dat",0);
}

bool BG_Game::new_game(Vga_file &shapes)
{
	int menuy = topy+110;
	Font *font = fontManager.get_font("MENU_FONT");

	// Need to know if SI is installed
	bool si_installed = SI_Game::is_installed();

	U7object faces_u7o("<DATA>/exult_bg.flx", EXULT_BG_FLX_MR_INTRO_SHP);
	size_t shapesize;
	char *shape_buf = faces_u7o.retrieve(shapesize);
	BufferDataSource faces_ds(shape_buf, shapesize);
	Shape_file faces_shape(&faces_ds);

	const int max_name_len = 16;
	char npc_name[max_name_len+1];
	char disp_name[max_name_len+2];
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

			Shape_frame *sex_shape = shapes.get_shape(0xA, selected==1?1:0);
			gwin->paint_shape(topx+10,menuy+25,sex_shape);
			int sex_width = sex_shape->get_width()+10;
			if (sex_width > 35) sex_width += 25; 
			else sex_width = 60;

			if (si_installed)
			{
				gwin->paint_shape(topx+sex_width,menuy+25,shapes.get_shape(0xB,sex%2));

				if (sex >= 2)
				{
					gwin->paint_shape(topx+250,menuy+10,faces_shape.get_frame(7-sex));
				}
				else
					gwin->paint_shape(topx+250,menuy+10,shapes.get_shape(sex,0));
			}
			else
			{
				gwin->paint_shape(topx+sex_width,menuy+25,shapes.get_shape(0xB,sex));
				gwin->paint_shape(topx+250,menuy+10,shapes.get_shape(sex,0));
			}

			gwin->paint_shape(topx+10,topy+180,shapes.get_shape(0x8,selected==2?1:0));
			gwin->paint_shape(centerx+10,topy+180,shapes.get_shape(0x7,selected==3?1:0));
			if(selected==0)
				snprintf(disp_name, max_name_len+2, "%s_", npc_name);
			else
				snprintf(disp_name, max_name_len+2, "%s", npc_name);
			font->draw_text(ibuf, topx+60, menuy+10, disp_name);
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
					int len = strlen(npc_name);
					if(len<max_name_len)
					{
						npc_name[len] = ' ';
						npc_name[len+1] = 0;
					}
				}
				else if(selected==1)
				{
					if (si_installed)
					{
						sex++;
						if (sex >= 8) sex = 0;
					}
					else
						sex = !sex;
				}
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
				if(selected==1)
				{
					if (si_installed)
					{
						sex--;
						if (sex < 0) sex = 7;
					}
					else
						sex = !sex;
				}
				break;

			case SDLK_RIGHT:
				if(selected==1)
				{
					if (si_installed)
					{
						sex++;
						if (sex >= 8) sex = 0;
					}
					else
						sex = !sex;
				}
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
					if(strlen(npc_name)>0)
						npc_name[strlen(npc_name)-1] = 0;
				}
				break;
			default:
				{
					int c = event.key.keysym.sym;
					if(selected==0 && c>=SDLK_SPACE && c<=SDLK_z)
					{
						int len = strlen(npc_name);
						char chr = (event.key.keysym.mod & KMOD_SHIFT) ? toupper(c) : c;
						if(len<max_name_len)
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

	FORGET_ARRAY(shape_buf);

	if(ok)
	{
		std::cout << "Skin is: " << (3-(sex/2)) << " Sex is: " << (sex%2) << std::endl;
		set_avskin(3-(sex/2));
		set_avname (npc_name);
		set_avsex (sex%2);
		pal.fade_out(c_fade_out_time);
		gwin->clear_screen(true);	
		ok =gwin->init_gamedat(true);
	}
	win->fill8(0,gwin->get_width(),90,0,menuy);

	return ok;
}

bool BG_Game::is_installed()
{
	std::string buf("<BLACKGATE_STATIC>/endgame.dat");
	std::cout << "is_installed: '" << get_system_path(buf);
	bool found = U7exists(buf) && U7exists("<DATA>/exult_bg.flx");
	if (found)
		std::cout << "': yes" << std::endl;
	else
		std::cout << "': no" << std::endl;

	return found;
}
