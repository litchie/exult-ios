/**
 **	Actors.cc - Game actors.
 **
 **	Written: 11/3/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <iostream.h>			/* Debugging. */
#include <stdlib.h>
#include "actors.h"
#include "convers.h"
#include "imagewin.h"

/*
 *	Set default set of frames.
 */

void Actor::set_default_frames
	(
	)
	{
					// Set up actor's frame lists.
					// These are rough guesses.
	static unsigned char 	north_frames[3] = {0, 1, 2},
				south_frames[3] = {16, 17, 18},
//				east_frames[3] = {3, 4, 5},
				east_frames[3] = {7, 8, 9},
//				west_frames[3] = {19, 20, 21};
				west_frames[3] = {23, 24, 25};
	set_frame_sequence(north, 3, north_frames);
	set_frame_sequence(south, 3, south_frames);
	set_frame_sequence(east, 3, east_frames);
	set_frame_sequence(west, 3, west_frames);
	}

/*
 *	Kill an actor.
 */

Npc_actor::~Npc_actor
	(
	)
	{
	delete npc;
	delete sentences;
	}

/*
 *	Set personality.
 */

void Npc_actor::set_npc
	(
	Npc *n
	)
	{
	delete npc;
	npc = n;
	npc->set_user(this);		// Set user-data to ourselves.
	}

/*
 *	Get usecode function to run.
 */

int Npc_actor::get_usecode
	(
	)
	{
	return usecode;
	}

/*
 *	Print the sentences we'll respond to.
 *
 *	Output:	0 if we don't converse.
 */

int Npc_actor::converse
	(
	Image_window *win,		// Window to draw in.
	Font_face *font,		// Font to use.
	Rectangle& box			// Where to write on screen.
	)
	{
	if (!npc)
		return (0);
					// Create new list.
	int maxcnt = npc->get_max_sentence_cnt();
	delete sentences;
	sentences = new Npc_sentence[maxcnt + 1];
	int cnt = 0;
	Sentence *sent;
	int id;				// Get them.
	Npc_sentence_iterator next(npc);
	int x = 0, y = 0;		// Keep track of coords. in box.
	int height = win->get_text_height(font);
	int space_width = win->get_text_width(font, "   ");
	while (next(sent, id))
		{
		char *text = sent->get_text();
		int width = win->get_text_width(font, text);
		if (x > 0 && x + width > box.w)
			{		// Start a new line.
			x = 0;
			y += height;
			}
					// Store info.
		sentences[cnt].loc = Rectangle(box.x + x, box.y + y,
					width, height);
		sentences[cnt].id = id;
		win->draw_text(font, text, box.x + x, box.y + y);
		x += width + space_width;
		cnt++;
		}
	sentences[cnt].id = -1;		// Put -1 in last one.
	return (1);
	}

/*
 *	Start conversation.
 */

void Npc_actor::start_conversation
	(
	Image_window *win,		// Window to draw in.
	Font_face *font,		// Font to use.
	Rectangle& box			// Where to write on screen.
	)
	{
	our_win = win;
	our_face = font;
	our_box = box;			// Set area to write our speech in.
	npc->start();
	}

/*
 *	Respond to a mouse click from the user on a sentence.
 *
 *	Output:	0 to end conversation (User said "Bye").
 */

int Npc_actor::respond
	(
	Image_window *win,		// Window to draw in.
	Font_face *font,		// Font to use.
	Rectangle& box,			// Where to write on screen.
	int x, int y			// Mouse location.
	)
	{
	int i;				// Locate sentence clicked on.
	for (i = 0; sentences[i].id != -1 &&
			!sentences[i].loc.has_point(x, y); i++)
		;
	if (sentences[i].id == -1)	// Missed them all.
		return (1);
//+++++++++++++++These are already set in start_conversation().
	our_win = win;
	our_face = font;
	our_box = box;			// Set area to write our speech in.
	return (npc->respond_to_sentence(sentences[i].id));
	}

/*
 *	Print response.  This is called from an action.
 */

void Npc_actor::show_response
	(
	char *msg
	)
	{
	our_win->draw_text_box(our_face, msg, our_box.x, our_box.y,
						our_box.w, our_box.h);
	}

/*
 *	Figure where the sprite will be in the next frame.
 *
 *	Output:	0 if don't need to move.
 */

int Area_actor::next_frame
	(
	timeval& time,			// Current time.
	int& new_cx, int& new_cy,	// New chunk coords. returned.
	int& new_sx, int& new_sy,	// New shape coords. returned.
	int& next_frame			// Next frame # returned.
	)
	{
					// See if we should change motion.
	if (time.tv_sec < next_change.tv_sec)
		return (Actor::next_frame(time, new_cx, new_cy,
						new_sx, new_sy, next_frame));
	if (is_moving())
		{
		stop();
		new_cx = get_cx();
		new_cy = get_cy();
		new_sx = get_shape_pos_x();
		new_sy = get_shape_pos_y();
		next_frame = get_framenum();
					// Wait 0 to 5 seconds.
		next_change.tv_sec = time.tv_sec +
			(long) (5.0*rand()/(RAND_MAX + 1.0));
		return (1);
		}
	else
		{			// Where should we aim for?
		long deltax = 64 - 
			(1 + (long) (128.0*rand()/(RAND_MAX + 1.0)));
		long deltay = 64 - 
			(1 + (long) (128.0*rand()/(RAND_MAX + 1.0)));
		long newx = get_worldx() + deltax;
		long newy = get_worldy() + deltay;
					// Move every 1/4 second.
		start(newx, newy, 250000);
					// Go 1 to 4 seconds.
		next_change.tv_sec = time.tv_sec + 1 +
			(long) (4.0*rand()/(RAND_MAX + 1.0));
		return (0);
		}
	}
