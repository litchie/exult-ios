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
#  include <cctype>
#endif

#include "SDL_events.h"
#include "files/U7file.h"
#include "files/utils.h"
#include "flic/playfli.h"
#include "gamewin.h"
#include "Audio.h"
#include "sigame.h"
#include "palette.h"
#include "databuf.h"
#include "font.h"
#include "txtscroll.h"
#include "exult_types.h"
#include "mappatch.h"
#include "shapeid.h"

using std::cout;
using std::endl;
using std::rand;
using std::strlen;
using std::toupper;

SI_Game::SI_Game()
	{
		add_shape("gumps/check",2);
		add_shape("gumps/fileio",3);
		add_shape("gumps/fntext",4);
		add_shape("gumps/loadbtn",5);
		add_shape("gumps/savebtn",6);
		add_shape("gumps/halo",7);
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
		add_shape("gumps/book",27);
		add_shape("gumps/scroll",49);
		add_shape("gumps/combat_stats",91);
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
		add_shape("gumps/chest", 18);
		add_shape("gumps/shipshold", 21);
		add_shape("gumps/drawer", 22);
		add_shape("gumps/woodsign", 44);
		add_shape("gumps/tombstone", 45);
		add_shape("gumps/goldsign", 46);
		add_shape("gumps/body", 48);
		add_shape("gumps/tree", 64);

		add_shape("gumps/cstats/1",58);
		add_shape("gumps/cstats/2",59);
		add_shape("gumps/cstats/3",60);
		add_shape("gumps/cstats/4",61);
		add_shape("gumps/cstats/5",62);
		add_shape("gumps/cstats/6",63);

		add_shape("sprites/map", 22);
		add_shape("sprites/cheatmap", 0);

		// Need to be done
		add_shape("gumps/spell_scroll",65);
		add_shape("gumps/jawbone", 56);
		add_shape("gumps/tooth", 57);

		add_resource("files/shapes/count", 0, 8);
		add_resource("files/shapes/0", "<STATIC>/shapes.vga", 0);
		add_resource("files/shapes/1", "<STATIC>/faces.vga", 0);
		add_resource("files/shapes/2", "<STATIC>/gumps.vga", 0);
		add_resource("files/shapes/3", "<STATIC>/sprites.vga", 0);
		add_resource("files/shapes/4", MAINSHP_FLX, 0);
		add_resource("files/shapes/5", "<STATIC>/paperdol.vga", 0);
		add_resource("files/shapes/6", "<DATA>/exult.flx", 0);
		add_resource("files/shapes/7", "<STATIC>/fonts.vga", 0);

		add_resource("files/gameflx", "<DATA>/exult_si.flx", 0);
	
		add_resource("config/defaultkeys", "<DATA>/exult_si.flx", 1);

		add_resource("palettes/count", 0, 14);
		add_resource("palettes/0", PALETTES_FLX, 0);
		add_resource("palettes/1", PALETTES_FLX, 1);
		add_resource("palettes/2", PALETTES_FLX, 2);
		add_resource("palettes/3", PALETTES_FLX, 3);
		add_resource("palettes/4", PALETTES_FLX, 4);
		add_resource("palettes/5", PALETTES_FLX, 5);
		add_resource("palettes/6", PALETTES_FLX, 6);
		add_resource("palettes/7", PALETTES_FLX, 7);
		add_resource("palettes/8", PALETTES_FLX, 8);
		add_resource("palettes/9", PALETTES_FLX, 9);
		add_resource("palettes/10", PALETTES_FLX, 10);
		add_resource("palettes/11", PALETTES_FLX, 11);
		add_resource("palettes/12", PALETTES_FLX, 12);
		add_resource("palettes/13", MAINSHP_FLX, 1);
		add_resource("palettes/14", MAINSHP_FLX, 26);
		
		fontManager.add_font("MENU_FONT", MAINSHP_FLX, 9, 1);
		fontManager.add_font("SIINTRO_FONT", "<STATIC>/intro.dat", 14, 0);
		Map_patch_collection *mp = gwin->get_map_patches();
					// Egg by "PC pirate" in forest:
		mp->add(new Map_patch_remove(Object_spec(
				Tile_coord(647, 1899, 0), 275, 7, 1)));
					// Carpets above roof in Monitor:
		mp->add(new Map_patch_remove(Object_spec(
				Tile_coord(1035, 2572, 8), 483, 1, 0), true));
		mp->add(new Map_patch_remove(Object_spec(
				Tile_coord(1034, 2571, 6), 483, 1, 0)));
		mp->add(new Map_patch_remove(Object_spec(
				Tile_coord(1034, 2571, 5), 483, 1, 0), true));
					// Neyobi under a fur:
		mp->add(new Map_patch_modify(Object_spec(
				Tile_coord(1012, 873, 0), 867, 13, 0), 
					Object_spec(
				Tile_coord(1013, 873, 1), 867, 13, 0)));

	}

SI_Game::~SI_Game()
	{
	}

// Little weighted random function for lightning on the castle
static int get_frame (void)
{
	int num = rand() % 9;

	if (num >= 8) return 2;
	else if (num >= 6) return 1;
	return 0;
}

void SI_Game::play_intro()
{
	int	next = 0;
	size_t	flisize;
	char	*fli_b = 0;
	uint8	*buffer = 0;
	size_t	size;
	size_t	shapesize;
	char *	shape_buf = 0;
	int		i,j;
	Font *font = fontManager.get_font("MENU_FONT");
	Font *sifont = fontManager.get_font("SIINTRO_FONT");

	bool speech = Audio::get_ptr()->is_speech_enabled();

	gwin->clear_screen(true);
	
	Audio::get_ptr()->stop_music();

	// Lord British presents...
	try
	{
		U7object lbflic("<STATIC>/intro.dat", 0);
		fli_b = lbflic.retrieve(flisize);
		playfli fli0(fli_b+8, flisize-8);
		fli0.info();

		const char *txt_msg[] = { "with help from",
				"The Exult Team", 
				"Driven by Exult" };

		for (j = 0; j < 20; j++)
		{
			next = fli0.play(win, 0, 0, next, j*5);
			font->center_text(ibuf, centerx, centery+50, txt_msg[0]);
			font->center_text(ibuf, centerx, centery+65, txt_msg[1]);
			win->show();
		}


		next = fli0.play(win, 0, 0, next, 100);
		font->center_text(ibuf, centerx, centery+50, txt_msg[0]);
		font->center_text(ibuf, centerx, centery+65, txt_msg[1]);
		win->show();

		if (wait_delay (3000))
			throw UserBreakException();

		for (j = 20; j; j--)
		{
			next = fli0.play(win, 0, 0, next, j*5);
			font->center_text(ibuf, centerx, centery+50, txt_msg[0]);
			font->center_text(ibuf, centerx, centery+65, txt_msg[1]);
			win->show();
		}


		FORGET_ARRAY(fli_b);

		if (wait_delay (0))
			throw UserBreakException();

		gwin->clear_screen(true);

		
		// Castle Outside

		// Start Music
		Audio *audio = Audio::get_ptr();
		if (audio) {
			const char *fn = "<STATIC>/r_sintro.xmi";
			MyMidiPlayer *midi = audio->get_midi();
			if (midi && midi->is_fm_synth()) fn = "<STATIC>/a_sintro.xmi";
			audio->start_music (fn, 0, false);
		}

		// Thunder, note we use the buffer again later so it's not freed here
		if (speech)
		{
			U7object voc_thunder("<STATIC>/intro.dat", 15);
			buffer = (uint8 *) voc_thunder.retrieve(size);
			Audio::get_ptr()->play (buffer+8, size-8, false);
		}

		U7object flic("<STATIC>/intro.dat", 1);
		fli_b = flic.retrieve(flisize);
		playfli fli1(fli_b+8, flisize-8);
		fli1.info();

		fli1.play(win, 0, 1, 0, 0);

		next = SDL_GetTicks();
		int	prev = -1;
		int 	num;
		
		for (j = 0; j < 20; j++)
		{
			num = get_frame();
			if (prev != num)
				for (i = 0; i < num+1; i++)
					fli1.play(win, i, i, next);

			prev = num;
			next += 75;
			win->show();
			if (wait_delay (1))
				throw UserBreakException();

		}

		const char *lb_cas = "Lord British's Castle";
		const char *db_cas = "Dick British's Castle";
		
		for (j = 0; j < 50; j++)
		{
			num = get_frame();
			if (prev != num)
				for (i = 0; i < num+1; i++)
					fli1.play(win, i, i, next);

			if (jive)
				sifont->center_text(ibuf, centerx, centery+50, db_cas);
			else 
				sifont->center_text(ibuf, centerx, centery+50, lb_cas);

			prev = num;
			next += 75;
			win->show();
			if (wait_delay (1))
				throw UserBreakException();

		}

		for (j = 0; j < 10; j++)
		{
			num = get_frame();
			if (prev != num)
				for (i = 0; i < num+1; i++)
					fli1.play(win, i, i, next);

			// Thunder again, we free the buffer here 
			if (speech && j == 5) { 
				Audio::get_ptr()->play (buffer+8, size-8, false);
				FORGET_ARRAY(buffer);
			}

			prev = num;
			next += 75;
			win->show();
			if (wait_delay (1))
				throw UserBreakException();

		}

		const char *bg_fellow[] = { "Eighteen months after the destruction", 
				"of the Black Gate and the", 
				"dismantling of The Fellowship"};


		for (j = 0; j < 75; j++)
		{
			num = get_frame();
			if (prev != num)
				for (i = 0; i < num+1; i++)
					fli1.play(win, i, i, next);

			for(i=0; i<3; i++) {
				sifont->center_text(ibuf, centerx, centery+50+15*i, bg_fellow[i]);
			}

			prev = num;
			next += 75;
			win->show();
			if (wait_delay (1))
				throw UserBreakException();

		}

		for (j = 20; j; j--)
		{
			next = fli1.play(win, 0, 0, next, j*5);
			win->show();
			if (wait_delay (0))
				throw UserBreakException();

		}
		FORGET_ARRAY(fli_b);

		if (wait_delay (0))
			throw UserBreakException();

		// Do this! Prevents palette corruption
		gwin->clear_screen(true);

		// Guard walks in
		U7object flic2("<STATIC>/intro.dat", 2);
		fli_b = flic2.retrieve(flisize);
		playfli fli2(fli_b+8, flisize-8);
		fli2.info();

		for (j = 0; j < 20; j++)
		{
			next = fli2.play(win, 0, 0, next, j*5);
			win->show();
			if (wait_delay (0))
				throw UserBreakException();
		}

		// Guard walks in
		for (j = 0; j < 37; j++)
		{
			next = fli2.play(win, j, j, next);
			win->show();
			if (wait_delay (0))
				throw UserBreakException();
		}

		// Guard walks in
		const char *my_leige = "My leige";
		const char *yo_homes = "Yo, homes";

		if (speech && !jive)
		{
			U7object voc_my_leige("<STATIC>/intro.dat", 16);
			buffer = (uint8 *) voc_my_leige.retrieve (size);
			Audio::get_ptr()->play (buffer+8, size-8, false);
			FORGET_ARRAY(buffer);
		}


		for (; j < 55; j++)
		{
			next = fli2.play(win, j, j, next);

			if (jive)
				sifont->draw_text(ibuf, centerx+30, centery+87, yo_homes);
			else if (!speech)
				sifont->draw_text(ibuf, centerx+30, centery+87, my_leige);

			win->show();
			if (wait_delay (0))
				throw UserBreakException();
		}

		next = fli2.play(win, j, j, next);
		win->show();

		const char *all_we[2] = { "All we found among Batlin's",
					"belongings was this enchanted scroll.." };

		if (speech && !jive)
		{
			U7object voc_all_we("<STATIC>/intro.dat", 17);
			buffer = (uint8 *) voc_all_we.retrieve (size);
			Audio::get_ptr()->play (buffer+8, size-8, false);
			FORGET_ARRAY(buffer);
		}

		for (; j < 73; j++)
		{
			next = fli2.play(win, j, j, next);

			if (!speech || jive)
			{
				sifont->draw_text(ibuf, centerx+150-sifont->get_text_width(all_we[0]), centery+74, all_we[0]);
				sifont->draw_text(ibuf, centerx+160-sifont->get_text_width(all_we[1]), centery+87, all_we[1]);
			}

			win->show();
			if (wait_delay (0))
				throw UserBreakException();
		}
		for (i = 0; i < 220; i++)
			if (wait_delay (10))
				throw UserBreakException();

		const char *and_a[2] = { "and a map showing the way to",
					"a place called the Serpent Isle" };

		next = fli2.play(win, j, j, next);

		if (!speech || jive)
		{
			sifont->draw_text(ibuf, centerx+150-sifont->get_text_width(and_a[0]), centery+74, and_a[0]);
			sifont->draw_text(ibuf, centerx+150-sifont->get_text_width(and_a[1]), centery+87, and_a[1]);
		}

		win->show();
		j++;
		
		for (i = 0; i < 290; i++)
			if (wait_delay (10))
				throw UserBreakException();


		fli2.play(win, j, j);
		j++;

		for (i = 0; i < 50; i++)
			if (wait_delay (10))
				throw UserBreakException();

		const char *indeed[2] = { "Indeed.",
					"Put it on the table." };

		const char *iree = { "Iree. Slap it down there!" };

		if (speech && !jive)
		{
			U7object voc_indeed("<STATIC>/intro.dat", 18);
			buffer = (uint8 *) voc_indeed.retrieve(size);
			Audio::get_ptr()->play (buffer+8, size-8, false);
			FORGET_ARRAY(buffer);
		}

		next = fli2.play(win, j, j);
		j++;
		
		for (; j < 81; j++)
		{
			next = fli2.play(win, j, j, next);

			if (jive)
				sifont->draw_text(ibuf, topx+40, centery+74, iree);
			else if (!speech)
			{
				sifont->draw_text(ibuf, topx+40, centery+74, indeed[0]);
				sifont->draw_text(ibuf, topx+40, centery+87, indeed[1]);
			}

			win->show();
			if (wait_delay (0))
				throw UserBreakException();
		}

		for (i = 0; i < 200; i++)
			if (wait_delay (10))
				throw UserBreakException();

		FORGET_ARRAY(fli_b);

		// Do this! Prevents palette corruption
		gwin->clear_screen(true);

		// Scroll opens
		U7object flic3("<STATIC>/intro.dat", 3);
		fli_b = flic3.retrieve(flisize);
		playfli fli3(fli_b+8, flisize-8);
		fli3.info();

		next = 0;

		// Scroll opens
		for (j = 0; j < 20; j++)
		{
			next = fli3.play(win, j, j, next)+20;
			win->show();
			if (wait_delay (0))
				throw UserBreakException();
		}


		// 'Stand Back'
		const char *stand_back = "Stand Back!";
		const char *jump_back = "Jump Back!";

		if (speech && !jive)
		{
			U7object voc_stand_back("<STATIC>/intro.dat", 19);
			buffer = (uint8 *) voc_stand_back.retrieve(size);
			Audio::get_ptr()->play (buffer+8, size-8, false);
			FORGET_ARRAY(buffer);
		}

		for (; j < 61; j++)
		{
			next = fli3.play(win, j, j, next)+20;

			if (jive)
				sifont->draw_text(ibuf, topx+70, centery+60, jump_back);
			else if (!speech)	
				sifont->draw_text(ibuf, topx+70, centery+60, stand_back);

			win->show();
			if (wait_delay (0))
				throw UserBreakException();
		}

		FORGET_ARRAY(fli_b);

		// Do this! Prevents palette corruption
		gwin->clear_screen(true);

		// Big G speaks
		U7object flic4("<STATIC>/intro.dat", 4);
		fli_b = flic4.retrieve(flisize);
		playfli fli4(fli_b+8, flisize-8);
		fli4.info();

		U7object shapes("<STATIC>/intro.dat", 30);
		shape_buf = shapes.retrieve(shapesize);
		BufferDataSource gshape_ds(shape_buf+8, shapesize-8);
		Shape_frame *sf;
	
		Shape_file gshape(&gshape_ds);
		
		cout << "Shape in intro.dat has " << gshape.get_num_frames() << endl;

		if (speech && !jive)
		{
			U7object voc_big_g("<STATIC>/intro.dat", 20);
			buffer = (uint8 *) voc_big_g.retrieve ( size);
			Audio::get_ptr()->play (buffer+8, size-8, false);
			FORGET_ARRAY(buffer);
		}

		next = 0;

		// Batlin...
		const char *batlin[2] = { "Batlin! In the event that the",
					"Avatar destroys the Black Gate" };

		const char *you_shall[2] = { "you shall follow the unwitting",
					"human Gwenno to the Serpent Isle" };

		const char *there_i[2] = { "There I shall outline my plan",
					"to destroy Britannia!" };


		const char *batlin2[2] = { "Batlin! Know that my face is most",
					"muppet like!" };

		const char *you_must[2] = { "You must go to the Serpent Isle",
					"to learn the secret of Acne Medication" };

		const char *soon_i[2] = { "Soon I and my horde of muppets will",
					"destroy Britannia!" };

		for (j = 0; j < 320; j++)
		{
			next = fli4.play(win, j%40, j%40, next);

			if (j < 300)
				sf = gshape.get_frame((j/2)%7);
			else
				sf = gshape.get_frame(0);

			if (sf)
				sman->paint_shape (centerx-36, centery, sf);

			if (j < 100 && jive)
			{
				sifont->center_text(ibuf, centerx, centery+74, batlin2[0]);
				sifont->center_text(ibuf, centerx, centery+87, batlin2[1]);
			}
			else if (j < 200 && jive)
			{
				sifont->center_text(ibuf, centerx, centery+74, you_must[0]);
				sifont->center_text(ibuf, centerx, centery+87, you_must[1]);
			}
			else if (j < 300 && jive)
			{
				sifont->center_text(ibuf, centerx, centery+74, soon_i[0]);
				sifont->center_text(ibuf, centerx, centery+87, soon_i[1]);
			}
			else if (j < 100 && !speech)
			{
				sifont->center_text(ibuf, centerx, centery+74, batlin[0]);
				sifont->center_text(ibuf, centerx, centery+87, batlin[1]);
			}
			else if (j < 200 && !speech)
			{
				sifont->center_text(ibuf, centerx, centery+74, you_shall[0]);
				sifont->center_text(ibuf, centerx, centery+87, you_shall[1]);
			}
			else if (j < 300 && !speech)
			{
				sifont->center_text(ibuf, centerx, centery+74, there_i[0]);
				sifont->center_text(ibuf, centerx, centery+87, there_i[1]);
			}

			win->show();
			if (wait_delay (0))
				throw UserBreakException();
		}
		sf = gshape.get_frame(0);

		for (j = 20; j; j--)
		{
			next = fli4.play(win, 0, 0, next, j*5);
			if (sf)
				sman->paint_shape (centerx-36, centery, sf);

			win->show();
			if (wait_delay (0))
				throw UserBreakException();
		}

		FORGET_ARRAY(shape_buf);
		FORGET_ARRAY(fli_b);
		
		// Do this! Prevents palette corruption
		gwin->clear_screen(true);

		// Tis LBs's Worst fear
		U7object flic5("<STATIC>/intro.dat", 5);
		fli_b = flic5.retrieve(flisize);
		playfli fli5(fli_b+8, flisize-8);
		fli5.info();

		for (j=0; j < 20; j++)
		{
			next = fli5.play(win, 0, 0, next, j*5);

			win->show();
			if (wait_delay (0))
				throw UserBreakException();
		}


		const char *tis_my[3] = {"'Tis my worst fear!",
					"I must send the Avatar through",
					"the pillars to the Serpent Isle!" };


		if (speech && !jive)
		{
			U7object voc_tis_my("<STATIC>/intro.dat", 21);
			buffer = (uint8 *) voc_tis_my.retrieve(size);
			Audio::get_ptr()->play (buffer+8, size-8, false);
			FORGET_ARRAY(buffer);
		}

		for (j=0; j < 61; j++)
		{
			next = fli5.play(win, j, j, next)+30;

			if (j < 20 && (!speech || jive))
			{
				sifont->center_text(ibuf, centerx, centery+74, tis_my[0]);
			}
			else if (j > 22 && (!speech || jive))
			{
				sifont->center_text(ibuf, centerx, centery+74, tis_my[1]);
				sifont->center_text(ibuf, centerx, centery+87, tis_my[2]);
			}

			win->show();
			if (wait_delay (0))
				throw UserBreakException();
		}

		FORGET_ARRAY(fli_b);

		// Do this! Prevents palette corruption
		gwin->clear_screen(true);

		// Boat 1
		U7object flic6("<STATIC>/intro.dat", 6);
		fli_b = flic6.retrieve(flisize);
		playfli fli6(fli_b+8, flisize-8);
		fli6.info();

		for (j=0; j < 61; j++)
		{
			next = fli6.play(win, j, j, next)+30;

			win->show();
			if (wait_delay (0))
				throw UserBreakException();
		}

		FORGET_ARRAY(fli_b);

		// Do this! Prevents palette corruption
		gwin->clear_screen(true);

		// Boat 2
		U7object flic7("<STATIC>/intro.dat", 7);
		fli_b = flic7.retrieve(flisize);
		playfli fli7(fli_b+8, flisize-8);
		fli7.info();

		const char *zot = "Zot!";

		for (j=0; j < 61; j++)
		{
			next = fli7.play(win, j, j, next)+30;

			if (j > 55 && jive)
				sifont->center_text(ibuf, centerx, centery+74, zot);

			win->show();
			if (wait_delay (0))
				throw UserBreakException();
		}

		FORGET_ARRAY(fli_b);

		// Do this! Prevents palette corruption
		gwin->clear_screen(true);

		// Ultima VII Part 2
		U7object flic8("<STATIC>/intro.dat", 8);
		fli_b = flic8.retrieve(flisize);
		playfli fli8(fli_b+8, flisize-8);
		fli8.info();

		for (j = 0; j < 20; j++)
		{
			next = fli8.play(win, 0, 0, next, j*5);
			font->center_text(ibuf, centerx, centery+75, txt_msg[2]);
			win->show();
		}


		next = fli8.play(win, 0, 0, next, 100);
		font->center_text(ibuf, centerx, centery+75, txt_msg[2]);
		win->show();

		for (i = 0; i < 300; i++)
			if (wait_delay (10))
				throw UserBreakException();


		for (j = 20; j; j--)
		{
			next = fli8.play(win, 0, 0, next, j*5);
			font->center_text(ibuf, centerx, centery+75, txt_msg[2]);
			win->show();
		}
		
		FORGET_ARRAY(fli_b);
	}
	catch(const UserBreakException &x)
	{
		FORGET_ARRAY(shape_buf);
		FORGET_ARRAY(fli_b);
		FORGET_ARRAY(buffer);
	}
	
	// Fade out the palette...
//	pal.fade_out(c_fade_out_time);
// this doesn't work right ATM since the FLIC player has its own palette handling

	
	// ... and clean the screen.
	gwin->clear_screen(true);
	
	// Stop all audio output
	Audio::get_ptr()->cancel_streams();
}

void SI_Game::top_menu()
{
	play_midi(28, true);
	sman->paint_shape(topx,topy,menushapes.get_shape(0x2,0));
	pal->load(MAINSHP_FLX,26);
	pal->fade_in(60);	
}

void SI_Game::show_journey_failed()
{
    pal->fade_out(50);
	sman->paint_shape(topx,topy,menushapes.get_shape(0x2,0));
	journey_failed_text();
}

/*
 *	ExCineLite
 */

// ExCineEvent
struct ExCineEvent {
	uint32			time;	// Time to start, In MS
	char			*file;
	int			index;

	virtual bool	play_it(Image_window *win, int time) = 0;		// Return true if screen updated

	bool can_play() { return file != 0; }

	ExCineEvent(uint32 t, char*f, int i) :
		time(t), file(f), index(i)  { }

	virtual ~ExCineEvent() { }
};

//
// ExCineFlic
//

struct ExCineFlic : public ExCineEvent {
private:
	int	start;		// First frame to play
	int	count;		// Number of frames
	bool	repeat;		// Repeat?
	int	cur;		// Frame currently being displayed (note, it's not the actual frame)
	int	speed;		// Speed of playback (ms per frame)

	// Data info
	U7object	*flic_obj;
	size_t		size;
	char		*buffer;
	playfli		*player;

public:
	virtual bool	play_it(Image_window *win, int t);

	void		load_flic(void);
	void		free_flic(void);

	void		fade_out(int cycles);

	ExCineFlic(uint32 time, char *file, int i, int s, int c, bool r, int spd) :
		ExCineEvent(time, file, i), start(s), count(c), repeat(r), cur(-1), speed(spd),
		flic_obj(0), size(0), buffer(0), player(0) { }

	ExCineFlic(uint32 time) : ExCineEvent(time, 0, 0), start(0), count(0),
				repeat(false), cur(0), speed(0),
				flic_obj(0), size(0), buffer(0), player(0) { }

	virtual ~ExCineFlic() {
		free_flic();
	}
};

void ExCineFlic::load_flic()
{
	free_flic();

	if (file) COUT("Loading " << file << ":" << index);

	flic_obj = new U7object(file, index);

	buffer = flic_obj->retrieve(size);

	player = new playfli(buffer+8, size-8);
	player->info();
}

void ExCineFlic::free_flic()
{
	if (file) COUT("Freeing " << file << ":" << index);

	FORGET_OBJECT(player);
	FORGET_ARRAY(buffer);
	size = 0;
	FORGET_OBJECT(flic_obj);
}

bool	ExCineFlic::play_it(Image_window *win, int t)
{
	if (t < time) return false;

	if (cur+1 < count || repeat) {

		// Only advance frame if we can
		int time_next = time + (cur+1) * speed;
		if (time_next <= t) {
			cur++;

			// The actual frame number
			int actual = start + (cur%count);

			player->play(win, actual, actual, 0);

			return true;
		}
	}

	player->put_buffer(win);

	return false;
}

void ExCineFlic::fade_out(int cycles)
{
	if (player) player->get_palette()->fade_out(cycles);
}

//
// ExCineVoc
//

struct ExCineVoc : public ExCineEvent {
private:
	bool		played;

public:
	virtual bool	play_it(Image_window *win, int t);

	ExCineVoc(uint32 time, char *f, int index) :
		ExCineEvent(time, file, index), played(false) { }

	virtual ~ExCineVoc() { }
};

bool ExCineVoc::play_it(Image_window *win, int t)
{
	size_t	size;
	U7object voc("<STATIC>/intro.dat", index);
	uint8 *buffer = (uint8 *) voc.retrieve (size);
	Audio::get_ptr()->play (buffer+8, size-8, false);
	FORGET_ARRAY(buffer);
	played = true;

	return false;
}

//
// Serpent Isle Endgame
//
void SI_Game::end_game(bool success) 
{
	int	next = 0;

	Font	*font = fontManager.get_font("MENU_FONT");
	Font	*sifont = fontManager.get_font("SIINTRO_FONT");

	bool speech = Audio::get_ptr()->is_speech_enabled();

	gwin->clear_screen(true);
	

/* Endgame General Timings (in ms)

      0 - Avatar floating right
   6350 - Begin Serpent Swirling
  10850 - Serpent Comes on screen
  14643 - Balanced, and "there we are done"
  17281 - "balance is restored"
  21300 - "serpent isle"
  22900 - "briatnnia"
  24300 - "your earth"
  26000 - "the entire universe"
  28600 - "all are phased"
  31600 - Avatar floating right, "worry not about your friend dupre"
  35100 - "he is one with us"
  37000 - "and content"
  39800 - "good bye avatar"
  42040 - "we thank you"
  48550 - "well well well avatar"
  48900 - Avatar floating left
  51750 - "You have managed to thawt me one again"
  55500 - "by restoring balance..."
  62500 - floating far, "but now here you are"
  64500 - "poised at the edge of eternity"
  67000 - "where would you go?"
  70250 - floating left, "back to britannia?"
  72159 - "to earth?"
  74750 - "perhaps you would join me.."
  75500 - big g's hand
  78000 - "we do have a score to settle"

Flic Index
9 = Avfloat
10 = snake1
11 = snake2
12 = avfar
13 = xavgrab

Frame count : 61
Width :       320
Height :      200
Depth :       8
Speed :       5

Frame count : 156
Width :       320
Height :      200
Depth :       8
Speed :       8

Frame count : 4
Width :       320
Height :      200
Depth :       8
Speed :       10

Frame count : 61
Width :       320
Height :      200
Depth :       8
Speed :       5

Frame count : 121
Width :       320
Height :      200
Depth :       8
Speed :       5

Sound Index
22 = "there we are done, balance is restored"
23 = "serpent isle...."
24 = "goodbye avatar..."
25 = "well well well avatar..."
26 = "by restoring balance..."
27 = "but now here you are..."
28 = "back to britannia..."
29 = "perhaps you..."


      0 - Repeat 9
   6350 - Play 10
  14643 - Repeat 10, Play 22
  21300 - Play 23
  31600 - Repeat 9
  39800 - Repeat 11, Play 24
  48550 - Play 25
  48900 - Repeat 13
  55500 - Play 26
  62500 - Repeat 12, play 27
  70250 - Play 13, play 28
  72159 - "to earth?"
  74750 - play 29
  75500 - big g's hand
  78000 - "we do have a score to settle"

*/

	// Flic List
	ExCineFlic flics[] = {
		ExCineFlic(0, "<STATIC>/intro.dat", 9, 0, 61, true, 75),
		ExCineFlic(6350, "<STATIC>/intro.dat", 10, 0, 156, false, 95),
		ExCineFlic(21170, "<STATIC>/intro.dat", 9, 0, 61, true, 75),
		ExCineFlic(39800, "<STATIC>/intro.dat", 11, 0, 4, true, 75),
		ExCineFlic(48900, "<STATIC>/intro.dat", 13, 0, 61, true, 75),
		ExCineFlic(62500, "<STATIC>/intro.dat", 12, 0, 61, true, 75),
		ExCineFlic(70250, "<STATIC>/intro.dat", 13, 0, 121, false, 75),
		ExCineFlic(82300)
	};
	int last_flic = 7;
	int cur_flic = -1;
	ExCineFlic *flic = 0;
	ExCineFlic *pal_flic = 0;

	// Voc List
	ExCineVoc vocs[] = {
		ExCineVoc(14700, "<STATIC>/intro.dat", 22),
		ExCineVoc(21300, "<STATIC>/intro.dat", 23),
		ExCineVoc(39800, "<STATIC>/intro.dat", 24),
		ExCineVoc(47700, "<STATIC>/intro.dat", 25),
		ExCineVoc(55400, "<STATIC>/intro.dat", 26),
		ExCineVoc(62500, "<STATIC>/intro.dat", 27),
		ExCineVoc(70250, "<STATIC>/intro.dat", 28),
		ExCineVoc(74750, "<STATIC>/intro.dat", 29)
	};
	int last_voc = 7;
	int cur_voc = -1;

	// Stop previous music
	Audio::get_ptr()->stop_music();

	// Start the music
	Audio *audio = Audio::get_ptr();
	if (audio) {
		const char *fn = "<STATIC>/r_send.xmi";
		MyMidiPlayer *midi = audio->get_midi();
		if (midi && midi->is_fm_synth()) fn = "<STATIC>/a_send.xmi";
		audio->start_music (fn, 0, false);
	}


	int start_time = SDL_GetTicks();

	while (1) {

		int time = SDL_GetTicks() - start_time;

		// Need to go to the next flic?
		if (cur_flic < last_flic && flics[cur_flic+1].time <= time) {

			bool next_play = flics[cur_flic+1].can_play();

			// Can play the new one, don't need the old one anymore
			if(next_play) {
				// Free it
				if (flic) flic->free_flic();

				// Free the palette too, if we need to
				if (pal_flic && pal_flic != flic)
					pal_flic->free_flic();

				pal_flic = 0;
			}
			// Set palette to prev if required
			else if (flic && flic->can_play()) {
				pal_flic = flic;
			}
			// Previous flic didn't have a palette, so free it anyway
			else if (flic) {
				flic->free_flic();
			}
			
			cur_flic++;
			flic = flics+cur_flic;

			if (next_play) {
				// Clear the screen to prevent palette corruption
				gwin->clear_screen(true);

				// Load the flic, and set pal_flic
				(pal_flic = flic)->load_flic();
			}
			else COUT("Teminator ");
			COUT("Flic at time: " << flic->time);
		}

		// Need to go to the next voc?
		if (cur_voc < last_voc && vocs[cur_voc+1].time <= time) {
			cur_voc++;
			ExCineVoc *voc = vocs+cur_voc;

			// Just play it!
			voc->play_it(NULL, time);
			//else COUT("Teminator ");
			COUT("voc at time: " << voc->time);
			
		}

		// We've finished
		if (cur_flic == last_flic && cur_voc == last_voc) {
			// Do a fade out
			if (pal_flic && pal_flic->can_play()) pal_flic->fade_out(100);

			COUT("Finished!" << std::endl);
			break;
		}

		// Play the flic if possible

		bool updated = false;

		if (flic->can_play()) updated = flic->play_it(win, time);

		if (updated) win->show();

		if (wait_delay (0)) {

			// Do a quick fade out
			if (pal_flic && pal_flic->can_play()) pal_flic->fade_out(20);
			break;
		}
	}

	gwin->clear_screen(true);

	// Stop all sounds
	Audio::get_ptr()->cancel_streams();

	// Stop music
	Audio::get_ptr()->stop_music();
}

void SI_Game::show_quotes()
	{
		play_midi(32);
		TextScroller quotes(MAINSHP_FLX, 0x10, 
			     fontManager.get_font("MENU_FONT"),
			     menushapes.extract_shape(0x14)
			    );
		quotes.run(gwin);
	}

void SI_Game::show_credits()
	{
		play_midi(30);
		TextScroller credits(MAINSHP_FLX, 0x0E, 
			     fontManager.get_font("MENU_FONT"),
			     menushapes.extract_shape(0x14)
			    );
		if(credits.run(gwin)) {	// Watched through the entire sequence?
			std::ofstream quotesflg;
			U7open(quotesflg, "<SAVEGAME>/quotes.flg");
			quotesflg.close();
		}
	}

bool SI_Game::new_game(Vga_file &shapes)
{
	int menuy = topy+110;
	Font *font = fontManager.get_font("MENU_FONT");

	Vga_file faces_vga(FACES_VGA);
	
	const int max_len = 16;
	char npc_name[max_len+1];
	char disp_name[max_len+2];
	npc_name[0] = 0;
	int sex = 0;
	int selected = 0;
	int num_choices = 4;
	//pal.load("<STATIC>/intropal.dat",6);
	SDL_Event event;
	bool editing = true;
	bool redraw = true;
	bool ok = true;
	do
	{
		if (redraw)
		{
			gwin->clear_screen();
			sman->paint_shape(topx,topy,menushapes.get_shape(0x2,0));
			sman->paint_shape(topx+10,menuy+10,shapes.get_shape(0xC, selected==0?1:0));
			sman->paint_shape(topx+10,menuy+25,shapes.get_shape(0x19, selected==1?1:0));
			Shape_frame *sh = faces_vga.get_shape(0,sex);
			sman->paint_shape(topx+300,menuy+50,sh);
			sman->paint_shape(topx+10,topy+180,shapes.get_shape(0x8,selected==2?1:0));
			sman->paint_shape(centerx+10,topy+180,shapes.get_shape(0x7,selected==3?1:0));
			if(selected==0)
				snprintf(disp_name, max_len+2, "%s_", npc_name);
			else
				snprintf(disp_name, max_len+2, "%s", npc_name);
			font->draw_text(ibuf, topx+60, menuy+10, disp_name);
			pal->apply();
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
					if(len<max_len)
					{
						npc_name[len] = ' ';
						npc_name[len+1] = 0;
					}
				}
				else if(selected==1)
				{
					++sex;
					if(sex>5)
						sex = 0;
				}
				break;
			case SDLK_LEFT:
				if(selected==1)
				{
					--sex;
					if(sex<0)
						sex = 5;
				}
				break;
			case SDLK_RIGHT:
				if(selected==1)
				{
					++sex;
					if(sex>5)
						sex = 0;
				}
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

	gwin->clear_screen();
	sman->paint_shape(topx,topy,menushapes.get_shape(0x2,0));

	if(ok)
	{
		set_avname (npc_name);
		set_avsex (1-(sex%2));
		set_avskin (sex/2);
		pal->fade_out(c_fade_out_time);
		gwin->clear_screen(true);	
		ok = gwin->init_gamedat(true);
	}

	return ok;
}

bool SI_Game::is_installed()
{
	std::string buf("<SERPENTISLE_STATIC>/sispeech.spc");
	std::cout << "is_installed: '" << get_system_path(buf);
	bool found = U7exists(buf) && U7exists("<DATA>/exult_si.flx");
	if (found)
		std::cout << "' : yes" << std::endl;
	else
		std::cout << "' : no" << std::endl;

	return found;
}
