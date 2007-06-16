/*
 *	modmgr.cc - Mod manager for Exult.
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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <vector>
#include <iostream>
#include <string>
#include "modmgr.h"
#include "fnames.h"
#include "listfiles.h"
#include "exult_constants.h"
#include "utils.h"
#include "Configuration.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

// BaseGameInfo: Generic information and functions common to mods and games
void BaseGameInfo::setup_game_paths ()
{
	// Make aliases to the current game's paths.
	string system_path_tag(to_uppercase(title.c_str()));
	clone_system_path("<STATIC>", "<" + system_path_tag + "_STATIC>");
	clone_system_path("<MODS>", "<" + system_path_tag + "_MODS>");

	if (mod_title != "")
		system_path_tag = system_path_tag + "_" + mod_title;
	to_uppercase(system_path_tag);
	clone_system_path("<GAMEDAT>", "<" + system_path_tag + "_GAMEDAT>");
	clone_system_path("<SAVEGAME>", "<" + system_path_tag + "_SAVEGAME>");
	if (is_system_path_defined("<" + system_path_tag + "_PATCH>"))
		clone_system_path("<PATCH>", "<" + system_path_tag + "_PATCH>");
	else
		clear_system_path("<PATCH>");

	U7mkdir("<SAVEGAME>", 0755);	// make sure savegame directory exists
	U7mkdir("<GAMEDAT>", 0755);		// make sure gamedat directory exists
}

// ModInfo: class that manages one mod's information
ModInfo::ModInfo (Exult_Game game, string name, string mod, Configuration *modconfig)
{
	type = game;
	title = name;
	mod_title = mod;

	string config_path, default_dir, modversion, savedir, patchdir, gamedatdir;
	
	config_path = "mod_info/mod_title";
	default_dir = mod;
	string modname;
	modconfig->value(config_path, modname, default_dir.c_str());
	mod_title = modname;

	config_path = "mod_info/display_string";
	default_dir = "Description missing!";
	string menustr;
	modconfig->value(config_path, menustr, default_dir.c_str());
	menustring = menustr;
	
	config_path = "mod_info/required_version";
	default_dir = "0.0.00R";
	modconfig->value(config_path, modversion, default_dir.c_str());
	if (modversion == default_dir)
	{
		// Required version is missing; assume the mod to be incompatible
		compatible = false;
	}
	else
	{
		const char *ptrmod = modversion.c_str(), *ptrver = VERSION;
		char *eptrmod, *eptrver = VERSION;
		int modver = strtol(ptrmod, &eptrmod, 0), exver = strtol(ptrver, &eptrver, 0);

		// Assume compatibility:
		compatible = true;
		// Comparing major version number:
		if (modver > exver)
			compatible = false;
		else if (modver == exver)
		{
			modver = strtol(eptrmod + 1, &eptrmod, 0);
			exver = strtol(eptrver + 1, &eptrver, 0);
			// Comparing minor version number:
			if (modver > exver)
				compatible = false;
			else if (modver == exver)
			{
				modver = strtol(eptrmod + 1, &eptrmod, 0);
				exver = strtol(eptrver + 1, &eptrver, 0);
				// Comparing revision number:
				if (modver > exver)
					compatible = false;
				else if (modver == exver)
				{
					string mver(to_uppercase(eptrmod)), ever(to_uppercase(eptrver));
					// Release vs CVS:
					if (mver == "CVS" && ever == "R")
						compatible = false;
				}
			}
		}
	}

	string tagstr(mod_title), systagstr(to_uppercase(title.c_str())),
		system_path_tag(to_uppercase(title + ("_" + tagstr))),
		mods_dir("<" + systagstr + "_MODS>"), data_directory(mods_dir + "/" + tagstr),
		mods_macro("__MODS__"), mod_path_macro("__MOD_PATH__");
	string::size_type pos;

	config_path = "mod_info/gamedat_path";
	default_dir = data_directory + "/gamedat";
	modconfig->value(config_path, gamedatdir, get_system_path(default_dir).c_str());
	// Path 'macros' for relavite paths:
	pos = gamedatdir.find(mods_macro);
	if (pos != string::npos)
		gamedatdir.replace(pos, mods_macro.length(), mods_dir);
	pos = gamedatdir.find(mod_path_macro);
	if (pos != string::npos)
		gamedatdir.replace(pos, mod_path_macro.length(), data_directory);
	add_system_path("<" + system_path_tag + "_GAMEDAT>", get_system_path(gamedatdir));

	config_path = "mod_info/savegame_path";
	modconfig->value(config_path, savedir, get_system_path(data_directory).c_str());
	// Path 'macros' for relavite paths:
	pos = savedir.find(mods_macro);
	if (pos != string::npos)
		savedir.replace(pos, mods_macro.length(), mods_dir);
	pos = savedir.find(mod_path_macro);
	if (pos != string::npos)
		savedir.replace(pos, mod_path_macro.length(), data_directory);
	add_system_path("<" + system_path_tag + "_SAVEGAME>", get_system_path(savedir));
	
	config_path = "mod_info/patch";
	default_dir = data_directory + "/patch";
	modconfig->value(config_path, patchdir, get_system_path(default_dir).c_str());
	// Path 'macros' for relavite paths:
	pos = patchdir.find(mods_macro);
	if (pos != string::npos)
		patchdir.replace(pos, mods_macro.length(), mods_dir);
	pos = patchdir.find(mod_path_macro);
	if (pos != string::npos)
		patchdir.replace(pos, mod_path_macro.length(), data_directory);
	add_system_path("<" + system_path_tag + "_PATCH>", get_system_path(patchdir));
}

// ModManager: class that manages a game's modlist and paths
ModManager::ModManager (Exult_Game game, string name, string menu)
{
	title = name;
	mod_title = "";
	type = game;
	menustring = menu;

	modlist = new std::vector<ModInfo *>();

	FileList filenames;
	to_uppercase(name);
	string pathname("<" + name + "_MODS>");
	int ptroff = get_system_path(pathname).length()+1;
	
	// If the dir doesn't exist, leave at once.
	if (!U7exists(pathname))
		return;

	U7ListFiles(pathname + "/*.cfg", filenames);
	int num_mods = filenames.size();

	if (num_mods>0)
	{
		modlist->reserve(num_mods);
		for (int i=0; i<num_mods; i++)
		{
			string modtitle = filenames[i].substr(ptroff,filenames[i].size()-ptroff-4);
			Configuration *modcfg = new Configuration(filenames[i], "modinfo");
			ModInfo *mod = new ModInfo(type, title, modtitle, modcfg);
			modlist->push_back(mod);
		}
	}
}

ModManager::~ModManager ()
{
	for (int i = 0; i<modlist->size(); i++)
	{
		ModInfo *exultmod = (*modlist)[i];
		delete exultmod;
	}
	modlist->clear();
}

ModInfo *ModManager::find_mod (string name)
{
	for(int i=0; i<modlist->size(); i++)
	{
		ModInfo *it = (*modlist)[i];
		if (it->get_mod_title() == name)
			return it;
	}
	return 0;
}

int ModManager::find_mod_index (string name)
{
	for(int i=0; i<modlist->size(); i++)
	{
		ModInfo *it = (*modlist)[i];
		if (it->get_mod_title() == name)
			return i;
	}
	return -1;
}

void ModManager::add_mod (string mod, Configuration *modconfig)
{
	modlist->push_back(new ModInfo(type, title, mod, modconfig));
	store_system_paths();
}


// Checks the game 'gam' for the presence of the mod given in 'arg_modname'
// and checks the mod's compatibility. If the mod exists and is compatible with
// Exult, returns a reference to the mod; otherwise, returns the mod's parent game.
// Outputs error messages is the mod is not found or is not compatible.
BaseGameInfo *ModManager::get_mod(string name, bool checkversion)
{
	ModInfo *newgame = 0;
	if (has_mods())
		newgame = find_mod(name);
	if (newgame)
	{
		if (checkversion && !newgame->is_mod_compatible())
		{
			cerr << "Mod '" << name << "' is not compatible with this version of Exult." << endl;
			return 0;
		}
	}
	if (!newgame)
	{
		cerr << "Mod '" << name << "' not found." << endl;
		return 0;
	}
	return newgame;
}

/*
 *	Calculate paths for the given game, using the config file and
 *	falling back to defaults if necessary.  These are stored in
 *	per-game system_path entries, which are then used later once the
 *	game is selected.
 */
static void get_game_paths(const string &gametitle)
{
	string data_directory, static_dir, gamedat_dir, savegame_dir,
		default_dir, system_path_tag(to_uppercase(gametitle)),
		config_path("config/disk/game/" + gametitle + "/path");
	config->value(config_path.c_str(), data_directory, ".");
	if (data_directory == ".")
		config->set(config_path.c_str(), data_directory, true);
#if 0
	cout << "setting " << gametitle
		<< " game directories to: " << data_directory << endl;
#endif

	config_path = "config/disk/game/" + gametitle + "/static_path";
	default_dir = data_directory + "/static";
	config->value(config_path.c_str(), static_dir, default_dir.c_str());
	add_system_path("<" + system_path_tag + "_STATIC>", static_dir);
#if 0
	cout << "setting " << gametitle
		<< " static directory to: " << static_dir << endl;
#endif

	const char *home = 0;		// Will get $HOME.
	string home_game("");		// Gets $HOME/.exult/gametitle.
	config_path = "config/disk/game/" + gametitle + "/gamedat_path";
	default_dir = data_directory + "/gamedat";
	config->value(config_path.c_str(), gamedat_dir, "");
#if (!defined(WIN32) && !defined(MACOS))
	if (gamedat_dir == "" &&	// Not set?
					// And default doesn't exist?
	    !U7exists(default_dir.c_str()) && (home = getenv("HOME")) != 0)
		{
		home_game = home;
		home_game += "/.exult";
					// Create $HOME/.exult/gametitle.
		U7mkdir(home_game.c_str(), 0755);
		home_game = home_game + '/' + gametitle;
		U7mkdir(home_game.c_str(), 0755);
					// Successfully created dir?
		if (U7exists(home_game.c_str()))
			{		// Use $HOME/.exult/gametitle/gamedat.
			gamedat_dir = home_game + "/gamedat";
			config->set(config_path.c_str(), gamedat_dir.c_str(),
								true);
			}
		else
			home_game = "";	// Failed.
		}
#endif
	if (gamedat_dir == "")		// Didn't create it in $HOME/.exult?
		gamedat_dir = default_dir;
	add_system_path("<" + system_path_tag + "_GAMEDAT>", gamedat_dir);
#if 0
	cout << "setting " << gametitle
		<< " gamedat directory to: " << gamedat_dir << endl;
#endif

	config_path = "config/disk/game/" + gametitle + "/savegame_path";
	if (home_game == "")
		config->value(config_path.c_str(), savegame_dir, 
						data_directory.c_str());
	else
		{			// Store saves under $HOME/....
		config->value(config_path.c_str(), savegame_dir,
						home_game.c_str());
		config->set(config_path.c_str(), savegame_dir.c_str(), true);
		}
	add_system_path("<" + system_path_tag + "_SAVEGAME>", savegame_dir);
#if 0
	cout << "setting " << gametitle
		<< " savegame directory to: " << savegame_dir << endl;
#endif

	config_path = "config/disk/game/" + gametitle + "/patch";
	string patch_directory;
	default_dir = data_directory + "/patch";
	config->value(config_path.c_str(), patch_directory, 
							default_dir.c_str());
	add_system_path("<" + system_path_tag + "_PATCH>", patch_directory.c_str());

	config_path = "config/disk/game/" + gametitle + "/mods";
	string mods_directory;
	default_dir = data_directory + "/mods";
	config->value(config_path.c_str(), mods_directory, 
							default_dir.c_str());
	if (*(mods_directory.end()-1) == '/' || *(mods_directory.end()-1) == '\\')
	{
		// Remove any trailing slashes, just in case:
		mods_directory.resize(mods_directory.length()-1);
		config->set(config_path.c_str(), mods_directory, true);
	}
	add_system_path("<" + system_path_tag + "_MODS>", mods_directory.c_str());
}

// GameManager: class that manages the installed games
GameManager::GameManager()
{
	games = new std::vector<ModManager *>();

	// Search for games defined in exult.cfg:
	string config_path("config/disk/game"), game_title;
	std::vector<string> gamestrs = config->listkeys(config_path, false);
	games->reserve(gamestrs.size());

	// Get the paths for BG and SI:
	get_game_paths(CFG_BG_NAME);
	get_game_paths(CFG_SI_NAME);
	
	// Don't trust exult.cfg for BG and SI:
	set_bg_installed();
	set_si_installed();

	if (bg_installed)
	{
		ModManager *gambg = new ModManager(BLACK_GATE, CFG_BG_NAME, "ULTIMA VII\nTHE BLACK GATE");
		games->push_back(gambg);
	}
	if (si_installed)
	{
		ModManager *gamsi = new ModManager(SERPENT_ISLE, CFG_SI_NAME, "ULTIMA VII PART 2\nSERPENT ISLE");
		games->push_back(gamsi);
	}
	
	for (std::vector<string>::iterator it = gamestrs.begin();
				it != gamestrs.end(); it++)
	{
		string gameentry = *it;
		// BG and SI were already added above, so exclude them here:
		if (gameentry != CFG_BG_NAME && gameentry != CFG_SI_NAME)
		{
			// Load the paths for the found games:
			get_game_paths(gameentry);
			config->value(config_path + "/" + gameentry + "/title",
					game_title, "Missing Title");
			ModManager *devgam = new ModManager(EXULT_DEVEL_GAME, gameentry, game_title);
			games->push_back(devgam);
		}
	}
	store_system_paths();
}

GameManager::~GameManager ()
{
	for (int i = 0; i<games->size(); i++)
	{
		ModManager *exultgame = (*games)[i];
		delete exultgame;
	}
	games->clear();
}

void GameManager::set_bg_installed()
{
	string buf("<BLACKGATE_STATIC>/endgame.dat");
	bool foundbg = U7exists(buf);
	bool foundbgflx = U7exists("<DATA>/exult_bg.flx");

	if (foundbg)
		cout << "Black Gate   : found" << endl;
	else
		cout << "Black Gate   : not found (" 
				  << get_system_path(buf) << ")" << endl;

	if (foundbgflx)
		cout << "exult_bg.flx : found" << endl;
	else
		cout << "exult_bg.flx : not found (" 
				  << get_system_path("<DATA>/exult_bg.flx")
				  << ")" << endl;

	bg_installed = (foundbg && foundbgflx);
}

void GameManager::set_si_installed()
{
	string buf("<SERPENTISLE_STATIC>/sispeech.spc");
	bool foundsi = U7exists(buf);
	bool foundsiflx = U7exists("<DATA>/exult_si.flx");

	if (foundsi)
		cout << "Serpent Isle : found" << endl;
	else
		cout << "Serpent Isle : not found (" 
				  << get_system_path(buf) << ")" << endl;

	if (foundsiflx)
		cout << "exult_si.flx : found" << endl;
	else
		cout << "exult_si.flx : not found (" 
				  << get_system_path("<DATA>/exult_si.flx")
				  << ")" << endl;

	si_installed = (foundsi && foundsiflx);
}

ModManager *GameManager::find_game (string name)
{
	for(int i=0; i<games->size(); i++)
	{
		ModManager *it = (*games)[i];
		if (it->get_title() == name)
			return it;
	}
	return 0;
}

int GameManager::find_game_index (string name)
{
	for(int i=0; i<games->size(); i++)
	{
		ModManager *it = (*games)[i];
		if (it->get_title() == name)
			return i;
	}
	return -1;
}

void GameManager::add_game (string name, string menu)
{
	games->push_back(new ModManager(EXULT_DEVEL_GAME, name, menu));
	get_game_paths(name);
	store_system_paths();
}
