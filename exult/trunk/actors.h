/**
 **	Actors.h - Game actors.
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

#ifndef INCL_ACTORS
#define INCL_ACTORS	1

#include "objs.h"
#include "npc.h"

class Npc;
class Image_window;
class Font_face;

/*
 *	An actor:
 */
class Actor : public Sprite
	{
	Actor *next, *prev;		// Next, prev. in vicinity.
	int face_shapenum;		// Portrait shape #, or -1.
public:
	void set_default_frames();	// Set usual frame sequence.
	Actor(int shapenum, int fshape = -1) : Sprite(shapenum),
					face_shapenum(fshape)
		{
		next = prev = this;
		set_default_frames();
		}
	int get_face_shapenum()		// Get "portrait" shape #.
		{ return face_shapenum; }
	void add_after_this(Actor *a)	// Add another actor after this.
		{
					// Remove from current chain.
		a->next->prev = a->prev;
		a->prev->next = a->next;
		a->next = next;		// Add to this one.
		a->prev = this;
		next->prev = a;
		next = a;
		}
	Actor *get_next()
		{ return next; }			
	};

/*
 *	Here's a sentence that you can click on to send to an NCP:
 */
class Npc_sentence
	{
public:
	Npc_sentence() : id(0), loc()
		{  }
	Rectangle loc;			// Location on screen.
	int id;				// Sentence ID #.
	};

/*
 *	A non-player-character that one can converse with:
 */
class Npc_actor : public Actor, public Npc_user
	{
	Npc *npc;			// It's "personality".
	Npc_sentence *sentences;	// Set during conversation.
	Image_window *our_win;		// Window to write to.
	Font_face *our_face;		// Font to write with.
	Rectangle our_box;		// Where to write our responses.
	int usecode;			// # of usecode function.
public:
	Npc_actor(int shapenum, int fshape = -1, int uc = -1)
			: Actor(shapenum, fshape), npc(0), sentences(0),
				usecode(uc)
		{  }
	~Npc_actor();
	void set_npc(Npc *n);
	Npc *get_npc()
		{ return npc; }
	virtual int get_usecode();	// Get usecode function to run.
					// Print sentences we'll respond to.
	int converse(Image_window *win, Font_face *face, Rectangle& box);
					// Start conversation.
	void start_conversation(Image_window *win, Font_face *face, 
					Rectangle& box);
					// Respond to mouse click.
	int respond(Image_window *win, Font_face *face, Rectangle& box,
							int x, int y);
					// Print our response on screen.
	virtual void show_response(char *msg);
	};

/*
 *	Here's an actor that's just hanging around an area.
 */
class Area_actor : public Npc_actor
	{
	timeval next_change;		// When to change motion.
public:
	Area_actor(int shapenum, int fshape = -1) : Npc_actor(shapenum, fshape)
		{
		next_change.tv_sec = next_change.tv_usec = 0;
		}
					// Figure next frame location.
	virtual int next_frame(timeval& time,
		int& new_cx, int& new_cy, int& new_sx, int& new_sy,
		int& new_frame);
	};

#endif
