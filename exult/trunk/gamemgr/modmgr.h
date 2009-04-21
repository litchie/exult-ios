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
#include "Configuration.h"

using std::string;
extern Configuration *config;

class BaseGameInfo
	{
protected:
	Exult_Game type;	// Game type
	string cfgname;		// What the game is called in Exult.cfg
	string path_prefix;	// System path prefix for the game/mod.
	string mod_title;	// Internal mod name, the mod's title
	string menustring;	// Text displayed in mods menu
	bool expansion; 	// For FoV/SS ONLY.
	bool found;			// If the game/mod is found.
	bool editing;		// Game is being edited and may have missing files.
	string codepage;	// Game/mod codepage (mainly for ES).
public:
	BaseGameInfo () : type(NONE), cfgname(""), path_prefix(""), mod_title(""),
		menustring(""), expansion(false), found(false), editing(false),
		codepage("CP437")
		{  }
	BaseGameInfo (const Exult_Game ty, const char *cf, const char *mt,
		const char *pt, const char *ms, bool exp, bool f, bool ed,
		const char *cp)
		: type(ty), cfgname(cf), path_prefix(pt), mod_title(mt), menustring(ms),
		  expansion(exp), found(f), editing(ed), codepage(cp)
		{  }
	BaseGameInfo (const BaseGameInfo& other)
		: type(other.type), cfgname(other.cfgname),
		  path_prefix(other.path_prefix), mod_title(other.mod_title),
		  menustring(other.menustring), expansion(other.expansion),
		  found(other.found), editing(other.editing), codepage(other.codepage)
		{  }
	~BaseGameInfo () {  }

	const string get_cfgname () const { return cfgname; }
	const string get_path_prefix () const { return path_prefix; }
	const string get_mod_title () const { return mod_title; }
	const string get_menu_string () const { return menustring; }
	Exult_Game get_game_type () const { return type; }
	const string get_codepage () const { return codepage; }
	// For FoV/SS ONLY.
	bool have_expansion () const { return expansion; }
	bool is_there () const { return found; }
	bool being_edited () const { return editing; }
	void set_cfgname (const string& name) { cfgname = name; }
	void set_path_prefix (const string& pt) { path_prefix = pt; }
	void set_mod_title (const string& mod) { mod_title = mod; }
	void set_menu_string (const string& menu) { menustring = menu; }
	void set_game_type (Exult_Game game) { type = game; }
	// For FoV/SS ONLY.
	void set_expansion (bool tf) { expansion = tf; }
	void set_found (bool tf) { found = tf; }
	void set_editing (bool tf) { editing = tf; }
	void set_codepage (const string& cp) { codepage = cp; }
	
	void setup_game_paths ();

	virtual bool get_config_file(Configuration *&cfg, string& root)=0;
	};

class ModInfo : public BaseGameInfo
	{
protected:
	bool compatible;
	string configfile;
public:
	ModInfo (Exult_Game game, const string& name, const string& mod,
			const string& path, bool exp, bool ed, const string& cfg);
	ModInfo (const ModInfo& other)
		: BaseGameInfo(other.type, other.cfgname.c_str(),
		  other.mod_title.c_str(), other.path_prefix.c_str(),
		  other.menustring.c_str(), other.expansion, other.found,
		  other.editing, other.codepage.c_str()),
		  compatible(other.compatible), configfile(other.configfile)
		{  }
	~ModInfo () {}

	bool is_mod_compatible () const { return compatible; }

	virtual bool get_config_file(Configuration *&cfg, string& root)
		{
		cfg = new Configuration(configfile, "modinfo");
		root = "mod_info/";
		return true;
		}
	};

class ModManager : public BaseGameInfo
	{
protected:
	std::vector<ModInfo> modlist;
public:
	ModManager (const string& name, const string& menu, bool needtitle);
	ModManager () {  }
	ModManager (const ModManager& other)
		: BaseGameInfo(other.type, other.cfgname.c_str(),
		  other.mod_title.c_str(), other.path_prefix.c_str(),
		  other.menustring.c_str(), other.expansion, other.found,
		  other.editing, other.codepage.c_str())
		{
		for (std::vector<ModInfo>::const_iterator it = other.modlist.begin();
				it != other.modlist.end(); ++it)
			modlist.push_back(*it);
		}
	~ModManager ()
		{ modlist.clear(); }

	std::vector<ModInfo>& get_mod_list () { return modlist; }
	ModInfo *find_mod (const string& name);
	int find_mod_index (const string& name);

	bool has_mods () const { return modlist.size() > 0; }
	ModInfo *get_mod (int i)
		{
		if (i >= 0 && (unsigned)i < modlist.size())
			return &(modlist[i]);
		return 0;
		}
	BaseGameInfo *get_mod(const string& name, bool checkversion=true);
	void add_mod (const string& mod, const string& modconfig);

	void get_game_paths();
	void gather_mods();

	virtual bool get_config_file(Configuration *&cfg, string& root)
		{
		cfg = config; root = "config/disk/game/" + cfgname + "/";
		return false;
		}
	};

class GameManager
	{
	UNREPLICATABLE_CLASS(GameManager);
protected:
	// -> to original games.
	ModManager *bg;
	ModManager *fov;
	ModManager *si;
	ModManager *ss;
	std::vector<ModManager> games;
	void print_found(ModManager *game, const char *flex,
			const char *title, const char *cfgname, const char *basepath);
public:
	GameManager ();
	~GameManager ()
		{ games.clear(); }

	std::vector<ModManager>& get_game_list () { return games; }
	int get_game_count () const { return games.size(); }
	ModManager *get_game (int i)
		{
		if (i >= 0 && (unsigned)i < games.size())
			return &(games[i]);
		return 0;
		}
	bool is_bg_installed () const { return bg != 0; };
	bool is_fov_installed () const { return fov != 0; };
	bool is_si_installed () const { return si != 0; };
	bool is_ss_installed () const { return ss != 0; };
	ModManager *find_game (const string& name);
	ModManager *get_bg ()
		{ return bg ? bg : fov ? fov : 0; }
	ModManager *get_fov ()
		{ return fov ? fov : 0; }
	ModManager *get_si ()
		{ return si ? si : ss ? ss : 0; }
	ModManager *get_ss ()
		{ return ss ? ss : 0; }
	int find_game_index (const string& name);
	void add_game (const string& name, const string& menu);
	};

#endif
