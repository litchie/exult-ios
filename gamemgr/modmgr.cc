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
#include <fstream>
#include <string>
#include "modmgr.h"
#include "fnames.h"
#include "listfiles.h"
#include "exult_constants.h"
#include "utils.h"
#include "Configuration.h"
#include "Flex.h"
#include "databuf.h"

using std::ifstream;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;

// BaseGameInfo: Generic information and functions common to mods and games
void BaseGameInfo::setup_game_paths ()
	{
	// Make aliases to the current game's paths.
	string system_path_tag(to_uppercase(title.c_str()));
	clone_system_path("<STATIC>", "<" + system_path_tag + "_STATIC>");
	clone_system_path("<MODS>", "<" + system_path_tag + "_MODS>");

	if (!mod_title.empty())
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

static inline void ReplaceMacro
	(
	string &path,
	string srch,
	string repl
	)
	{
	string::size_type pos = path.find(srch);
	if (pos != string::npos)
		path.replace(pos, srch.length(), repl);
	}

// ModInfo: class that manages one mod's information
ModInfo::ModInfo
	(
	Exult_Game game,
	string name,
	string mod,
	bool exp,
	const Configuration& modconfig
	)
	{
	type = game;
	title = name;
	mod_title = mod;
	expansion = exp;

	string config_path, default_dir, modversion, savedir, patchdir, gamedatdir;
	
	config_path = "mod_info/mod_title";
	default_dir = mod;
	string modname;
	modconfig.value(config_path, modname, default_dir.c_str());
	mod_title = modname;

	config_path = "mod_info/display_string";
	default_dir = "Description missing!";
	string menustr;
	modconfig.value(config_path, menustr, default_dir.c_str());
	menustring = menustr;
	
	config_path = "mod_info/required_version";
	default_dir = "0.0.00R";
	modconfig.value(config_path, modversion, default_dir.c_str());
	if (modversion == default_dir)
		// Required version is missing; assume the mod to be incompatible
		compatible = false;
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

	const char *home = 0;
	string home_game("");		// Gets $HOME/.exult/title/tagstr.
	string save_path = data_directory;

#if (!defined(WIN32) && !defined(MACOS))
	if ((home = getenv("HOME")) != 0)
		{
		home_game = home;
		home_game += "/.exult";
					// Create $HOME/.exult/title.
		U7mkdir(home_game.c_str(), 0755);
		home_game = home_game + '/' + title;
		U7mkdir(home_game.c_str(), 0755);
					// Successfully created dir?
		if (U7exists(home_game.c_str()))
			{
			home_game = home_game + '/' + tagstr;
			U7mkdir(home_game.c_str(), 0755);
			if (U7exists(home_game.c_str()))
				save_path = home_game;
			}
		}
#endif

	config_path = "mod_info/gamedat_path";
	default_dir = save_path + "/gamedat";
	modconfig.value(config_path, gamedatdir, default_dir.c_str());
	// Path 'macros' for relative paths:
	ReplaceMacro(gamedatdir, mods_macro, mods_dir);
	ReplaceMacro(gamedatdir, mod_path_macro, save_path);
	add_system_path("<" + system_path_tag + "_GAMEDAT>", get_system_path(gamedatdir));

	config_path = "mod_info/savegame_path";
	modconfig.value(config_path, savedir, save_path.c_str());
	// Path 'macros' for relative paths:
	ReplaceMacro(savedir, mods_macro, mods_dir);
	ReplaceMacro(savedir, mod_path_macro, save_path);
	add_system_path("<" + system_path_tag + "_SAVEGAME>", get_system_path(savedir));
	
	config_path = "mod_info/patch";
	default_dir = data_directory + "/patch";
	modconfig.value(config_path, patchdir, default_dir.c_str());
	// Path 'macros' for relative paths:
	ReplaceMacro(patchdir, mods_macro, mods_dir);
	ReplaceMacro(patchdir, mod_path_macro, data_directory);
	add_system_path("<" + system_path_tag + "_PATCH>", get_system_path(patchdir));
	}

#ifdef HAVE_ZIP_SUPPORT
#include "files/zip/unzip.h"
#include "files/zip/zip.h"
#endif

// Need this for ES.
static char *get_game_identity(const char *savename, std::string title)
	{
	char *game_identity = 0;
	ifstream in_stream;
	if (!U7exists(savename))
		return newstrdup(title.c_str());
	if (!Flex::is_flex(savename))
#ifdef HAVE_ZIP_SUPPORT
		{
		unzFile unzipfile = unzOpen(get_system_path(savename).c_str());
		if (unzipfile)
			{
						// Find IDENTITY, ignoring case.
			if (unzLocateFile(unzipfile, "identity", 2) != UNZ_OK)
				{
				unzClose(unzipfile);
				return newstrdup("*");		// Old game.  Return wildcard.
				}
			else
				{
				unz_file_info file_info;
				unzGetCurrentFileInfo(unzipfile, &file_info, NULL,
						0, NULL, 0, NULL, 0);
				game_identity = new char[file_info.uncompressed_size + 1];

				if (unzOpenCurrentFile(unzipfile) != UNZ_OK)
					{
					unzClose(unzipfile);
					throw file_read_exception(savename);
					}
				unzReadCurrentFile(unzipfile, game_identity,
							file_info.uncompressed_size);
				if (unzCloseCurrentFile(unzipfile) == UNZ_OK)
							// 0-delimit.
					game_identity[file_info.uncompressed_size] = 0;
				}
			}
		}
#else
		return newstrdup(title.c_str());
#endif
	else
		{
		U7open(in_stream, savename);		// Open file.
		StreamDataSource in(&in_stream);

		in.seek(0x54);			// Get to where file count sits.
		int numfiles = in.read4();
		in.seek(0x80);			// Get to file info.
		// Read pos., length of each file.
		int *finfo = new int[2*numfiles];
		int i;
		for (i = 0; i < numfiles; i++)
			{
			finfo[2*i] = in.read4();	// The position, then the length.
			finfo[2*i + 1] = in.read4();
			}
		for (i = 0; i < numfiles; i++)	// Now read each file.
			{
			// Get file length.
			int len = finfo[2*i + 1] - 13;
			if (len <= 0)
				continue;
			in.seek(finfo[2*i]);	// Get to it.
			char fname[50];		// Set up name.
			in.read(fname, 13);
			if (!strcmp("identity",fname))
				{
				game_identity = new char[len];
				in.read(game_identity, len);
				break;
				}
			}
		in_stream.close();
		delete [] finfo;
		}
	if (!game_identity)
		return newstrdup(title.c_str());
	// Truncate identity
	char *ptr = game_identity;
	for(; (*ptr!=0x1a && *ptr!=0x0d); ptr++)
		;
	*ptr = 0;
	ptr = newstrdup(game_identity);
	delete [] game_identity;
	return ptr;
	}

// ModManager: class that manages a game's modlist and paths
ModManager::ModManager (string name, string menu)
	{
	title = name;
	mod_title = "";
	menustring = menu;
	to_uppercase(name);

		// We will NOT trust config with these values.
	string initgam_path("<" + name + "_STATIC>/initgame.dat");
	char *static_identity = get_game_identity(initgam_path.c_str(), title);
	if (!strcmp(static_identity,"ULTIMA7"))
		{
		type = BLACK_GATE;
		expansion = false;
		}
	else if (!strcmp(static_identity, "FORGE"))
		{
		type = BLACK_GATE;
		expansion = true;
		}
	else if (!strcmp(static_identity, "SERPENT ISLE"))
		{
		type = SERPENT_ISLE;
		expansion = false;
		}
	else if (!strcmp(static_identity, "SILVER SEED"))
		{
		type = SERPENT_ISLE;
		expansion = true;
		}
	else
		{
		type = EXULT_DEVEL_GAME;
		expansion = false;
		}
	delete[] static_identity;

	modlist.clear();

	FileList filenames;
	string pathname("<" + name + "_MODS>");
	int ptroff = get_system_path(pathname).length()+1;
	
	// If the dir doesn't exist, leave at once.
	if (!U7exists(pathname))
		return;

	U7ListFiles(pathname + "/*.cfg", filenames);
	int num_mods = filenames.size();

	if (num_mods > 0)
		{
		modlist.reserve(num_mods);
		for (int i = 0; i < num_mods; i++)
			{
			string modtitle = filenames[i].substr(ptroff,
					filenames[i].size() - ptroff - 4);
			modlist.push_back(ModInfo(type, title, modtitle, expansion,
					Configuration(filenames[i], "modinfo")));
			}
		}
	}

ModInfo *ModManager::find_mod (string name)
	{
	for (vector<ModInfo>::iterator it = modlist.begin();
			it != modlist.end(); ++it)
		if (it->get_mod_title() == name)
			return &*it;
	return 0;
	}

int ModManager::find_mod_index (string name)
	{
	for(int i = 0; i < modlist.size(); i++)
		if (modlist[i].get_mod_title() == name)
			return i;
	return -1;
	}

void ModManager::add_mod (string mod, Configuration& modconfig)
	{
	modlist.push_back(ModInfo(type, title, mod, expansion, modconfig));
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
		cerr << "Mod '" << name << "' not found." << endl;
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
		default_dir("./" + gametitle),
		system_path_tag(to_uppercase(gametitle)), config_path,
		base_cfg_path("config/disk/game/" + gametitle);
	config_path = base_cfg_path + "/path";
	config->value(config_path.c_str(), data_directory, default_dir.c_str());
	if (data_directory == default_dir)
		config->set(config_path.c_str(), data_directory, true);
#if 0
	cout << "setting " << gametitle
		<< " game directories to: " << data_directory << endl;
#endif

	config_path = base_cfg_path + "/static_path";
	default_dir = data_directory + "/static";
	config->value(config_path.c_str(), static_dir, default_dir.c_str());
	add_system_path("<" + system_path_tag + "_STATIC>", static_dir);
#if 0
	cout << "setting " << gametitle
		<< " static directory to: " << static_dir << endl;
#endif

	const char *home = 0;		// Will get $HOME.
	string home_game("");		// Gets $HOME/.exult/gametitle.
	config_path = base_cfg_path + "/gamedat_path";
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

	config_path = base_cfg_path + "/savegame_path";
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

	config_path = base_cfg_path + "/patch";
	string patch_directory;
	default_dir = data_directory + "/patch";
	config->value(config_path.c_str(), patch_directory, 
							default_dir.c_str());
	add_system_path("<" + system_path_tag + "_PATCH>", patch_directory.c_str());

	config_path = base_cfg_path + "/mods";
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
	games.clear();
	bg = fov = si = ss = 0;

	// Search for games defined in exult.cfg:
	string config_path("config/disk/game"), game_title;
	std::vector<string> gamestrs = config->listkeys(config_path, false);
	if (gamestrs.empty())
		{
			// The original games plus expansions.
		gamestrs.push_back(CFG_BG_NAME);
		gamestrs.push_back(CFG_FOV_NAME);
		gamestrs.push_back(CFG_SI_NAME);
		gamestrs.push_back(CFG_SS_NAME);
		}
	
	games.reserve(gamestrs.size());
	int bgind = -1, fovind = -1, siind = -1, ssind = -1;

	for (std::vector<string>::iterator it = gamestrs.begin();
			it != gamestrs.end(); ++it)
		{
		string gameentry = *it;
		// Load the paths for all games found:
		get_game_paths(gameentry);
		string name = gameentry, new_title;
		to_uppercase(name);
		name += "\nMissing Title";
		config->value(config_path + "/" + gameentry + "/title",
				game_title, name.c_str());
		bool need_title = game_title == name;
			// This checks static identity and sets game type.
		ModManager game = ModManager(gameentry, game_title);
		if (game.get_game_type() == BLACK_GATE)
			{
			if (game.have_expansion())
				{
				fovind = games.size();
				new_title = CFG_FOV_TITLE;
				}
			else
				{
				bgind = games.size();
				new_title = CFG_BG_TITLE;
				}
			}
		else if (game.get_game_type() == SERPENT_ISLE)
			{
			if (game.have_expansion())
				{
				ssind = games.size();
				new_title = CFG_SS_TITLE;
				}
			else
				{
				siind = games.size();
				new_title = CFG_SI_TITLE;
				}
			}
		if (need_title && new_title.size() > 0)
			game.set_menu_string(new_title);
		games.push_back(game);
		}
	store_system_paths();
	if (bgind >= 0)
		bg = &(games[bgind]);
	if (fovind >= 0)
		fov = &(games[fovind]);
	if (siind >= 0)
		si = &(games[siind]);
	if (ssind >= 0)
		ss = &(games[ssind]);
	print_found(bg, "exult_bg.flx", "Black Gate", CFG_BG_NAME);
	print_found(fov, "exult_bg.flx", "Forge of Virtue", CFG_FOV_NAME);
	print_found(si, "exult_si.flx", "Serpent Isle", CFG_SI_NAME);
	print_found(ss, "exult_si.flx", "Silver Seed", CFG_SS_NAME);
	}

void GameManager::print_found
	(
	ModManager *game,
	const char *flex,
	const char *title,
	const char *cfgname
	)
	{
	char path[50], fname[50];
	string cfgstr(cfgname);
	to_uppercase(cfgstr);
	snprintf(path, sizeof(path), "<%s_STATIC>/", cfgstr.c_str());
	snprintf(fname, sizeof(fname), "<DATA>/%s", flex);

	if (game != 0)
		cout << title << "   : found" << endl;
	else
		cout << title << "   : not found (" 
				  << get_system_path(path) << ")" << endl;

	if (U7exists(fname))
		cout << flex << " : found" << endl;
	else
		cout << flex << " : not found (" 
				  << get_system_path(fname)
				  << ")" << endl;

	}

ModManager *GameManager::find_game (string name)
	{
	for (vector<ModManager>::iterator it = games.begin();
			it != games.end(); ++it)
		if (it->get_title() == name)
			return &*it;
	return 0;
	}

int GameManager::find_game_index (string name)
	{
	for(int i=0; i < games.size(); i++)
		if (games[i].get_title() == name)
			return i;
	return -1;
	}

void GameManager::add_game (string name, string menu)
	{
	get_game_paths(name);
	games.push_back(ModManager(name, menu));
	store_system_paths();
	}
