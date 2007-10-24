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
#include <string>

class Actor;

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

enum AniType				// Type of animation
{
	FA_LOOPING = 0,
	FA_HOURLY = 1,
	FA_NON_LOOPING = 2,
	FA_RANDOM_LOOP = 3,
	FA_LOOP_RECYCLE = 4,
	FA_RANDOM_FRAMES = 5
};
struct Animation_info
{
	AniType	type;
	int		first_frame;	// First frame of the animation cycle
	int		frame_count;	// Frame count of the animation cycle
	int		offset;			// Overall phase shift of the animation
	int		offset_type;	// If 1, set offset = init_frame % frame_count
	                        // If -1, set offset = init_frame
	int		frame_delay;	// Delay multiplier between frames.
	int		sfx_info;		// Extra sfx info; exact use dependens on type.
};

struct SFX_info
{
	int		num;
	bool	rand;			// sfx in range are to be randomly chosen.
	int		range;			// # of sequential sfx to be used.
	int		chance;			// % chance of playing the SFX.
	int		extra;			// Play this SFX at noon and midnight.
};

struct Explosion_info
{
	int		sprite;			// Explosion sprite.
	int		sfxnum;			// SFX to play or 255 for none.
};

struct Base_Avatar_info
{
	int shape_num;
	int face_shape;			// Shape and frame for face during the
	int face_frame;			// normal game.
};

struct Avatar_default_skin
{
	int default_skin;		// The starting skin color.
	bool default_female;	// True if the default sex if female.
};

struct Skin_data
{
	int skin_id;
	bool is_female;
	int shape_num;
	int naked_shape;
	int face_shape;			// Shape and frame for face during the
	int face_frame;			// normal game.
	int alter_face_shape;	// Shape and frame for face to be used
	int alter_face_frame;	// when flag 33 is set.
	bool copy_info;			// Whether or not Exult should overwrite shape info
				// with info from the default avatar shape
};

struct Usecode_function_data
{
	int fun_id;
	int event_id;
};

class Shapeinfo_entry_parser;

/*
 *	A class to get the extra information for a given shape.
 */
class Shapeinfo_lookup
	{
	typedef std::vector<char *> Readstrings;
	static Skin_data *ScrollSkins(int skin, bool sex, bool sishapes, bool ignoresex, bool prev, bool sel);
public:
	static void reset();

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
	static int get_explosion_sfx (int shapenum);
	static SFX_info *get_sfx_info (int shapenum);
	static Animation_info *get_animation_cycle_info (int shapenum, int init_frame);
	static bool get_usecode_events (int shapenum);
	static bool get_mountain_top (int shapenum);

	static std::vector<std::pair<std::string, int> > *GetFacesSources();
	static std::vector<std::pair<std::string, int> > *GetPaperdollSources();
	static std::vector<std::pair<int, int> > *GetImportedSkins();
	static std::vector<std::pair<int, int> > *GetImportedGumpShapes();
	static int GetBlueShapeData(int spot);
	static bool IsSkinImported(int shape);

	static Base_Avatar_info *GetBaseAvInfo(bool sex);
	static int GetMaleAvShape();
	static int GetFemaleAvShape();
	static int GetNextSkin(int skin, bool sex, bool sishapes, bool ignoresex = false)
		{
		return (ScrollSkins(skin, sex, sishapes, ignoresex, false, false))->skin_id;
		}
	static int GetPrevSkin(int skin, bool sex, bool sishapes, bool ignoresex = false)
		{
		return (ScrollSkins(skin, sex, sishapes, ignoresex, true, false))->skin_id;
		}
	static Skin_data *GetNextSelSkin(Skin_data *sk, bool sishapes, bool ignoresex = false)
		{
		return ScrollSkins(sk->skin_id, sk->is_female, sishapes, ignoresex, false, true);
		}
	static Skin_data *GetPrevSelSkin(Skin_data *sk, bool sishapes, bool ignoresex = false)
		{
		return ScrollSkins(sk->skin_id, sk->is_female, sishapes, ignoresex, true, true);
		}
	static int GetNumSkins(bool sex);
	static Avatar_default_skin *GetDefaultAvSkin();
	static std::vector<Skin_data> *GetSkinList();
	static Skin_data *GetSkinInfo(int skin, bool sex);
	static Skin_data *GetSkinInfoSafe(int skin, bool sex, bool sishapes);
	static Skin_data *GetSkinInfoSafe(Actor *npc);
	static bool IsSkinSelectable(int skin);
	static bool HasFaceReplacement(int npcid);
	static int GetFaceReplacement(int facenum);
	static Usecode_function_data *GetAvUsecode(int type);
private:
	static void Read_data_file(const char *fname, const char *sections[],
			Shapeinfo_entry_parser *parsers[],
			int numsections);
	static void setup_miscinf();
	static void setup_bodies();
	static void setup_paperdolls();
	static void setup_shape_files();
	static void setup_avatar_data();
	};

#endif
