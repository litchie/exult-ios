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
 
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "SDL_events.h"
#include "gamewin.h"
#include "Audio.h"
#include "soundtest.h"
#include "Scroll_gump.h"
#include "mouse.h"
#include "font.h"

extern void make_screenshot(bool silent=false);
 
SoundTester::SoundTester() : song(0), sfx(0), voice(0), active(0), repeat(true)
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
	Image_buffer8 *ibuf = gwin->get_win()->get_ib8();
	Font *font = gwin->get_font(4);

	Audio *audio = Audio::get_ptr();
	Scroll_gump *scroll = NULL;

	char buf[256];
	bool looping = true;
	bool redraw = true;
	SDL_Event event;

	int centerx = gwin->get_width()/2;
	int centery = gwin->get_height()/2;
	int left = centerx - 65;
	int first_line = centery-53;
	int line;
	int height = 6;
	int width = 6;
	
	Mouse::mouse->hide();
	
	do
	{
		if (redraw)
		{
		     
			scroll = new Scroll_gump();
			scroll->add_text(" ~");
			scroll->paint(gwin);

			line = first_line;
			font->paint_text_fixedwidth(ibuf, "Sound Tester", left, line, width);
			line += height;
			font->paint_text_fixedwidth(ibuf, "------------", left, line, width);

			line += height*2;
			font->paint_text_fixedwidth(ibuf, "   Up - Previous Type", left, line, width);
			
			line += height;
			font->paint_text_fixedwidth(ibuf, " Down - Next Type", left, line, width);
			
			line += height;
			font->paint_text_fixedwidth(ibuf, " Left - Previous Number", left, line, width);
			
			line += height;
			font->paint_text_fixedwidth(ibuf, "Right - Next Number", left, line, width);

			line += height;
			font->paint_text_fixedwidth(ibuf, "Enter - Play it", left, line, width);

			line += height;
			font->paint_text_fixedwidth(ibuf, "  ESC - Leave", left, line, width);

			line += height;
			font->paint_text_fixedwidth(ibuf, "    R - Repeat Music", left, line, width);

			line += height;
			font->paint_text_fixedwidth(ibuf, "    S - Stop Music", left, line, width);


			snprintf (buf, 256, "%2s Music %c %3i %c %s",
					active==0?"->":"",
					active==0?'<':' ',
					song,
					active==0?'>':' ',
					repeat?"- Repeat":"");
			line += height*2;
			font->paint_text_fixedwidth(ibuf, buf, left, line, width);
			
			
			snprintf (buf, 256, "%2s SFX   %c %3i %c",
					active==1?"->":"",
						active==1?'<':' ',
					sfx,
					active==1?'>':' ');

			line += height*2;
			font->paint_text_fixedwidth(ibuf, buf, left, line, width);


			snprintf (buf, 256, "%2s Voice %c %3i %c",
					active==2?"->":"",
					active==2?'<':' ',
					voice,
					active==2?'>':' ');

			line += height*2;
			font->paint_text_fixedwidth(ibuf, buf, left, line, width);

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
					audio->start_music (song, repeat, 0);
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
				
			case SDLK_r:
				repeat = repeat?false:true;
				break;
			case SDLK_s:
				if ((event.key.keysym.mod & KMOD_ALT) && (event.key.keysym.mod & KMOD_CTRL)) 
					make_screenshot(true);
				else
					audio->stop_music();
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
