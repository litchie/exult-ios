/*
Copyright (C) 2001  The Exult Team

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

#include "../alpha_kludges.h"

#include "conversation.h"
#include "useval.h"
#include "gamewin.h"
#include "actors.h"
#include "mouse.h"
#include "game.h"

using std::cout;
using std::endl;
using std::strcpy;
using std::string;
using std::vector;

//TODO: show_face & show_avatar_choices seem to share code?
//TODO: show_avatar_choices shouldn't first convert to char**, probably

/*
 *	Store information about an NPC's face and text on the screen during
 *	a conversation:
 */
class Npc_face_info {
 public:
  int shape;			// NPC's shape #.
  bool text_pending;	// Text has been written, but user
  //   has not yet been prompted.
  Rectangle face_rect;		// Rectangle where face is shown.
  Rectangle text_rect;		// Rectangle NPC statement is shown in.
  int last_text_height;		// Height of last text painted.
  Npc_face_info(int sh) : shape(sh), text_pending(0)
  {  }
};

Conversation::Conversation() :
  num_faces(0), last_face_shown(0), conv_choices(0)
{
  gwin = Game_window::get_game_window();

  const int max_faces = sizeof(face_info)/sizeof(face_info[0]);
  for (int i = 0; i < max_faces; i++)
    face_info[i] = 0;
}

Conversation::~Conversation()
{
  delete [] conv_choices;
}


void Conversation::clear_answers(void)
{
	answers.clear();
}

void Conversation::add_answer(const char *str)
{
	remove_answer(str);
	string s(str);
	answers.push_back(s);
}

/*
 *	Add an answer to the list.
 */

void Conversation::add_answer(Usecode_value& val)
{
	const char *str;
	int size = val.get_array_size();
	if (size)			// An array?
		{
		for (int i = 0; i < size; i++)
			add_answer(val.get_elem(i));
		}
	else if ((str = val.get_str_value()) != 0)
		add_answer(str);
}

void Conversation::remove_answer(const char *str)
{
	vector<string>::iterator it;

	for(it=answers.begin();	it!=answers.end(); ++it)
		if(*it==str)
			break;

	if(it!=answers.end())
		answers.erase(it);
}

/*
 *	Remove an answer from the list.
 */

void Conversation::remove_answer(Usecode_value& val)
{
	const char *str;
	if (val.is_array()) {
		int size = val.get_array_size();
		for (int i=0; i < size; i++) {
			str = val.get_elem(i).get_str_value();
			if (str) remove_answer(str);
		}
	} else {
		str = val.get_str_value();
		remove_answer(str);
	}
}

/*
 *	Initialize face list.
 */

void Conversation::init_faces()
{
	const int max_faces = sizeof(face_info)/sizeof(face_info[0]);
	for (int i = 0; i < max_faces; i++)
		{
		if( face_info[i] )
			delete face_info[i];
		face_info[i] = 0;
		}
	num_faces = 0;
	last_face_shown = -1;
}



/*
 *	Show a "face" on the screen.  Npc_text_rect is also set.
 */

void Conversation::show_face(int shape, int frame, int slot)
{
	Main_actor* main_actor = gwin->get_main_actor();
	const int max_faces = sizeof(face_info)/sizeof(face_info[0]);

	// Make sure mode is set right.
      	gwin->set_mode(Game_window::conversation);

	// Petra?
	if (shape == 28 && main_actor->get_flag(Obj_flags::petra)) {
		shape = main_actor->get_face_shapenum();
		if (main_actor->get_skin_color() == 0) // WH
		{
			frame = 1 - main_actor->get_type_flag(Actor::tf_sex);
		}
		else if (main_actor->get_skin_color() == 1) // BN
		{
			frame = 3 - main_actor->get_type_flag(Actor::tf_sex);
		}
		else if (main_actor->get_skin_color() == 2) // BK
		{
			frame = 5 - main_actor->get_type_flag(Actor::tf_sex);
		}
		else // None
		{
			frame = main_actor->get_type_flag(Actor::tf_sex);
		}
	}
					// Get screen dims.
	int screenw = gwin->get_width(), screenh = gwin->get_height();
					// Get character's portrait.
	Shape_frame *face = gwin->get_face(shape, frame);
	Npc_face_info *info = 0;
	Rectangle actbox;		// Gets box where face goes.
					// See if already on screen.
	for (int i = 0; i < max_faces; i++)
		if (face_info[i] && face_info[i]->shape == shape)
			{
			info = face_info[i];
			last_face_shown = i;
			break;
			}
	if (!info)			// New one?
		{
		if (num_faces == max_faces)
			{
			cout << "Can't show more than " << max_faces << 
							" faces" << endl;
			return;
			}
		info = new Npc_face_info(shape);
		if (slot == -1)		// Want next one?
			slot = num_faces;
					// Get last one shown.
		Npc_face_info *prev = slot ? face_info[slot - 1] : 0;
		last_face_shown = slot;
		if (!face_info[slot])
			num_faces++;	// We're adding one (not replacing).
		else
			delete face_info[slot];
		face_info[slot] = info;
					// Get text height.
		int text_height = gwin->get_text_height(0);
					// Figure starting y-coord.
		int starty;
		if (prev)
			{
			starty = prev->text_rect.y + prev->last_text_height;
			if (starty < prev->face_rect.y + prev->face_rect.h)
				starty = prev->face_rect.y + prev->face_rect.h;
			starty += 2*text_height;
			if (starty + face->get_height() > screenh - 1)
				starty = screenh - face->get_height() - 1;
			}
		else
			starty = 1;
		actbox = gwin->clip_to_win(Rectangle(8, starty,
			face->get_width() + 4, face->get_height() + 4));
		info->face_rect = actbox;
					// This is where NPC text will go.
		info->text_rect = gwin->clip_to_win(Rectangle(
			actbox.x + actbox.w + 3, actbox.y + 3,
			screenw - actbox.x - actbox.w - 6, 4*text_height));
		info->last_text_height = info->text_rect.h;
		}
	else
		actbox = info->face_rect;
	gwin->get_win()->set_clip(0, 0, screenw, screenh);
					// Draw whom we're talking to.
	gwin->paint_shape(actbox.x + face->get_xleft(),
			actbox.y + face->get_yabove(), face);
	gwin->get_win()->clear_clip();
	}


/*
 *	Remove face from screen.
 */

void Conversation::remove_face(int shape)
{
	const int max_faces = sizeof(face_info)/sizeof(face_info[0]);
	int i;				// See if already on screen.
	for (i = 0; i < max_faces; i++)
		if (face_info[i] && face_info[i]->shape == shape)
			break;
	if (i == max_faces)
		return;			// Not found.
	Npc_face_info *info = face_info[i];
	gwin->paint(info->face_rect);
	gwin->paint(info->text_rect);
	delete face_info[i];
	face_info[i] = 0;
	num_faces--;
	if (last_face_shown == i)	// Just in case.
		last_face_shown = num_faces - 1;
}

/*
 *	Remove the last face shown (SI).  (Or maybe it's just slot 1 always?)
 */

void Conversation::remove_last_face
	(
	)
	{
	if (last_face_shown >= 0 && face_info[last_face_shown])
		remove_face(face_info[last_face_shown]->shape);
	}

/*
 *	Show what the NPC had to say.
 */

void Conversation::show_npc_message(const char *msg)
{
	extern int Get_click(int& x, int& y, Mouse::Mouse_shapes shape, 
								char *key = 0);
	if (last_face_shown == -1)
		return;
	Npc_face_info *info = face_info[last_face_shown];
	Rectangle& box = info->text_rect;
	gwin->paint(box);		// Clear what was there before.
	int height;			// Break at punctuation.
	while ((height = gwin->paint_text_box(0, msg, box.x,box.y,box.w,box.h, 
								-1, 1)) < 0)
		{			// More to do?
		int x, y; char c;
		Get_click(x, y, Mouse::hand, &c);
		gwin->paint(box);	// Clear area again.
		msg += -height;
		}
					// All fit?  Store height painted.
	info->last_text_height = height;
	info->text_pending = 1;
	gwin->set_painted();
	gwin->show();
}


/*
 *	Is there NPC text that the user hasn't had a chance to read?
 */

bool Conversation::is_npc_text_pending()
{
	const int max_faces = sizeof(face_info)/sizeof(face_info[0]);
	for (int i = 0; i < max_faces; i++)
		if (face_info[i] && face_info[i]->text_pending)
			return true;
	return false;
}

/*
 *	Clear text-pending flags.
 */

void Conversation::clear_text_pending()
{
	const int max_faces = sizeof(face_info)/sizeof(face_info[0]);
	for (int i = 0; i < max_faces; i++)	// Clear 'pending' flags.
		if (face_info[i])
			face_info[i]->text_pending = 0;
}

/*
 *	Show the Avatar's conversation choices (and face).
 */

void Conversation::show_avatar_choices(int num_choices,	char **choices)
{
	Main_actor *main_actor = gwin->get_main_actor();
	const int max_faces = sizeof(face_info)/sizeof(face_info[0]);
	gwin->set_mode(Game_window::conversation);
					// Get screen rectangle.
	Rectangle sbox = gwin->get_win_rect();
	int x = 0, y = 0;		// Keep track of coords. in box.
	int height = gwin->get_text_height(0);
	int space_width = gwin->get_text_width(0, " ");


					// Get main actor's portrait.
	int shape = main_actor->get_face_shapenum();
	int frame;

	if (main_actor->get_flag(Obj_flags::petra)) // Petra
	{
		shape = 28;
		frame = 0;
	}
	else if (main_actor->get_skin_color() == 0) // WH
	{
		frame = 1 - main_actor->get_type_flag(Actor::tf_sex);
	}
	else if (main_actor->get_skin_color() == 1) // BN
	{
		frame = 3 - main_actor->get_type_flag(Actor::tf_sex);
	}
	else if (main_actor->get_skin_color() == 2) // BK
	{
		frame = 5 - main_actor->get_type_flag(Actor::tf_sex);
	}
	else // None
	{
		frame = main_actor->get_type_flag(Actor::tf_sex);
	}

	Shape_frame *face = gwin->get_face(shape, frame);
	int empty;			// Find face prev. to 1st empty slot.
	for (empty = 0; empty < max_faces; empty++)
		if (!face_info[empty])
			break;
					// Get last one shown.
	Npc_face_info *prev = empty ? face_info[empty - 1] : 0;
	int fx = prev ? prev->face_rect.x + prev->face_rect.w + 4 : 16;
	int fy;
	if (Game::get_game_type()==SERPENT_ISLE)
	{
		fy = sbox.h - 2 - face->get_height();
		fx = 8;
	}
	else if (!prev)
		fy = sbox.h - face->get_height() - 3*height;
	else
		{
		fy = prev->text_rect.y + prev->last_text_height;
		if (fy < prev->face_rect.y + prev->face_rect.h)
			fy = prev->face_rect.y + prev->face_rect.h;
		fy += height;
		}
	Rectangle mbox(fx, fy, face->get_width(), face->get_height());
	mbox = mbox.intersect(sbox);
	avatar_face = mbox;		// Repaint entire width.
					// Draw portrait.
	gwin->paint_shape(mbox.x + face->get_xleft(), 
			  mbox.y + face->get_yabove(), face);
					// Set to where to draw sentences.
	Rectangle tbox(mbox.x + mbox.w + 8, mbox.y + 4,
				sbox.w - mbox.x - mbox.w - 16,
//				sbox.h - mbox.y - 16);
				5*height);// Try 5 lines.
	tbox = tbox.intersect(sbox);
	gwin->paint(tbox);              // Paint background.
	delete [] conv_choices;		// Set up new list of choices.
	conv_choices = new Rectangle[num_choices + 1];
	for (int i = 0; i < num_choices; i++)
		{
		char text[256];
		text[0] = 127;		// A circle.
		strcpy(&text[1], choices[i]);
		int width = gwin->get_text_width(0, text);
		if (x > 0 && x + width >= tbox.w)
			{		// Start a new line.
			x = 0;
			y += height - 1;
			}
					// Store info.
		conv_choices[i] = Rectangle(tbox.x + x, tbox.y + y,
					width, height);
		conv_choices[i] = conv_choices[i].intersect(sbox);
		avatar_face = avatar_face.add(conv_choices[i]);
		gwin->paint_text(0, text, tbox.x + x, tbox.y + y);
		x += width + space_width;
		}
	avatar_face.enlarge(6);		// Encloses entire area.
	avatar_face = avatar_face.intersect(sbox);
					// Terminate the list.
	conv_choices[num_choices] = Rectangle(0, 0, 0, 0);
	clear_text_pending();
	gwin->set_painted();
}

void Conversation::show_avatar_choices()
{
	char	**result;

	result=new char *[answers.size()];
	for(size_t i=0;i<answers.size();i++)
		{
		result[i]=new char[answers[i].size()+1];
		strcpy(result[i],answers[i].c_str());
		}
	show_avatar_choices(answers.size(),result);
	for(size_t i=0;i<answers.size();i++)
		{
		delete [] result[i];
		}
	delete [] result;
	}

void Conversation::clear_avatar_choices()
{
	gwin->paint(avatar_face);
}


/*
 *	User clicked during a conversation.
 *
 *	Output:	Index (0-n) of choice, or -1 if not on a choice.
 */

int Conversation::conversation_choice(int x, int y)
{
	int i;
	for (i = 0; conv_choices[i].w != 0 &&
			!conv_choices[i].has_point(x, y); i++)
		;
	if (conv_choices[i].w != 0)	// Found one?
		return (i);
	else
		return (-1);
}

/*
 *  return nr. of conversation option 'str'. -1 if not found
 */

int Conversation::locate_answer(const char *str)
{
  int num;
  vector<string>::iterator it;
  num = 0;
  for(it=answers.begin(); it!=answers.end(); ++it) {
    if(*it==str)
      return num;
    num++;
  }

  return -1;
}

void Conversation::push_answers()
{
  answer_stack.push_front(answers);
  answers.clear();
}

void Conversation::pop_answers()
{
  answers=answer_stack.front();
  answer_stack.pop_front();
}
