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

Game::Game()
	{
		gwin = Game_window::get_game_window();
		win = gwin->get_win();
	}

Game::~Game()
	{
	}

Game *Game::get_game(const char *static_identity)
	{

	Exult_Game game_type;

	if((!strcmp(static_identity,"ULTIMA7"))||(!strcmp(static_identity,"FORGE")))
                game_type = BLACK_GATE;
        else if((!strcmp(static_identity,"SERPENT ISLE"))||(!strcmp(static_identity,"SILVER SEED")))
                game_type = SERPENT_ISLE;

	switch(game_type) {
	case BLACK_GATE:
		return new BG_Game();
	case SERPENT_ISLE:
		return new SI_Game();
	default:
		return 0;
	}
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
