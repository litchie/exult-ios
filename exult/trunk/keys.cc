/*
 *  Copyright (C) 2001 The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "SDL_keyboard.h"

#include "keys.h"
#include "game.h"
#include "cheat.h"
#include "U7file.h"
#include "Scroll_gump.h"
#include "mouse.h"
#include "gamewin.h"
#include "utils.h"
#include "keyactions.h"

extern Cheat cheat;

using std::pair;
using std::atoi;
using std::cerr;
using std::cout;
using std::clog;
using std::endl;
using std::ifstream;
using std::isspace;
using std::strchr;
using std::string;
using std::strlen;
using std::vector;

extern void to_uppercase(string &str);
extern int Get_click(int& x, int& y, Mouse::Mouse_shapes shape, char *chr = 0);

typedef void(*ActionFunc)(int*);

const struct Action {
	const char *s;
	ActionFunc func;
	const char* desc;
	bool show;
	bool cheat;
	Exult_Game game;
} ExultActions[] = {
	{ "QUIT", ActionQuit, "Quit", true, false, NONE },
	{ "SAVE_RESTORE", ActionFileGump, "Save/restore", true, false, NONE },
	{ "QUICKSAVE", ActionQuicksave, "Quick-save", true, false, NONE },
	{ "QUICKRESTORE", 
	  ActionQuickrestore, "Quick-restore", true, false, NONE },
	{ "ABOUT", ActionAbout, "About Exult", true, false, NONE },
	{ "HELP", ActionHelp, "List keys", true, false, NONE },
	{ "CLOSE_GUMPS", ActionCloseGumps, "Close gumps", false, false, NONE },
	{ "CLOSE_OR_MENU", ActionCloseOrMenu, "Game menu", true, false, NONE },
	{ "SCREENSHOT",
	  ActionScreenshot, "Take screenshot", true, false, NONE  },
	{ "GAME_MENU", ActionMenuGump, "Game Menu", true, false, NONE },
	{ "OLD_FILE_GUMP",
	  ActionOldFileGump, "Save/restore", true, false, NONE },
	
	{ "REPAINT", ActionRepaint, "Repaint screen", false, false, NONE },
	{ "RESOLUTION_INCREASE", 
	  ActionResIncrease, "Increase resolution", true, true, NONE },
	{ "RESOLUTION_DECREASE", 
	  ActionResDecrease,  "Decrease resolution", true, true, NONE },
	{ "BRIGHTER", 
	  ActionBrighter, "Increase brightness", true, false, NONE },
	{ "DARKER", ActionDarker, "Decrease brightness", true, false, NONE },
	{ "TOGGLE_FULLSCREEN", 
	  ActionFullscreen, "Toggle fullscreen", true, false, NONE },
	
	{ "USEITEM", ActionUseItem, "Use item", false, false, NONE },
	{ "TOGGLE_COMBAT", ActionCombat, "Toggle combat", true, false, NONE },
	{ "TARGET_MODE", ActionTarget, "Target mode", true, false, NONE },
	{ "INVENTORY", ActionInventory, "Show inventory", true, false, NONE },
	{ "TRY_KEYS", ActionTryKeys, "Try keys", true, false, NONE },
	{ "STATS", ActionStats, "Show stats", true, false, NONE },
	{ "COMBAT_STATS",
	  ActionCombatStats, "Show combat stats", true, false, SERPENT_ISLE },
	  
	
	{ "SHOW_SI_INTRO",
	  ActionSIIntro, "Show SI intro", true, true, SERPENT_ISLE },
	{ "SHOW_ENDGAME", ActionEndgame, "Show endgame", true, true, NONE },
	{ "SCROLL_LEFT", ActionScrollLeft, "Scroll left", true, true, NONE },
	{ "SCROLL_RIGHT", ActionScrollRight, "Scroll right", true, true, NONE },
	{ "SCROLL_UP", ActionScrollUp, "Scroll up", true, true, NONE },
	{ "SCROLL_DOWN", ActionScrollDown, "Scroll down", true, true, NONE },
	{ "CENTER_SCREEN", ActionCenter, "Center screen", true, true, NONE },
	{ "SHAPE_BROWSER",
	  ActionShapeBrowser, "Shape browser", true, true, NONE },
	{ "CREATE_ITEM",
	  ActionCreateShape, "Create last shape", true, true, NONE },
	{ "DELETE_OBJECT", 
	  ActionDeleteObject, "Delete object", true, true, NONE },
	{ "TOGGLE_EGGS",
	  ActionToggleEggs, "Toggle egg display", true, true, NONE },
	{ "TOGGLE_GOD_MODE",
	  ActionGodMode, "Toggle god mode", true, true, NONE },
    { "CHANGE_GENDER", 
	  ActionGender, "Change gender", true, true, NONE },
	{ "CHEAT_HELP", ActionCheatHelp, "List cheat keys", true, true, NONE },
	{ "TOGGLE_INFRAVISION",
	  ActionInfravision, "Toggle infravision", true, true, NONE },
	{ "SKIPLIFT_DECREMENT",
	  ActionSkipLift, "Decrement skiplift", true, true, NONE },
	{ "TOGGLE_MAP_EDITOR",
	  ActionMapEditor, "Toggle map-editor mode", true, true, NONE },
	{ "MAP_TELEPORT", ActionMapTeleport, "Map teleport", true, true, NONE },
	{ "CURSOR_TELEPORT",
	  ActionTeleport, "Teleport to cursor", true, true, NONE },
	{ "NEXT_TIME_PERIOD",
	  ActionTime, "Next time period", true, true, NONE },
	{ "TOGGLE_WIZARD_MODE",
	  ActionWizard, "Toggle archwizard mode", true, true, NONE },
	{ "PARTY_HEAL", ActionHeal, "Heal party", true, true, NONE },
	{ "PARTY_INCREASE_LEVEL",
	  ActionLevelup, "Level-up party", true, true, NONE },
	{ "CHEAT_SCREEN", ActionCheatScreen, "Cheat Screen", true, true, NONE },
	{ "PICK_POCKET",
	  ActionPickPocket, "Toggle Pick Pocket", true, true, NONE },
	{ "NPC_NUMBERS",
	  ActionNPCNumbers, "Toggle NPC Numbers", true, true, NONE },
	{ "GRAB_ACTOR",
	  ActionGrabActor, "Grab NPC for Cheat Screen", true, true, NONE },
	
	{ "PLAY_MUSIC", ActionPlayMusic, "Play song", false, true, NONE },
	{ "TOGGLE_NAKED",
	  ActionNaked, "Toggle naked mode", true, true, SERPENT_ISLE },
	{ "TOGGLE_PETRA",
	  ActionPetra, "Toggle Petra mode", true, true, SERPENT_ISLE },
	{ "CHANGE_SKIN",
	  ActionSkinColour, "Change skin colour", true, true, SERPENT_ISLE },
	{ "SOUND_TESTER", 
	  ActionSoundTester, "Sound tester", false, true, NONE },
	{ "TEST", ActionTest, "Test", false, false, NONE },
	{ "", 0, "", false, false, NONE } //terminator
};

const struct {
	const char *s;
	SDLKey k;
} SDLKeyStringTable[] = {
	{"BACKSPACE", SDLK_BACKSPACE},
	{"TAB",       SDLK_TAB}, 
	{"ENTER",     SDLK_RETURN}, 
	{"PAUSE",     SDLK_PAUSE}, 
	{"ESC",       SDLK_ESCAPE}, 
	{"SPACE",     SDLK_SPACE}, 
	{"DEL",       SDLK_DELETE}, 
	{"KP0",       SDLK_KP0}, 
	{"KP1",       SDLK_KP1}, 
	{"KP2",       SDLK_KP2}, 
	{"KP3",       SDLK_KP3}, 
	{"KP4",       SDLK_KP4}, 
	{"KP5",       SDLK_KP5}, 
	{"KP6",       SDLK_KP6}, 
	{"KP7",       SDLK_KP7}, 
	{"KP8",       SDLK_KP8}, 
	{"KP9",       SDLK_KP9}, 
	{"KP.",       SDLK_KP_PERIOD}, 
	{"KP/",       SDLK_KP_DIVIDE}, 
	{"KP*",       SDLK_KP_MULTIPLY}, 
	{"KP-",       SDLK_KP_MINUS}, 
	{"KP+",       SDLK_KP_PLUS}, 
	{"KP_ENTER",  SDLK_KP_ENTER}, 
	{"UP",        SDLK_UP}, 
	{"DOWN",      SDLK_DOWN}, 
	{"RIGHT",     SDLK_RIGHT}, 
	{"LEFT",      SDLK_LEFT}, 
	{"INSERT",    SDLK_INSERT}, 
	{"HOME",      SDLK_HOME}, 
	{"END",       SDLK_END}, 
	{"PAGEUP",    SDLK_PAGEUP}, 
	{"PAGEDOWN",  SDLK_PAGEDOWN}, 
	{"F1",        SDLK_F1}, 
	{"F2",        SDLK_F2}, 
	{"F3",        SDLK_F3}, 
	{"F4",        SDLK_F4}, 
	{"F5",        SDLK_F5}, 
	{"F6",        SDLK_F6}, 
	{"F7",        SDLK_F7}, 
	{"F8",        SDLK_F8}, 
	{"F9",        SDLK_F9}, 
	{"F10",       SDLK_F10}, 
	{"F11",       SDLK_F11}, 
	{"F12",       SDLK_F12}, 
	{"F13",       SDLK_F13}, 
	{"F14",       SDLK_F14}, 
	{"F15",       SDLK_F15},
	{"", SDLK_UNKNOWN} // terminator
};


typedef std::map<std::string, SDLKey> ParseKeyMap;
typedef std::map<std::string, const Action*> ParseActionMap;

static ParseKeyMap keys;
static ParseActionMap actions;


KeyBinder::KeyBinder()
{
	FillParseMaps();
}

KeyBinder::~KeyBinder()
{
}

void KeyBinder::AddKeyBinding( SDLKey key, int mod, const Action* action, 
				 int nparams, int* params)
{
	SDL_keysym k;
	ActionType a;
	
	k.scancode = 0;
	k.sym      = key;
	k.mod      = (SDLMod) mod;
	k.unicode  = 0;
	a.action    = action;
	for (int i = 0; i < c_maxparams && i < nparams; i++)
		a.params[i] = params[i];
	for (int i = nparams; i < c_maxparams; i++)
		a.params[i] = -1;
	
	bindings[k] = a;
}

bool KeyBinder::DoAction(ActionType a)
{
	if (a.action->cheat && !cheat())
		return true;
	if (a.action->game != NONE && a.action->game != Game::get_game_type())
		return true;
	
	a.action->func(a.params);
	
	return true;
}

bool KeyBinder::HandleEvent(SDL_Event &ev)
{
	SDL_keysym key = ev.key.keysym;
	KeyMap::iterator sdlkey_index;
	
	if (ev.type != SDL_KEYDOWN)
		return false;
	
	key.mod = KMOD_NONE;
	if (ev.key.keysym.mod & KMOD_SHIFT)
		key.mod = (SDLMod)(key.mod | KMOD_SHIFT);
	if (ev.key.keysym.mod & KMOD_CTRL)
		key.mod = (SDLMod)(key.mod | KMOD_CTRL);
#ifdef MACOS
	// map Meta to Alt on MacOS
	if (ev.key.keysym.mod & KMOD_META)
		key.mod = (SDLMod)(key.mod | KMOD_ALT);
#else
	if (ev.key.keysym.mod & KMOD_ALT)
		key.mod = (SDLMod)(key.mod | KMOD_ALT);
#endif
	
	sdlkey_index = bindings.find(key);
	if (sdlkey_index != bindings.end())
		return DoAction(sdlkey_index->second);
	
	return false;
}

void KeyBinder::ShowHelp()
{
	Scroll_gump *scroll;
	scroll = new Scroll_gump();
	
	vector<string>::iterator iter;
	
	for (iter = keyhelp.begin(); iter != keyhelp.end(); iter++)
		scroll->add_text(iter->c_str());
	
	scroll->paint(Game_window::get_game_window());
	do
	{
		int x, y;
		Get_click(x,y, Mouse::hand);
	} while (scroll->show_next_page(Game_window::get_game_window()));
	Game_window::get_game_window()->paint();
	delete scroll;
}

void KeyBinder::ShowCheatHelp()
{
	Scroll_gump *scroll;
	scroll = new Scroll_gump();
	
	vector<string>::iterator iter;
	
	for (iter = cheathelp.begin(); iter != cheathelp.end(); iter++)
		scroll->add_text(iter->c_str());
	
	scroll->paint(Game_window::get_game_window());
	do
	{
		int x, y;
		Get_click(x,y, Mouse::hand);
	} while (scroll->show_next_page(Game_window::get_game_window()));
	Game_window::get_game_window()->paint();
	delete scroll;
}

void KeyBinder::ParseText(char *text)
{
	char *ptr, *end;
	const char LF = '\n';
	
	ptr = text;
	
	// last (useful) line must end with LF
	while ((end = strchr(ptr, LF)) != 0) {
		*end = '\0';
		ParseLine(ptr);
		ptr = end + 1;
	}  
}

static void skipspace(string &s) {
	while (s.length() && isspace(s[0]))
		s.erase(0,1);
}

void KeyBinder::ParseLine(char *line)
{
	size_t i;
	SDL_keysym k;
	ActionType a;
	k.sym      = SDLK_UNKNOWN;
	k.mod      = KMOD_NONE;
	string s = line, u;
	string d, desc, keycode;
	bool show;
	
	skipspace(s);
	
	// comments and empty lines
	if (s.length() == 0 || s[0] == '#')
		return;
	
	u = s;
	to_uppercase(u);
	
	// get key
	while (s.length() && !isspace(s[0])) {
		// check modifiers
		//    if (u.compare("ALT-",0,4) == 0) {
		if (u.substr(0,4) == "ALT-") {
			k.mod = (SDLMod)(k.mod | KMOD_ALT);
			s.erase(0,4); u.erase(0,4);
			//    } else if (u.compare("CTRL-",0,5) == 0) {
		} else if (u.substr(0,5) == "CTRL-") {
			k.mod = (SDLMod)(k.mod | KMOD_CTRL);
			s.erase(0,5); u.erase(0,5);
			//    } else if (u.compare("SHIFT-",0,6) == 0) {
		} else if (u.substr(0,6) == "SHIFT-") {
			k.mod = (SDLMod)(k.mod | KMOD_SHIFT);
			s.erase(0,6); u.erase(0,6);
		} else {
			
			for (i = 0; i < s.length() && !isspace(s[i]); i++)
				;
			
			keycode = s.substr(0, i); s.erase(0, i);
			string t = keycode;
			to_uppercase(t);
			
			if (t.length() == 0) {
				cerr << "Keybinder: parse error in line: " << s << endl;
				return;
			} else if (t.length() == 1) {
				// translate 1-letter keys straight to SDLKey
				char c = t[0];
				if (c >= 33 && c <= 122 && c != 37) {
					if (c >= 'A' && c <= 'Z')
						c += 32; // need lowercase
					k.sym = (SDLKey)c;
				} else {
					cerr << "Keybinder: unsupported key: " << keycode << endl;
				}
			} else {
				// lookup in table
				ParseKeyMap::iterator key_index;
				key_index = keys.find(t);
				if (key_index != keys.end()) {
					k.sym = key_index->second;
				} else {
					cerr << "Keybinder: unsupported key: " << keycode << endl;
					return;
				}
			}
		}
	}
	
	if (k.sym == SDLK_UNKNOWN) {
		cerr << "Keybinder: parse error in line: " << s << endl;
		return;
	}
	
	// get function
	skipspace(s);
	
	for (i = 0; i < s.length() && !isspace(s[i]); i++)
		;
	string t = s.substr(0, i); s.erase(0, i);
	to_uppercase(t);
	
	ParseActionMap::iterator action_index;
	action_index = actions.find(t);
	if (action_index != actions.end()) {
		a.action = action_index->second;
	} else {
		cerr << "Keybinder: unsupported action: " << t << endl;
		return;
	}
	
	// get params
	skipspace(s);
	
	int np = 0;
	while (s.length() && s[0] != '#' && np < c_maxparams) {
		for (i = 0; i < s.length() && !isspace(s[i]); i++)
			;
		string t = s.substr(0, i);
		s.erase(0, i);
		skipspace(s);
		
		int p = atoi(t.c_str());
		a.params[np++] = p;
	}
	
	// read optional help comment
	if (s.length() >= 1 && s[0] == '#') {
		if (s.length() >= 2 && s[1] == '-') {
			show = false;
		} else {
			s.erase(0,1);
			skipspace(s);
			d = s;
			show = true;
		}
	} else {
		d = a.action->desc;
		show = a.action->show;
	}
	
	if (show) {
		desc = "";
		if (k.mod & KMOD_CTRL)
			desc += "Ctrl-";
#ifdef MACOS
		if (k.mod & KMOD_ALT)
			desc += "Cmd-";
#else
		if (k.mod & KMOD_ALT)
			desc += "Alt-";
#endif
		if (k.mod & KMOD_SHIFT)
			desc += "Shift-";
		desc += keycode;
		
		desc += " - " + d;
		
		// add to help list
		if (a.action->cheat)
			cheathelp.push_back(desc);
		else
			keyhelp.push_back(desc);
	}
	
	// bind key
	AddKeyBinding(k.sym, k.mod, a.action, np, a.params);
}

void KeyBinder::LoadFromFile(const char* filename)
{
	ifstream keyfile;
	
	Flush();
	
	cout << "Loading keybindings from file " << filename << endl;
	
	U7open(keyfile, filename, true);
	char temp[1024]; // 1024 should be long enough
	while(!keyfile.eof()) {
		keyfile.getline(temp, 1024);
		if (keyfile.gcount() >= 1023) {
			cerr << "Keybinder: parse error: line too long. Skipping rest of file." 
				 << endl;
			return;
		}
		ParseLine(temp);
	}
	keyfile.close();
}

void KeyBinder::LoadDefaults()
{
	Flush();
	
	cout << "Loading default keybindings" << endl;
	
	U7object txtobj("<DATA>/exult.flx", 23);
	size_t len;
	char *txt = txtobj.retrieve(len);
	
	ParseText(txt);
	
	delete [] txt;
}

// codes used in keybindings-files. (use uppercase here)
void KeyBinder::FillParseMaps()
{
	for (int i = 0; strlen(SDLKeyStringTable[i].s) > 0; i++)
		keys[SDLKeyStringTable[i].s] = SDLKeyStringTable[i].k;
	
	for (int i = 0; strlen(ExultActions[i].s) > 0; i++)
		actions[ExultActions[i].s] = &(ExultActions[i]);
}
