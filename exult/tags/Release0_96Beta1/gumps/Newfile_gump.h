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

#ifndef NEWFILE_GUMP_H
#define NEWFILE_GUMP_H

#include "Modal_gump.h"

class Shape_file;
class Image_buffer;

#define MAX_SAVEGAME_NAME_LEN	0x50

struct SaveGame_Details
{
	// Time that the game was saved (needed????)
	char	real_minute;	// 1
	char	real_hour;	// 2
	char	real_day;	// 3
	char	real_month;	// 4
	short	real_year;	// 6

	// The Game Time that the save was done at
	char	game_minute;	// 7
	char	game_hour;	// 8
	short	game_day;	// 10

	short	save_count;	// 12
	char	party_size;	// 13

	char	unused;		// 14 Quite literally unused

	char	real_second;	// 15

	//Incase we want to add more later
	char		reserved0;	// 16
	char	reserved1[48];	// 64
};

struct SaveGame_Party
{
	char		name[18];	// 18
	short		shape;		// 20
	unsigned int	exp;		// 24
	unsigned int	flags;		// 28
	unsigned int	flags2;		// 32

	unsigned char	food;		// 33
	unsigned char	str;		// 34
	unsigned char	combat;		// 35
	unsigned char	dext;		// 36
	unsigned char	intel;		// 37
	unsigned char	magic;		// 38
	unsigned char	mana;		// 39
	unsigned char	training;	// 40
	short		health;		// 42

	short		shape_file;	// 44

	//Incase we want to add more later
	int		reserved1;	// 48
	int		reserved2;	// 52
	int		reserved3;	// 56
	int		reserved4;	// 60
	int		reserved5;	// 64
};

/*
 *	The file save/load box:
 */
class Newfile_gump : public Modal_gump
{
	UNREPLICATABLE_CLASS_I(Newfile_gump,Modal_gump(0,0,0,0));

public:
	struct SaveInfo {

		int			num;
		char			*filename;
		char			*savename;
		bool			readable;
		SaveGame_Details	*details;
		SaveGame_Party		*party;
		Shape_file		*screenshot;

		static int		CompareGames(const void *a, const void *b);
		int			CompareThis(const SaveInfo *other) const;
		void			SetSeqNumber();

		SaveInfo();
		~SaveInfo();

	};

protected:
	Gump_button *buttons[8];	// 2 sets of 4 buttons
	static const short btn_cols[5];	// x-coord of each button.
	static const short btn_rows[5];	// y-coord of each button.

	// Text field info
	static const short fieldx;	// Start Y of each field
	static const short fieldy;	// Start X of first
	static const short fieldw;	// Width of each field
	static const short fieldh;	// Height of each field
	static const short fieldgap;	// Gap between fields
	static const short fieldcount;	// Number of fields
	static const short textx;	// X Offset in field
	static const short texty;	// Y Offset in field
	static const short textw;	// Maximum allowable width of text
	static const short iconx;	// X Offset in field
	static const short icony;	// Y Offset in field

	// Scrollbar and Slider Info
	static const short scrollx;	// X Offset
	static const short scrolly;	// Y Offset
	static const short scrollh;	// Height of Scroll Bar
	static const short sliderw;	// Width of Slider
	static const short sliderh;	// Height of Slider

	// Side Text
	static const short infox;	// X Offset for info
	static const short infoy;	// Y Offset for info
	static const short infow;	// Width of info box
	static const short infoh;	// Height of info box
	static const char infostring[];	// Text format for info

	static const char *months[12];	// Names of the months

	unsigned char restored;		// Set to 1 if we restored a game.

	Image_buffer	*back;

	SaveInfo	*games;		// The list of savegames
	int		num_games;	// Number of save games
	int		first_free;	// The number of the first free savegame

	Shape_file *cur_shot;		// Screenshot for current game
	SaveGame_Details *cur_details;	// Details of current game
	SaveGame_Party *cur_party;	// Party of current game

	// Gamedat is being used as a 'quicksave'
	Shape_file *gd_shot;		// Screenshot in Gamedat
	SaveGame_Details *gd_details;	// Details in Gamedat
	SaveGame_Party *gd_party;	// Parts in Gamedat

	Shape_file *screenshot;		// The picture to be drawn
	SaveGame_Details *details;	// The game details to show
	SaveGame_Party *party;		// The party to show
	bool is_readable;		// Is the save game readable
	const char *filename;		// Filename of the savegame, if exists

	int	list_position;		// The position in the savegame list (top game)
	int	selected;		// The savegame that has been selected (num in list)
	int	cursor;			// The position of the cursor
	int	slide_start;		// Pixel (v) where a slide started
	char	newname[MAX_SAVEGAME_NAME_LEN];	// The new name for the game
	 
	int	BackspacePressed();
	int	DeletePressed();
	int	MoveCursor(int count);
	int	AddCharacter(char c);

	void	LoadSaveGameDetails();	// Loads (and sorts) all the savegame details
	void	FreeSaveGameDetails();	// Frees all the savegame details

	void	PaintSaveName (int line);

public:
	Newfile_gump();
	~Newfile_gump();

	void load();			// 'Load' was clicked.
	void save();			// 'Save' was clicked.
	void delete_file();		// 'Delete' was clicked.

	void scroll_line(int dir);	// Scroll Line Button Pressed
	void scroll_page(int dir);	// Scroll Page Button Pressed.
	
	int restored_game()		// 1 if user restored.
		{ return restored; }
					// Paint it and its contents.
	virtual void paint(Game_window *gwin);
	virtual void close(Game_window *gwin)
		{ done = 1; }
					// Handle events:
	virtual void mouse_down(int mx, int my);
	virtual void mouse_up(int mx, int my);
	virtual void mouse_drag(int mx, int my);
	virtual void key_down(int chr);	// Character typed.

	virtual void mousewheel_up();
	virtual void mousewheel_down();
};

#endif //NEWFILE_GUMP_H
