/*
 *  Copyright (C) 2000-2001  The Exult Team
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

#ifndef CHEAT_SCREEN_H
#define CHEAT_SCREEN_H

class Game_window;
class Image_buffer8;
class Font;
class Game_clock;
class Actor;

class CheatScreen {
	Actor			*grabbed;
	static const char	*schedules[33];
	static const char	*flag_names[64];
public:
		CheatScreen();
		~CheatScreen();
	void	show_screen();
	void	SetGrabbedActor (Actor *g) { grabbed = g; }
	void	ClearThisGrabbedActor (Actor *g) const { if (g==grabbed) g = 0; }
private:
	enum Cheat_Prompt
	{
		CP_Command = 0,

		CP_HitKey = 1,
		CP_NotAvail = 2,
		CP_InvalidNPC = 3,
		CP_InvalidCom = 4,
		CP_Canceled = 5,
		CP_ClockSet = 6,
		CP_InvalidTime = 7,
		CP_InvalidShape = 8,
		CP_InvalidValue = 9,
		CP_Created = 10,
		CP_ShapeSet = 11,
		CP_ValueSet = 12,
		CP_NameSet = 13,
		CP_WrongShapeFile = 14,

		CP_ChooseNPC = 16,
		CP_EnterValue = 17,
		CP_Minute = 18,
		CP_Hour = 19,
		CP_Day = 20,
		CP_Shape = 21,
		CP_Activity = 22,
		CP_XCoord = 23,
		CP_YCoord = 24,
		CP_Lift = 25,
		CP_GFlagNum = 26,
		CP_NFlagNum = 27,

		CP_Name = 32
	};
	Game_window* gwin;
	Image_buffer8* ibuf;
	Font* font;
	Game_clock *clock;
	int maxx, maxy;
	int centerx, centery;
	Palette pal;

	void SharedPrompt (char *input, const Cheat_Prompt &mode);
	bool SharedInput (char *input, int len, int &command, Cheat_Prompt &mode, bool &activate);

	void NormalLoop();
	void NormalDisplay ();
	void NormalMenu();
	void NormalActivate (char *input, int &command, Cheat_Prompt &mode);
	bool NormalCheck (char *input, int &command, Cheat_Prompt &mode, bool &activate);

	void ActivityDisplay ();

	Cheat_Prompt GlobalFlagLoop(int num);

	Cheat_Prompt TimeSetLoop();

	Cheat_Prompt NPCLoop(int num);
	void NPCDisplay (Actor *actor, int &num);
	void NPCMenu(Actor *actor, int &num);
	void NPCActivate (char *input, int &command, Cheat_Prompt &mode, Actor *actor, int &num);
	bool NPCCheck (char *input, int &command, Cheat_Prompt &mode, bool &activate, Actor *actor, int &num);

	void FlagLoop(Actor *actor);
	void FlagMenu(Actor *actor);
	void FlagActivate (char *input, int &command, Cheat_Prompt &mode, Actor *actor);
	bool FlagCheck (char *input, int &command, Cheat_Prompt &mode, bool &activate, Actor *actor);
	Cheat_Prompt AdvancedFlagLoop(int flagnum, Actor *actor);

	void BusinessLoop(Actor *actor);
	void BusinessDisplay (Actor *actor);
	void BusinessMenu(Actor *actor);
	void BusinessActivate (char *input, int &command, Cheat_Prompt &mode, Actor *actor, int &time, int &prev);
	bool BusinessCheck (char *input, int &command, Cheat_Prompt &mode, bool &activate, Actor *actor, int &time);

	void StatLoop(Actor *actor);
	void StatMenu(Actor *actor);
	void StatActivate (char *input, int &command, Cheat_Prompt &mode, Actor *actor);
	bool StatCheck (char *input, int &command, Cheat_Prompt &mode, bool &activate, Actor *actor);

};

#endif
