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
#include <cstdlib>
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

#if 0
#define DEBUG_PATHS 1
#endif

// BaseGameInfo: Generic information and functions common to mods and games
void BaseGameInfo::setup_game_paths ()
	{
	// Make aliases to the current game's paths.
	clone_system_path("<STATIC>", "<" + path_prefix + "_STATIC>");
	clone_system_path("<MODS>", "<" + path_prefix + "_MODS>");

	string mod_path_tag = path_prefix;

	if (!mod_title.empty())
		mod_path_tag += ("_" + to_uppercase((const string)mod_title));

	clone_system_path("<GAMEDAT>", "<" + mod_path_tag + "_GAMEDAT>");
	clone_system_path("<SAVEGAME>", "<" + mod_path_tag + "_SAVEGAME>");

	if (is_system_path_defined("<" + mod_path_tag + "_PATCH>"))
		clone_system_path("<PATCH>", "<" + mod_path_tag + "_PATCH>");
	else
		clear_system_path("<PATCH>");

	if (type != EXULT_MENU_GAME)
		{
		U7mkdir("<SAVEGAME>", 0755);	// make sure savegame directory exists
		U7mkdir("<GAMEDAT>", 0755);		// make sure gamedat directory exists
		}
	}

static inline void ReplaceMacro
	(
	string& path,
	const string& srch,
	const string& repl
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
	const string& name,
	const string& mod,
	const string& path,
	bool exp,
	bool ed,
	const string& cfg
	)
	{
	type = game;
	cfgname = name;
	mod_title = mod;
	path_prefix = path;
	expansion = exp;
	editing = ed;
	configfile = cfg;
	Configuration modconfig(configfile, "modinfo");

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
		char *eptrmod, *eptrver;
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

	string tagstr(to_uppercase((const string)mod_title)),
		system_path_tag(path_prefix + "_" + tagstr),
		mods_dir("<" + path_prefix + "_MODS>"),
		data_directory(mods_dir + "/" + mod_title),
		mods_save_dir("<" + path_prefix + "_SAVEGAME>/mods"),
		savedata_directory(mods_save_dir + "/" + mod_title),
		mods_macro("__MODS__"), mod_path_macro("__MOD_PATH__");

	// Read codepage first.
	config_path = "mod_info/codepage";
	default_dir = "CP437";	// DOS code page.
	modconfig.value(config_path, codepage, default_dir.c_str());
	
	// Where game data is. This is defaults to a non-writable location because
	// mods_dir does too.
	config_path = "mod_info/patch";
	default_dir = data_directory + "/patch";
	modconfig.value(config_path, patchdir, default_dir.c_str());
	// Path 'macros' for relative paths:
	ReplaceMacro(patchdir, mods_macro, mods_dir);
	ReplaceMacro(patchdir, mod_path_macro, data_directory);
	add_system_path("<" + system_path_tag + "_PATCH>", get_system_path(patchdir));

	U7mkdir(mods_save_dir.c_str(), 0755);
	U7mkdir(savedata_directory.c_str(), 0755);

	// The following paths default to user-writable locations.
	config_path = "mod_info/gamedat_path";
	default_dir = savedata_directory + "/gamedat";
	modconfig.value(config_path, gamedatdir, default_dir.c_str());
	// Path 'macros' for relative paths:
	ReplaceMacro(gamedatdir, mods_macro, mods_save_dir);
	ReplaceMacro(gamedatdir, mod_path_macro, savedata_directory);
	add_system_path("<" + system_path_tag + "_GAMEDAT>", get_system_path(gamedatdir));
	U7mkdir(gamedatdir.c_str(), 0755);

	config_path = "mod_info/savegame_path";
	modconfig.value(config_path, savedir, savedata_directory.c_str());
	// Path 'macros' for relative paths:
	ReplaceMacro(savedir, mods_macro, mods_save_dir);
	ReplaceMacro(savedir, mod_path_macro, savedata_directory);
	add_system_path("<" + system_path_tag + "_SAVEGAME>", get_system_path(savedir));
	U7mkdir(savedir.c_str(), 0755);

#ifdef DEBUG_PATHS
	cout << "path prefix of " << cfgname << " mod " << mod_title
		<< " is: " << system_path_tag << endl;
	cout << "setting " << cfgname
		<< " game directories to: " << get_system_path(gamedatdir) << endl;
	cout << "setting " << cfgname
		<< " game directories to: " << get_system_path(savedir) << endl;
	cout << "setting " << cfgname
		<< " game directories to: " << get_system_path(patchdir) << endl;
#endif
	}

#ifdef HAVE_ZIP_SUPPORT
#include "files/zip/unzip.h"
#include "files/zip/zip.h"
#endif

// Need this for ES.
static const char *get_game_identity(const char *savename, const string& title)
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
					delete [] game_identity;
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
ModManager::ModManager (const string& name, const string& menu, bool needtitle)
	{
	cfgname = name;
	mod_title = "";
	to_uppercase(name);

		// We will NOT trust config with these values.
		// We MUST NOT use path tags at this point yet!
	string game_path, static_dir, base_cfg_path("config/disk/game/" + cfgname);
		{
		string default_dir, config_path;

			// ++++ These path settings are for that game data which requires only
			// ++++ read access. They default to a subdirectory of:
			// ++++ 	*nix: /usr/local/share/exult or /usr/share/exult
			// ++++ 	MacOS X: /Library/Application Support/Exult
			// ++++ 	Windows, MacOS, BeOS, Amiga: program path.

			// <path> setting: default is "$gameprefix".
		config_path = base_cfg_path + "/path";
		default_dir = get_system_path("<GAMEHOME>") + "/" + cfgname;
		config->value(config_path.c_str(), game_path, default_dir.c_str());
	
			// <static_path> setting: default is "$game_path/static".
		config_path = base_cfg_path + "/static_path";
		default_dir = game_path + "/static";
		config->value(config_path.c_str(), static_dir, default_dir.c_str());

		// Read codepage too.
		config_path = base_cfg_path + "/codepage";
		default_dir = "CP437";	// DOS code page.
		config->value(config_path, codepage, default_dir.c_str());

		// And edit flag.
		string cfgediting;
		config_path = base_cfg_path + "/editing";
		default_dir = "no";		// Not editing.
		config->value(config_path, cfgediting, default_dir.c_str());
		editing = (cfgediting == "yes");
		}

	cout << "Looking for '" << cfgname
		<< "' at '" << game_path << "'... ";
	string initgam_path(static_dir + "/initgame.dat");
	found = U7exists(initgam_path);

	const char *static_identity;
	if (found)
		{
		static_identity = get_game_identity(initgam_path.c_str(), cfgname);
		cout << "found game with identity '" << static_identity << "'" << endl;
		}
	else	// New game still under development.
		{
		static_identity = "DEVEL GAME";
		cout << "but it wasn't there." << endl;
		}

	string new_title;
	if (!strcmp(static_identity,"ULTIMA7"))
		{
		type = BLACK_GATE;
		path_prefix = to_uppercase(CFG_BG_NAME);
		if (needtitle)
			new_title = CFG_BG_TITLE;
		expansion = false;
		}
	else if (!strcmp(static_identity, "FORGE"))
		{
		type = BLACK_GATE;
		path_prefix = to_uppercase(CFG_FOV_NAME);
		if (needtitle)
			new_title = CFG_FOV_TITLE;
		expansion = true;
		}
	else if (!strcmp(static_identity, "SERPENT ISLE"))
		{
		type = SERPENT_ISLE;
		path_prefix = to_uppercase(CFG_SI_NAME);
		if (needtitle)
			new_title = CFG_SI_TITLE;
		expansion = false;
		}
	else if (!strcmp(static_identity, "SILVER SEED"))
		{
		type = SERPENT_ISLE;
		path_prefix = to_uppercase(CFG_SS_NAME);
		if (needtitle)
			new_title = CFG_SS_TITLE;
		expansion = true;
		}
	else
		{
		type = EXULT_DEVEL_GAME;
		path_prefix = "DEVEL" + to_uppercase(name);
		new_title = menu;	// To be safe.
		expansion = false;
		}

	if (found)
		delete[] static_identity;

	menustring = needtitle ? new_title : menu;
		// NOW we can store the path.
	add_system_path("<" + path_prefix + "_STATIC>", static_dir);

		{
		string patch_dir, mods_dir, default_dir, config_path;

		// <mods> setting: default is "$game_path/mods".
		config_path = base_cfg_path + "/mods";
		default_dir = game_path + "/mods";
		config->value(config_path.c_str(), mods_dir, default_dir.c_str());
		add_system_path("<" + path_prefix + "_MODS>", mods_dir);

		// <patch> setting: default is "$game_path/patch".
		config_path = base_cfg_path + "/patch";
		default_dir = game_path + "/patch";
		config->value(config_path.c_str(), patch_dir, default_dir.c_str());
		add_system_path("<" + path_prefix + "_PATCH>", patch_dir);
#ifdef DEBUG_PATHS
		cout << "path prefix of " << cfgname
			<< " is: " << path_prefix << endl;
		cout << "setting " << cfgname
			<< " static directory to: " << static_dir << endl;
		cout << "setting " << cfgname
			<< " patch directory to: " << patch_dir << endl;
		cout << "setting " << cfgname
			<< " modifications directory to: " << mods_dir << endl;
#endif
		}

	get_game_paths(game_path);
	gather_mods();
	}

void ModManager::gather_mods()
	{
	modlist.clear();	// Just to be on the safe side.

	FileList filenames;
	string pathname("<" + path_prefix + "_MODS>");
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
			modlist.push_back(ModInfo(type, cfgname,
					modtitle, path_prefix, expansion,
					editing, filenames[i]));
			}
		}
	}

ModInfo *ModManager::find_mod (const string& name)
	{
	for (vector<ModInfo>::iterator it = modlist.begin();
			it != modlist.end(); ++it)
		if (it->get_mod_title() == name)
			return &*it;
	return 0;
	}

int ModManager::find_mod_index (const string& name)
	{
	for(size_t i = 0; i < modlist.size(); i++)
		if (modlist[i].get_mod_title() == name)
			return i;
	return -1;
	}

void ModManager::add_mod (const string& mod, const string& modconfig)
	{
	modlist.push_back(ModInfo(type, cfgname, mod, path_prefix,
			expansion, editing, modconfig));
	store_system_paths();
	}


// Checks the game 'gam' for the presence of the mod given in 'arg_modname'
// and checks the mod's compatibility. If the mod exists and is compatible with
// Exult, returns a reference to the mod; otherwise, returns the mod's parent game.
// Outputs error messages is the mod is not found or is not compatible.
BaseGameInfo *ModManager::get_mod(const string& name, bool checkversion)
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
void ModManager::get_game_paths(const string& game_path)
	{
	string saveprefix(get_system_path("<SAVEHOME>") + "/" + cfgname),
		default_dir, config_path, gamedat_dir, static_dir, savegame_dir,
		base_cfg_path("config/disk/game/" + cfgname);
	
		// ++++ All of these are directories with read/write requirements.
		// ++++ They default to a directory in the current user's profile,
		// ++++ with Win9x and old MacOS (possibly others) being exceptions.

		// Usually for Win9x:
	if (saveprefix == ".")
		saveprefix = game_path;

		// <savegame_path> setting: default is "$dataprefix".
	config_path = base_cfg_path + "/savegame_path";
	default_dir = saveprefix;
	config->value(config_path.c_str(), savegame_dir, default_dir.c_str());
	add_system_path("<" + path_prefix + "_SAVEGAME>", savegame_dir);
	U7mkdir(savegame_dir.c_str(), 0755);

		// <gamedat_path> setting: default is "$dataprefix/gamedat".
	config_path = base_cfg_path + "/gamedat_path";
	default_dir = saveprefix + "/gamedat";
	config->value(config_path.c_str(), gamedat_dir, default_dir.c_str());
	add_system_path("<" + path_prefix + "_GAMEDAT>", gamedat_dir);
	U7mkdir(gamedat_dir.c_str(), 0755);

#ifdef DEBUG_PATHS
		<< " gamedat directory to: " << gamedat_dir << endl;
	cout << "setting " << cfgname
		<< " savegame directory to: " << savegame_dir << endl;
#endif
	}

// GameManager: class that manages the installed games
GameManager::GameManager()
	{
	games.clear();
	bg = fov = si = ss = 0;

	// Search for games defined in exult.cfg:
	string config_path("config/disk/game"), game_title;
	std::vector<string> gamestrs = config->listkeys(config_path, false);
	
	bool nobg = true, nosi = true, nofov = true, noss = true;
	for (std::vector<string>::iterator it = gamestrs.begin();
			it != gamestrs.end(); ++it)
		{
		if (*it == CFG_BG_NAME)
			nobg = false;
		else if (*it == CFG_FOV_NAME)
			nofov = false;
		else if (*it == CFG_SI_NAME)
			nosi = false;
		else if (*it == CFG_SS_NAME)
			noss = false;
		}

		// The original games plus expansions.
	if (nobg ) gamestrs.push_back(CFG_BG_NAME);
	if (nofov) gamestrs.push_back(CFG_FOV_NAME);
	if (nosi ) gamestrs.push_back(CFG_SI_NAME);
	if (noss ) gamestrs.push_back(CFG_SS_NAME);
	
	games.reserve(gamestrs.size());
	int bgind = -1, fovind = -1, siind = -1, ssind = -1;

	for (std::vector<string>::iterator it = gamestrs.begin();
			it != gamestrs.end(); ++it)
		{
		string gameentry = *it;
		// Load the paths for all games found:
		string base_title = gameentry, new_title;
		to_uppercase(base_title);
		base_title += "\nMissing Title";
		config->value(config_path + "/" + gameentry + "/title",
				game_title, base_title.c_str());
		bool need_title = game_title == base_title;
			// This checks static identity and sets game type.
		ModManager game = ModManager(gameentry, game_title, need_title);
		if (!game.being_edited() && !game.is_there())
			continue;
		if (game.get_game_type() == BLACK_GATE)
			{
			if (game.have_expansion())
				fovind = games.size();
			else
				bgind = games.size();
			}
		else if (game.get_game_type() == SERPENT_ISLE)
			{
			if (game.have_expansion())
				ssind = games.size();
			else
				siind = games.size();
			}

		games.push_back(game);
		}

	if (bgind >= 0)
		bg = &(games[bgind]);
	if (fovind >= 0)
		fov = &(games[fovind]);
	if (siind >= 0)
		si = &(games[siind]);
	if (ssind >= 0)
		ss = &(games[ssind]);
		
		// Sane defaults.
	add_system_path("<ULTIMA7_STATIC>", ".");
	add_system_path("<SERPENT_STATIC>", ".");
	print_found(bg, "exult_bg.flx", "Black Gate", CFG_BG_NAME, "ULTIMA7");
	print_found(fov, "exult_bg.flx", "Forge of Virtue", CFG_FOV_NAME, "ULTIMA7");
	print_found(si, "exult_si.flx", "Serpent Isle", CFG_SI_NAME, "SERPENT");
	print_found(ss, "exult_si.flx", "Silver Seed", CFG_SS_NAME, "SERPENT");

	store_system_paths();
	}

void GameManager::print_found
	(
	ModManager *game,
	const char *flex,
	const char *title,
	const char *cfgname,
	const char *basepath
	)
	{
	char path[50];
	string cfgstr(cfgname);
	to_uppercase(cfgstr);
	snprintf(path, sizeof(path), "<%s_STATIC>/", cfgstr.c_str());

	if (game != 0)
		cout << title << "   : found" << endl;
	else
		{
		cout << title << "   : not found (" 
				  << get_system_path(path) << ")" << endl;
		return;
		}

	// This stores the BG/SI static paths (preferring the expansions)
	// for easier support of things like multiracial avatars in BG.
	char staticpath[50];
	snprintf(path, sizeof(path), "<%s_STATIC>", cfgstr.c_str());
	snprintf(staticpath, sizeof(staticpath), "<%s_STATIC>", basepath);
	clone_system_path(staticpath, path);

	snprintf(path, sizeof(path), "<DATA>/%s", flex);
	if (U7exists(path))
		cout << flex << " : found" << endl;
	else
		cout << flex << " : not found (" 
				  << get_system_path(path)
				  << ")" << endl;

	}

ModManager *GameManager::find_game (const string& name)
	{
	for (vector<ModManager>::iterator it = games.begin();
			it != games.end(); ++it)
		if (it->get_cfgname() == name)
			return &*it;
	return 0;
	}

int GameManager::find_game_index (const string& name)
	{
	for(size_t i=0; i < games.size(); i++)
		if (games[i].get_cfgname() == name)
			return i;
	return -1;
	}

void GameManager::add_game (const string& name, const string& menu)
	{
	games.push_back(ModManager(name, menu, false));
	store_system_paths();
	}
