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

#include "../alpha_kludges.h"

#include "gamewin.h"
#include "Audio.h"
#include "soundtest.h"
#include "Scroll_gump.h"
#include "mouse.h"
 
SoundTester::SoundTester()
	{
	}

SoundTester::~SoundTester()
	{
	}

static void handle_key(int shift, int& value, int max, int amt = 1)
{
	if(shift)
		value -= amt;
	else
		value += amt;
	if(value<0)
		value = max-1;
	else if(value>=max)
		value = 0;
}
	
void SoundTester::test_sound()
{
		
	Game_window *gwin = Game_window::get_game_window();
	Audio *audio = Audio::get_ptr();
	Scroll_gump *scroll = NULL;

	int song = 0;
	int sfx = 0;
	int voice = 0;
	int active = 0;

	char buf[255];
	bool looping = true;
	bool redraw = true;
	SDL_Event event;
	//int active;

	Mouse::mouse->hide();
	
	do
	{
		        if (redraw)
		        {
			scroll = new Scroll_gump();
			
			scroll->add_text("Sound Tester\n");
			
			scroll->add_text("Keys\n");
			
			scroll->add_text("Up - Previous Type\n"
					 "Down - Next Type\n"
					 "Left - Previous Number\n"
					 "Right - Next Number\n"
					 "Enter - Play it\n");

			sprintf (buf, "%2s Music %c %3i %c~",
				active==0?"->":"",
				active==0?'<':' ',
				song,
				active==0?'>':' ');
				
			scroll->add_text(buf);

			sprintf (buf, "%2s SFX   %c %3i %c~",
				active==1?"->":"",
				active==1?'<':' ',
				sfx,
				active==1?'>':' ');
			scroll->add_text(buf);

			sprintf (buf, "%2s Voice %c %3i %c~",
				active==2?"->":"",
				active==2?'<':' ',
				voice,
				active==2?'>':' ');
			scroll->add_text(buf);


			scroll->paint(gwin);
			gwin->show();
			delete scroll;
			redraw = false;
		}
			SDL_WaitEvent(&event);
		if(event.type==SDL_KEYDOWN)
		{
       			redraw = true;
			int shift = event.key.keysym.mod & KMOD_SHIFT;
			//int ctrl = event.key.keysym.mod & KMOD_CTRL;
			switch(event.key.keysym.sym)
			{

			case SDLK_ESCAPE:
				looping = false;
				break;

			case SDLK_RETURN:
				if (active == 0)
				{
					audio->stop_music();
					audio->start_music (song, false, 0);
				}
				else if (active == 1)
				{
					audio->play_sound_effect (sfx);
				}
				else if (active == 2)
				{
					audio->cancel_streams();
					audio->start_speech (voice, false);
				}
				break;
				
			case SDLK_UP:
				active = (active + 2) % 3;
				break;
			case SDLK_DOWN:
				active = (active + 1) % 3;
				break;
			case SDLK_LEFT:
				if (active == 0)
				{
					song--;
					if (song < 0) song = 255;
				}
				else if (active == 1)
				{
					sfx--;
					if (sfx < 0) sfx = 255;
				}
				else if (active == 2)
				{
					voice--;
					if (voice < 0) voice = 255;
				}
				break;
			case SDLK_RIGHT:
				if (active == 0)
				{
					song++;
					if (song > 255) song = 0;
				}
				else if (active == 1)
				{
					sfx++;
					if (sfx > 255) sfx = 0;
				}
				else if (active == 2)
				{
					voice++;
					if (voice > 255) voice = 0;
				}
				break;
			default:
				break;
			}
		}
	} while(looping);
	
	gwin->paint();
	Mouse::mouse->show();
	gwin->show();
}
