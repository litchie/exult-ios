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

		add_resource("files/shapes/count", 0, 6);
		add_resource("files/shapes/0", "static/shapes.vga", 0);
		add_resource("files/shapes/1", "static/faces.vga", 0);
		add_resource("files/shapes/2", "static/gumps.vga", 0);
		add_resource("files/shapes/3", "static/sprites.vga", 0);
		add_resource("files/shapes/4", "static/mainshp.flx", 0);
		add_resource("files/shapes/5", "static/paperdol.vga", 0);

		add_resource("palettes/count", 0, 14);
		add_resource("palettes/0", "static/palettes.flx", 0);
		add_resource("palettes/1", "static/palettes.flx", 1);
		add_resource("palettes/2", "static/palettes.flx", 2);
		add_resource("palettes/3", "static/palettes.flx", 3);
		add_resource("palettes/4", "static/palettes.flx", 4);
		add_resource("palettes/5", "static/palettes.flx", 5);
		add_resource("palettes/6", "static/palettes.flx", 6);
		add_resource("palettes/7", "static/palettes.flx", 7);
		add_resource("palettes/8", "static/palettes.flx", 8);
		add_resource("palettes/9", "static/palettes.flx", 9);
		add_resource("palettes/10", "static/palettes.flx", 10);
		add_resource("palettes/11", "static/palettes.flx", 11);
		add_resource("palettes/12", "static/palettes.flx", 12);
		add_resource("palettes/13", "static/mainshp.flx", 1);
		add_resource("palettes/14", "static/mainshp.flx", 26);

		if (!gwin->setup_siintro_fonts())
			gwin->abort ("Unable to setup font from 'intro.dat' file.");
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
		char	*fli_b;
		char	*buf;
		Uint8	*buffer;
		size_t	size;
		int	i,j;

		bool speech = audio->is_speech_enabled();
		
		audio->stop_music();

		// Lord British presents...
		U7object lbflic("static/intro.dat", 0);
		lbflic.retrieve(&fli_b, flisize);
		playfli fli0(fli_b+8, flisize-8);
		fli0.info();

		const char *txt_msg[] = { "& Jeff Freedman, Dancer Vesperman,", 
				"Willem Jan Palenstijn, Tristan Tarrant,", 
				"Max Horn, Luke Dunstan, Ryan Nunn",
				"Driven by the Exult game engine V" VERSION };

		for (j = 0; j < 20; j++)
		{
			next = fli0.play(win, 0, 0, next, j*5);
			for(i=0; i<3; i++) {
				gwin->paint_text(0, txt_msg[i], centerx-gwin->get_text_width(0, txt_msg[i])/2, centery+50+15*i);
			}
			win->show();
		}


		next = fli0.play(win, 0, 0, next, 100);
		for(i=0; i<3; i++) {
			gwin->paint_text(0, txt_msg[i], centerx-gwin->get_text_width(0, txt_msg[i])/2, centery+50+15*i);
		}
		win->show();

		for (i = 0; i < 300; i++)
			if (wait_delay (10))
			{
				delete [] fli_b;
				return;
			}


		for (j = 20; j; j--)
		{
			next = fli0.play(win, 0, 0, next, j*5);
			for(i=0; i<3; i++) {
				gwin->paint_text(0, txt_msg[i], centerx-gwin->get_text_width(0, txt_msg[i])/2, centery+50+15*i);
			}
			win->show();
		}


		delete [] fli_b;


		if (wait_delay (0))
			return;
		
		// Castle Outside
		// No sound... yet, can't decode it :(

		// Start Music
		audio->start_music ("static/r_sintro.xmi", 0, false);


		U7object flic("static/intro.dat", 1);
		flic.retrieve(&fli_b, flisize);
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
			{
				delete [] fli_b;
				return;
			}

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
				gwin->paint_text(SIINTRO_FONT1, db_cas, centerx-gwin->get_text_width(SIINTRO_FONT1, db_cas)/2, centery+50);
			else 
				gwin->paint_text(SIINTRO_FONT1, lb_cas, centerx-gwin->get_text_width(SIINTRO_FONT1, lb_cas)/2, centery+50);

			prev = num;
			next += 75;
			win->show();
			if (wait_delay (1))
			{
				delete [] fli_b;
				return;
			}

		}

		for (j = 0; j < 10; j++)
		{
			num = get_frame();
			if (prev != num)
				for (i = 0; i < num+1; i++)
					fli1.play(win, i, i, next);

			prev = num;
			next += 75;
			win->show();
			if (wait_delay (1))
			{
				delete [] fli_b;
				return;
			}

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
				gwin->paint_text(SIINTRO_FONT1, bg_fellow[i], centerx-gwin->get_text_width(SIINTRO_FONT1, bg_fellow[i])/2, centery+50+15*i);
			}

			prev = num;
			next += 75;
			win->show();
			if (wait_delay (1))
			{
				delete [] fli_b;
				return;
			}

		}

		for (j = 20; j; j--)
		{
			next = fli1.play(win, 0, 0, next, j*5);
			win->show();
			if (wait_delay (0))
			{
				delete [] fli_b;
				return;
			}

		}
		delete [] fli_b;

		if (wait_delay (0))
			return;


		// Guard walks in
		U7object flic2("static/intro.dat", 2);
		flic2.retrieve(&fli_b, flisize);
		playfli fli2(fli_b+8, flisize-8);
		fli2.info();

		for (j = 0; j < 20; j++)
		{
			next = fli2.play(win, 0, 0, next, j*5);
			win->show();
			if (wait_delay (0))
			{
				delete [] fli_b;
				return;
			}
		}

		// Guard walks in
		for (j = 0; j < 37; j++)
		{
			next = fli2.play(win, j, j, next);
			win->show();
			if (wait_delay (0))
			{
				delete [] fli_b;
				return;
			}
		}

		// Guard walks in
		const char *my_leige = "My leige";
		const char *yo_homes = "Yo, homes";

		if (speech && !jive)
		{
			U7object voc_my_leige("static/intro.dat", 16);
			voc_my_leige.retrieve (&buf, size);
			buffer = (Uint8 *) buf;
			audio->play (buffer+8, size-8, false);
			delete [] buffer;
		}


		for (; j < 55; j++)
		{
			next = fli2.play(win, j, j, next);

			if (jive)
				gwin->paint_text(SIINTRO_FONT1, yo_homes, centerx+30, centery+87);
			else if (!speech)
				gwin->paint_text(SIINTRO_FONT1, my_leige, centerx+30, centery+87);

			win->show();
			if (wait_delay (0))
			{
				delete [] fli_b;
				return;
			}
		}

		next = fli2.play(win, j, j, next);
		win->show();

		const char *all_we[2] = { "All we found among Batlin's",
					"belongings was this enchanted scroll.." };

		if (speech && !jive)
		{
			U7object voc_all_we("static/intro.dat", 17);
			voc_all_we.retrieve (&buf, size);
			buffer = (Uint8 *) buf;
			audio->play (buffer+8, size-8, false);
			delete [] buf;
		}

		for (; j < 73; j++)
		{
			next = fli2.play(win, j, j, next);

			if (!speech || jive)
			{
				gwin->paint_text(SIINTRO_FONT1, all_we[0], centerx+150-gwin->get_text_width(SIINTRO_FONT1, all_we[0]), centery+74);
				gwin->paint_text(SIINTRO_FONT1, all_we[1], centerx+160-gwin->get_text_width(SIINTRO_FONT1, all_we[1]), centery+87);
			}

			win->show();
			if (wait_delay (0))
			{
				delete [] fli_b;
				return;
			}
		}
		for (i = 0; i < 220; i++)
			if (wait_delay (10))
			{
				delete [] fli_b;
				return;
			}

		const char *and_a[2] = { "and a map showing the way to",
					"a place called the Serpent Isle" };

		next = fli2.play(win, j, j, next);

		if (!speech || jive)
		{
			gwin->paint_text(SIINTRO_FONT1, and_a[0], centerx+150-gwin->get_text_width(SIINTRO_FONT1, and_a[0]), centery+74);
			gwin->paint_text(SIINTRO_FONT1, and_a[1], centerx+150-gwin->get_text_width(SIINTRO_FONT1, and_a[1]), centery+87);
		}

		win->show();
		j++;
		
		for (i = 0; i < 290; i++)
			if (wait_delay (10))
			{
				delete [] fli_b;
				return;
			}


		fli2.play(win, j, j);
		j++;

		for (i = 0; i < 50; i++)
			if (wait_delay (10))
			{
				delete [] fli_b;
				return;
			}

		const char *indeed[2] = { "Indeed.",
					"Put it on the table." };

		const char *iree = { "Iree. Slap it down there!" };

		if (speech && !jive)
		{
			U7object voc_indeed("static/intro.dat", 18);
			voc_indeed.retrieve (&buf, size);
			buffer = (Uint8 *) buf;
			audio->play (buffer+8, size-8, false);
			delete [] buf;
		}

		next = fli2.play(win, j, j);
		j++;
		
		for (; j < 81; j++)
		{
			next = fli2.play(win, j, j, next);

			if (jive)
				gwin->paint_text(SIINTRO_FONT1, iree, topx+40, centery+74);
			else if (!speech)
			{
				gwin->paint_text(SIINTRO_FONT1, indeed[0], topx+40, centery+74);
				gwin->paint_text(SIINTRO_FONT1, indeed[1], topx+40, centery+87);
			}

			win->show();
			if (wait_delay (0))
			{
				delete [] fli_b;
				return;
			}
		}

		for (i = 0; i < 200; i++)
			if (wait_delay (10))
			{
				delete [] fli_b;
				return;
			}

		delete [] fli_b;

		// Scroll opens
		U7object flic3("static/intro.dat", 3);
		flic3.retrieve(&fli_b, flisize);
		playfli fli3(fli_b+8, flisize-8);
		fli3.info();

		next = 0;

		// Scroll opens
		for (j = 0; j < 20; j++)
		{
			next = fli3.play(win, j, j, next)+20;
			win->show();
			if (wait_delay (0))
			{
				delete [] fli_b;
				return;
			}
		}


		// 'Stand Back'
		const char *stand_back = "Stand Back!";
		const char *jump_back = "Jump Back!";

		if (speech && !jive)
		{
			U7object voc_stand_back("static/intro.dat", 19);
			voc_stand_back.retrieve (&buf, size);
			buffer = (Uint8 *) buf;
			audio->play (buffer+8, size-8, false);
			delete [] buffer;
		}

		for (; j < 61; j++)
		{
			next = fli3.play(win, j, j, next)+20;

			if (jive)
				gwin->paint_text(SIINTRO_FONT1, jump_back, topx+70, centery+60);
			else if (!speech)	
				gwin->paint_text(SIINTRO_FONT1, stand_back, topx+70, centery+60);

			win->show();
			if (wait_delay (0))
			{
				delete [] fli_b;
				return;
			}
		}

		delete [] fli_b;


		// Big G speaks
		U7object flic4("static/intro.dat", 4);
		flic4.retrieve(&fli_b, flisize);
		playfli fli4(fli_b+8, flisize-8);
		fli4.info();

		U7object shapes("static/intro.dat", 30);
		size_t	shapesize;
		char *	shape_buf;
		shapes.retrieve(&shape_buf, shapesize);
		BufferDataSource gshape_ds(shape_buf+8, shapesize-8);
		Shape_frame *sf;
	
		Shape_file gshape(gshape_ds);
		
		cout << "Shape in intro.dat has " << gshape.get_num_frames() << endl;

		if (speech && !jive)
		{
			U7object voc_big_g("static/intro.dat", 20);
			voc_big_g.retrieve (&buf, size);
			buffer = (Uint8 *) buf;
			audio->play (buffer+8, size-8, false);
			delete [] buffer;
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
				gwin->paint_shape (centerx-36, centery, sf);

			if (j < 100 && jive)
			{
				gwin->paint_text(SIINTRO_FONT1, batlin2[0], centerx-gwin->get_text_width(SIINTRO_FONT1, batlin2[0])/2, centery+74);
				gwin->paint_text(SIINTRO_FONT1, batlin2[1], centerx-gwin->get_text_width(SIINTRO_FONT1, batlin2[1])/2, centery+87);
			}
			else if (j < 200 && jive)
			{
				gwin->paint_text(SIINTRO_FONT1, you_must[0], centerx-gwin->get_text_width(SIINTRO_FONT1, you_must[0])/2, centery+74);
				gwin->paint_text(SIINTRO_FONT1, you_must[1], centerx-gwin->get_text_width(SIINTRO_FONT1, you_must[1])/2, centery+87);
			}
			else if (j < 300 && jive)
			{
				gwin->paint_text(SIINTRO_FONT1, soon_i[0], centerx-gwin->get_text_width(SIINTRO_FONT1, soon_i[0])/2, centery+74);
				gwin->paint_text(SIINTRO_FONT1, soon_i[1], centerx-gwin->get_text_width(SIINTRO_FONT1, soon_i[1])/2, centery+87);
			}
			else if (j < 100 && !speech)
			{
				gwin->paint_text(SIINTRO_FONT1, batlin[0], centerx-gwin->get_text_width(SIINTRO_FONT1, batlin[0])/2, centery+74);
				gwin->paint_text(SIINTRO_FONT1, batlin[1], centerx-gwin->get_text_width(SIINTRO_FONT1, batlin[1])/2, centery+87);
			}
			else if (j < 200 && !speech)
			{
				gwin->paint_text(SIINTRO_FONT1, you_shall[0], centerx-gwin->get_text_width(SIINTRO_FONT1, you_shall[0])/2, centery+74);
				gwin->paint_text(SIINTRO_FONT1, you_shall[1], centerx-gwin->get_text_width(SIINTRO_FONT1, you_shall[1])/2, centery+87);
			}
			else if (j < 300 && !speech)
			{
				gwin->paint_text(SIINTRO_FONT1, there_i[0], centerx-gwin->get_text_width(SIINTRO_FONT1, there_i[0])/2, centery+74);
				gwin->paint_text(SIINTRO_FONT1, there_i[1], centerx-gwin->get_text_width(SIINTRO_FONT1, there_i[1])/2, centery+87);
			}

			win->show();
			if (wait_delay (0))
			{
				delete [] fli_b;
				delete [] shape_buf;
				return;
			}
		}
		sf = gshape.get_frame(0);

		for (j = 20; j; j--)
		{
			next = fli4.play(win, 0, 0, next, j*5);
			if (sf)
				gwin->paint_shape (centerx-36, centery, sf);

			win->show();
			if (wait_delay (0))
			{
				delete [] shape_buf;
				delete [] fli_b;
				return;
			}
		}

		delete [] shape_buf;
		delete [] fli_b;
		
		// Tis LBs's Worst fear
		U7object flic5("static/intro.dat", 5);
		flic5.retrieve(&fli_b, flisize);
		playfli fli5(fli_b+8, flisize-8);
		fli5.info();

		for (j=0; j < 20; j++)
		{
			next = fli5.play(win, 0, 0, next, j*5);

			win->show();
			if (wait_delay (0))
			{
				delete [] fli_b;
				return;
			}
		}


		const char *tis_my[3] = {"'Tis my worst fear!",
					"I must send the Avatar through",
					"the pilars to the Serpent Isle!" };


		if (speech && !jive)
		{
			U7object voc_tis_my("static/intro.dat", 21);
			voc_tis_my.retrieve (&buf, size);
			buffer = (Uint8 *) buf;
			audio->play (buffer+8, size-8, false);
			delete [] buffer;
		}

		for (j=0; j < 61; j++)
		{
			next = fli5.play(win, j, j, next)+30;

			if (j < 20 && (!speech || jive))
			{
				gwin->paint_text(SIINTRO_FONT1, tis_my[0], centerx-gwin->get_text_width(SIINTRO_FONT1, tis_my[0])/2, centery+74);
			}
			else if (j > 22 && (!speech || jive))
			{
				gwin->paint_text(SIINTRO_FONT1, tis_my[1], centerx-gwin->get_text_width(SIINTRO_FONT1, tis_my[1])/2, centery+74);
				gwin->paint_text(SIINTRO_FONT1, tis_my[2], centerx-gwin->get_text_width(SIINTRO_FONT1, tis_my[2])/2, centery+87);
			}

			win->show();
			if (wait_delay (0))
			{
				delete [] fli_b;
				return;
			}
		}

		delete [] fli_b;


		// Boat 1
		U7object flic6("static/intro.dat", 6);
		flic6.retrieve(&fli_b, flisize);
		playfli fli6(fli_b+8, flisize-8);
		fli6.info();

		for (j=0; j < 61; j++)
		{
			next = fli6.play(win, j, j, next)+30;

			win->show();
			if (wait_delay (0))
			{
				delete [] fli_b;
				return;
			}
		}

		delete [] fli_b;


		// Boat 2
		U7object flic7("static/intro.dat", 7);
		flic7.retrieve(&fli_b, flisize);
		playfli fli7(fli_b+8, flisize-8);
		fli7.info();

		const char *zot = "Zot!";

		for (j=0; j < 61; j++)
		{
			next = fli7.play(win, j, j, next)+30;

			if (j > 55 && jive)
				gwin->paint_text(SIINTRO_FONT1, zot, centerx-gwin->get_text_width(SIINTRO_FONT1, zot)/2, centery+74);

			win->show();
			if (wait_delay (0))
			{
				delete [] fli_b;
				return;
			}
		}

		delete [] fli_b;


		// Ultima VII Part 2
		U7object flic8("static/intro.dat", 8);
		flic8.retrieve(&fli_b, flisize);
		playfli fli8(fli_b+8, flisize-8);
		fli8.info();

		for (j = 0; j < 20; j++)
		{
			next = fli8.play(win, 0, 0, next, j*5);
			gwin->paint_text(7, txt_msg[3], centerx-gwin->get_text_width(7, txt_msg[3])/2, centery+75);
			win->show();
		}


		next = fli8.play(win, 0, 0, next, 100);
		gwin->paint_text(7, txt_msg[3], centerx-gwin->get_text_width(7, txt_msg[3])/2, centery+75);
		win->show();

		for (i = 0; i < 300; i++)
			if (wait_delay (10))
			{
				delete [] fli_b;
				return;
			}


		for (j = 20; j; j--)
		{
			next = fli8.play(win, 0, 0, next, j*5);
			gwin->paint_text(7, txt_msg[3], centerx-gwin->get_text_width(7, txt_msg[3])/2, centery+75);
			win->show();
		}


		delete [] fli_b;

	}

void SI_Game::top_menu()
{
	play_midi(28, true);
	gwin->paint_shape(topx,topy,menushapes.get_shape(0x2,0));
	pal.load("static/mainshp.flx",26);
	pal.fade_in(60);	
}
	
void SI_Game::end_game(bool success) 
	{
		size_t	flisize;
		char	*fli_b;

		audio->start_music ("static/r_send.xmi", 0, false);
		
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
		play_midi(32);
		vector<char *> *text = load_text("static/mainshp.flx", 0x10);
		scroll_text(text);
		destroy_text(text);
	}

void SI_Game::show_credits()
	{
		play_midi(30);
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
					if(sex>5)
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
		{
			set_avname (npc_name);
			set_avsex (1-(sex%2));
			set_avskin (sex/2);
			gwin->init_gamedat(true);
		}
		win->fill8(0,gwin->get_width(),90,0,menuy);
		return ok;
	}
