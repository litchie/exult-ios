/*
Copyright (C) 2000 The Exult Team

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

#ifndef _PAPERDOLL_GUMP_H_
#define _PAPERDOLL_GUMP_H_

#include "Gump.h"

class Heart_button;
class Disk_button;
class Combat_button;
class Halo_button;
class Cstats_button;
class Combat_mode_button;

//
// For best viewing use Tab size = 4
//

class Paperdoll_gump : public Gump 
{
public:
	enum Object_type
	{
		OT_Normal = 0,
		OT_Single = 1,
		OT_Double = 2,
		OT_Staff = 3,
		OT_Bow = 4,
		OT_Crossbow = 5,
		OT_Shield = 6,
		OT_Helm = 7,
		OT_Musket = 8
	};

	// This contains info on how to render an item when it's in a certain spot
	struct Paperdoll_item
	{
		int			world_shape;			// Shape in the world
		int			world_frame;			// Frame in the world (-1 for all)
		int			spot;				// Spot placed in

		Object_type		type;				// What type of object is this
		
		bool			gender;				// Is this object gender specific

		ShapeFile		file;				// Which  VGA file is the shape in
		int			shape;				// The shape (if -1 use world shape and frame)
		int			frame;				// The frame
		int			frame2;				// Second Frame (if used)
		int			frame3;				// Second Frame (if used)
		int			frame4;				// Second Frame (if used)
	};

	// This contain Information about NPC rendering
	struct  Paperdoll_npc
	{
		int			npc_shape;			// Choose the NPC based on shape, not NPC number
		bool			is_female;			// Is the NPC Female (or more specifically not male)
		ShapeFile		file;				// Which VGA file the head shape is in

		// Body info
		int			body_shape;			// Body Shape
		int			body_frame;			// Body Frame

		int			head_shape;			// Head Shape
		int			head_frame;			// Normal Head Frame
		int			head_frame_helm;		// Frame when wearing a helm

		int			arms_shape;			// Shape for Arms
		int			arms_frame;			// Normal Arms Frame
		int			arms_frame_2h;			// Frame when holding a two handed weapon
		int			arms_frame_staff;		// Frame when holding staff style weapon
	};

private:
	UNREPLICATABLE_CLASS(Paperdoll_gump);

protected:

	// Statics

	// Serpent Isle
	static Paperdoll_npc Characters[];	// NPC information
	static Paperdoll_item Items[];		// Item Information

	// Black Gate (it's a hack I tell you)
	static Paperdoll_npc Characters_BG[];	// NPC information
	static Paperdoll_item Items_BG[];	// Item Information

	static short coords[36];		// Coords. of where to draw things,
	static short coords_blue[36];		// indexed by spot # (0-17).
	static short shapes_blue[36];
	static short coords_hot[36];		// Hot spots
		
	static short diskx, disky;		// Where to show 'diskette' button.
	static short heartx, hearty;		// Where to show 'stats' button.
	static short combatx, combaty;		// Combat button.
	static short cstatx, cstaty;		// Combat mode.
	static short halox, haloy;	// "Protected" halo.
	static short cmodex, cmodey;	// Combat mode.

	static short bodyx, bodyy;		// Body
	static short headx, heady;		// Head
	static short beltfx, beltfy;		// Female Belt
	static short neckfx, neckfy;		// Female Neck
	static short beltmx, beltmy;		// Male Belt
	static short neckmx, neckmy;		// Male Neck
	static short legsx, legsy;		// Legs
	static short feetx, feety;		// Feet
	static short handsx, handsy;		// Hands
	static short lhandx, lhandy;		// Left Hand
	static short rhandx, rhandy;		// Right Hand
	static short ahandx, ahandy;		// Ammo in Left Hand
	static short ammox, ammoy;		// Quiver

	static short backfx, backfy;		// Female Back
	static short backmx, backmy;		// Male Back
	static short back2fx, back2fy;		// Female Back Weapon
	static short back2mx, back2my;		// Male Back Weapon
	static short shieldfx, shieldfy;	// Female Back Shield
	static short shieldmx, shieldmy;	// Male Back Shield


	// Non Statics

	Heart_button *heart_button;		// For bringing up stats.
	Disk_button *disk_button;		// For bringing up 'save' box. (Avatar Only)
	Combat_button *combat_button;		// Combat Toggle (Avatar Only)
	Cstats_button *cstats_button;		// Combat Stats (Not BG)
	Halo_button *halo_button;		// Halo (protection) (BG Only)
	Combat_mode_button *cmode_button;	// Combat Modes (BG Only)


	// Statics

	// Get the X and Y from a spot
	static int spotx(int i) { return coords[2*i]; }
	static int spoty(int i) { return coords[2*i + 1]; }

	// Non Statics

	// Find index of closest spot to the mouse pointer
	int find_closest(int mx, int my, int only_empty = 0);
	
	// Set to location of an object a spot
	void set_to_spot(Game_object *obj, int index);

public:

	// Statics

	inline static bool IsObjectAllowed(int shape, int frame, int spot)
		{ return GetItemInfo(shape, frame, spot)!=NULL?true:false; }

	inline static int GetFaceShape(int shape, int frame, int spot)
		{ return GetItemInfo(shape, frame, spot)!=NULL?true:false; }

	inline static bool IsNPCFemale(int shape)
	{ return GetCharacterInfo(shape)?GetCharacterInfoSafe(shape)->is_female:true; }

	// Retrieve info about an item or NPC
	static Paperdoll_npc *GetCharacterInfoSafe(int shape);
	static Paperdoll_npc *GetCharacterInfo(int shape);
	static Paperdoll_item *GetItemInfo(int shape, int frame = -1, int spot = -1);

	// Non Statics

	Paperdoll_gump(Container_game_object *cont, int initx, int inity, 
								int shnum);

	~Paperdoll_gump();

	// Is a given point on a button?
	virtual Gump_button *on_button(Game_window *gwin, int mx, int my);

	// Find the object the mouse is over
	virtual Game_object *find_object(int mx, int my);

	// Add object.
	virtual int add(Game_object *obj, int mx = -1, int my = -1, 
			int sx = -1, int sy = -1, bool dont_check = false,
						bool combine = false);

	// Paint it and its contents.
	virtual void paint();


	//
	// Painting Helpers
	//

	// Generic Paint Object Method
	void paint_object(const Rectangle &box, Paperdoll_npc *info, int spot,
						int sx, int sy, int frame = 0, int itemtype = -1, int checkspot = -1, int checktype = -1);

	// Generic Paint Object Method for something that is armed dependant
	void paint_object_arms(const Rectangle &box, Paperdoll_npc *info, int spot,
						int sx, int sy, int start = 0, int itemtype = -1);

	// Special 'Constant' Paint Methods
	void paint_body (const Rectangle &box, Paperdoll_npc *info);
	void paint_belt (const Rectangle &box, Paperdoll_npc *info);
	void paint_head (const Rectangle &box, Paperdoll_npc *info);
	void paint_arms (const Rectangle &box, Paperdoll_npc *info);

	// What are we holding?
	Object_type get_arm_type (void);


	//
	// Finding Helpers
	//

	// Generic Check Object Method
	Game_object *check_object (int mx, int my, Paperdoll_npc *info, int spot,
						int sx, int sy, int frame = 0, int itemtype = -1, int checkspot = -1, int checktype = -1);


	// Generic Check Object Method for something that is armed dependant
	Game_object *check_object_arms (int mx, int my, Paperdoll_npc *info, int spot,
						int sx, int sy, int start = 0, int itemtype = -1);

	// Special 'Constant' Check Methods
	bool check_body (int mx, int my, Paperdoll_npc *info);
	bool check_belt (int mx, int my, Paperdoll_npc *info);
	bool check_head (int mx, int my, Paperdoll_npc *info);
	bool check_arms (int mx, int my, Paperdoll_npc *info);

	// Generic Method to check a shape
	bool check_shape (int px, int py, int shape, int frame, ShapeFile file);

	virtual Container_game_object *find_actor(int mx, int my);
};

#endif
