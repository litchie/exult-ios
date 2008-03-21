/*
 *	modmgr.h - Mod manager for Exult.
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

#ifndef MODMGR_H
#define MODMGR_H

#include <vector>
#include <string>
#include "exult_constants.h"
#include "exceptions.h"

using std::string;
class Configuration;

class BaseGameInfo {
protected:
	Exult_Game type;	// Game type
	string title;		// The *Game* title
	string mod_title;	// Internal mod name, the mod's title
	string menustring;	// Text displayed in mods menu
public:
	BaseGameInfo () : title(""), mod_title(""), menustring("") {}
	~BaseGameInfo () {}

	string get_title () const { return title; }
	string get_mod_title () const { return mod_title; }
	string get_menu_string () const { return menustring; }
	Exult_Game get_game_type () const { return type; }
	void set_title (string name) { title = name; }
	void set_mod_title (string mod) { mod_title = mod; }
	void set_menu_string (string menu) { menustring = menu; }
	void set_game_type (Exult_Game game) { type = game; }
	
	void setup_game_paths ();
};

class ModInfo : public BaseGameInfo {
protected:
	bool compatible;
public:
	ModInfo (Exult_Game game, string name, string mod, Configuration *modconfig);
	~ModInfo () {}

	bool is_mod_compatible () const { return compatible; }
};

class ModManager : public BaseGameInfo {
protected:
	std::vector<ModInfo *> modlist;
public:
	ModManager (Exult_Game game, string name, string menu);
	~ModManager ();

	std::vector<ModInfo *>& get_mod_list () { return modlist; }
	ModInfo *find_mod (string name);
	int find_mod_index (string name);

	bool has_mods () const { return modlist.size() > 0; }
	ModInfo *get_mod (int i) const
		{
		if (i >= 0 && i < modlist.size())
			return modlist[i];
		return 0;
		}
	BaseGameInfo *get_mod(string name, bool checkversion=true);
	void add_mod (string mod, Configuration *modconfig);
};

class GameManager {
	UNREPLICATABLE_CLASS(GameManager);
protected:
	std::vector<ModManager *> games;
	bool bg_installed;
	bool si_installed;
	void set_bg_installed ();
	void set_si_installed ();
public:
	GameManager ();
	~GameManager ();

	std::vector<ModManager *>& get_game_list () { return games; }
	int get_game_count () const { return games.size(); }
	ModManager *get_game (int i) const
		{
		if (i >= 0 && i < games.size())
			return games[i];
		return 0;
		}
	bool is_bg_installed () const { return bg_installed; };
	bool is_si_installed () const { return si_installed; };
	ModManager *find_game (string name);
	int find_game_index (string name);
	void add_game (string name, string menu);
};

#endif
