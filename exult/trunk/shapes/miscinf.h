/*
 *  miscinf.h - Information about several previously-hardcoded shape data.
 *
 *  Copyright (C) 2006  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef MISCINF_H
#define MISCINF_H 1
#include <vector>
#include <map>

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
	int			world_frame;		// Frame in the world (-1 for all)
	int			spot;				// Spot placed in

	Object_type	type;				// What type of object is this
	
	bool		translucent;		// If the paperdoll should be drawn translucently or not
	bool		gender;				// Is this object gender specific

	int			shape;				// The shape (if -1 use world shape and frame)
	int			frame;				// The frame
	int			frame2;				// Second Frame (if used)
	int			frame3;				// Third Frame (if used)
	int			frame4;				// Fourth Frame (if used)
};

// This contain Information about NPC rendering
struct  Paperdoll_npc
{
	bool		is_female;			// Is the NPC Female (or more specifically not male)
	bool		translucent;		// If the paperdoll should be drawn translucently or not

	// Body info
	int			body_shape;			// Body Shape
	int			body_frame;			// Body Frame

	int			head_shape;			// Head Shape
	int			head_frame;			// Normal Head Frame
	int			head_frame_helm;		// Frame when wearing a helm

	int			arms_shape;			// Shape for Arms
	int			arms_frame;			// Normal Arms Frame
	int			arms_frame_2h;		// Frame when holding a two handed weapon
	int			arms_frame_staff;	// Frame when holding staff style weapon
	
	// BG gump info
	int			gump_shape;
};

struct Animation_info
{
	int		type;			// Type of animation; one of FA_LOOPING,
							// FA_SUNDIAL or FA_ENERGY_FIELD
	int		first_frame;	// First frame of the animation cycle
	int		frame_count;	// Frame count of the animation cycle
	int		offset;			// Overall phase shift of the animation
	int		offset_type;	// If 1, set offset = init_frame % frame_count
							// If -1, set offset = init_frame
};

/*
 *	Base parser class shape data.
 */
class Shapeinfo_entry_parser
	{
public:
	virtual void parse_entry(int index, char *eptr, bool for_patch) = 0;
	virtual int ReadInt(char *&eptr, int off = 1)
		{
		int ret = strtol(eptr + off, &eptr, 0);
		while (isspace(*eptr++))
			;
		return ret;
		}
	};

/*
 *	Parser class for integer shape data.
 */
class Int_pair_parser: public Shapeinfo_entry_parser
	{
	std::map<int, int> *table;
public:
	Int_pair_parser(std::map<int, int> *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr, bool for_patch)
		{
		int key = ReadInt(eptr, 0);
		int data = ReadInt(eptr);
		(*table)[key] = data;
		}
	};

/*
 *	Parser class for boolean shape data.
 */
class Bool_parser: public Shapeinfo_entry_parser
	{
	std::map<int, bool> *table;
public:
	Bool_parser(std::map<int, bool> *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr, bool for_patch)
		{
		int key = ReadInt(eptr, 0);
		(*table)[key] = true;
		}
	};

/*
 *	Parser class for body shape data.
 */
class Body_parser: public Shapeinfo_entry_parser
	{
	std::map<int, std::pair<int, int> > *table;
public:
	Body_parser(std::map<int, std::pair<int, int> > *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr, bool for_patch)
		{
		int bshape = ReadInt(eptr, 0);
		int bframe = ReadInt(eptr);
		((*table)[index]).first = bshape;
		((*table)[index]).second = bframe;
		}
	};

/*
 *	Parser class for information about animation cycles.
 */
class Animation_parser: public Shapeinfo_entry_parser
	{
	std::multimap<int, Animation_info> *table;
public:
	Animation_parser(std::multimap<int, Animation_info> *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr, bool for_patch);
	};

/*
 *	Parser class for NPC Paperdoll information.
 */
class Paperdoll_npc_parser: public Shapeinfo_entry_parser
	{
	std::map<int, Paperdoll_npc> *table;
public:
	Paperdoll_npc_parser(std::map<int, Paperdoll_npc> *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr, bool for_patch);
	};

/*
 *	Parser class for non-NPC Paperdoll information.
 */
class Paperdoll_item_parser: public Shapeinfo_entry_parser
	{
	std::multimap<int, Paperdoll_item> *table;
public:
	Paperdoll_item_parser(std::multimap<int, Paperdoll_item> *tbl)
		: table(tbl)
		{  }
	virtual void parse_entry(int index, char *eptr, bool for_patch);
	};

/*
 *	A class to get the extra information for a given shape.
 */
class Shapeinfo_lookup
	{
	typedef std::vector<char *> Readstrings;
public:
	// Body info:
	static int find_body(int liveshape, int& deadshape, int& deadframe);
	static bool Is_body_shape(int shapeid);
	
	// Paperdoll info:
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

	// Misc info:
	static int get_explosion_sprite (int shapenum);
	static int get_shape_sfx (int shapenum);
	static Animation_info *get_animation_cycle_info (int shapenum, int init_frame);
	static bool get_usecode_events (int shapenum);
	static bool get_mountain_top (int shapenum);
private:
	static void Read_data_file(const char *fname, const char *sections[],
			Shapeinfo_entry_parser *parsers[],
			int numsections);
	static void setup_miscinf();
	static void setup_bodies();
	static void setup_paperdolls();
	};
#endif
