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

#ifndef KEYS_H
#define KEYS_H

#include "SDL_events.h"
#include "game.h"
#include "keyactions.h"

#include <vector>
#include <map>
#include <string>

class Scroll_gump;

const int c_maxparams = 4;

typedef void(*ActionFunc)(int*);

struct Action {
  ActionFunc func;
  std::string desc;
  bool show;
  bool cheat;
  Exult_Game game;
};



enum ActionCode {
        ACTION_NOTHING = -1,
        // Game Shell Actions (Save, Restore, Quit)
        ACTION_QUIT,
        ACTION_SAVE_RESTORE,
        ACTION_QUICKSAVE,
        ACTION_QUICKRESTORE,
        ACTION_ABOUT,
        ACTION_HELP,
        ACTION_CLOSE_GUMPS,
        ACTION_SCREENSHOT,

        // Game Viewport manipulation
        ACTION_REPAINT,
        ACTION_RESOLUTION_INCREASE,
        ACTION_RESOLUTION_DECREASE,
        ACTION_BRIGHTER,
	ACTION_DARKER,
        ACTION_TOGGLE_FULLSCREEN,

        // In-game actions
        ACTION_USEITEM,
        ACTION_TOGGLE_COMBAT,
        ACTION_TARGET_MODE,
        ACTION_INVENTORY,
        ACTION_TRY_KEYS,
        ACTION_STATS,

        // Cheats!
        ACTION_SHOW_SI_INTRO,
        ACTION_SHOW_ENDGAME,
        ACTION_SCROLL_LEFT,
        ACTION_SCROLL_RIGHT,
        ACTION_SCROLL_UP,
        ACTION_SCROLL_DOWN,
        ACTION_CENTER_SCREEN,
        ACTION_SHAPE_BROWSER,
        ACTION_CREATE_SHAPE,
        ACTION_DELETE_OBJECT,
        ACTION_TOGGLE_EGGS,
        ACTION_TOGGLE_GOD_MODE,
        ACTION_CHANGE_GENDER,
        ACTION_CHEAT_HELP,
        ACTION_TOGGLE_INFRAVISION,
        ACTION_SKIPLIFT_DECREMENT,
        ACTION_TOGGLE_MAP_EDITOR,
        ACTION_MAP_TELEPORT,
        ACTION_CURSOR_TELEPORT,
        ACTION_NEXT_TIME_PERIOD,
        ACTION_TOGGLE_WIZARD_MODE,
        ACTION_PARTY_HEAL,
        ACTION_PARTY_INCREASE_LEVEL,

        // Test actions
        ACTION_PLAY_MUSIC,
        ACTION_TOGGLE_NAKED,
        ACTION_TOGGLE_PETRA,
        ACTION_CHANGE_SKIN,
        ACTION_SOUND_TESTER
};

struct ActionType {
        ActionCode code;
        int params[c_maxparams];
};

struct ltSDLkeysym
{
        bool operator()(SDL_keysym k1, SDL_keysym k2) const
        {
                if (k1.sym == k2.sym)
                        return k1.mod < k2.mod;
                else
                        return k1.sym < k2.sym;
        }
};

typedef std::map<SDL_keysym, ActionType, ltSDLkeysym>   KeyMap;
typedef std::map<std::string, SDLKey> ParseKeyMap;
typedef std::map<std::string, ActionCode> ParseCodeMap;

class KeyBinder {
private:
        KeyMap bindings;
	ParseKeyMap keys;
	ParseCodeMap codes;

	std::vector<std::string> keyhelp;
	std::vector<std::string> cheathelp;
public:
        KeyBinder();
        ~KeyBinder();
        /* Add keybinding */
        void AddKeyBinding(SDLKey sym, int mod, ActionCode action,
			   int nparams, int* params);

        /* Delete keybinding */
        void DelKeyBinding(SDLKey sym, int mod);

        /* Other methods */
        void Flush() { bindings.clear(); }
        bool DoAction(ActionType action);
        bool HandleEvent(SDL_Event &ev);

	void LoadFromFile(const char* filename);
	void LoadDefaults();

	void ShowHelp();
	void ShowCheatHelp();

private:
	void ParseText(char *text);
	void ParseLine(char *line);
	void FillParseMaps();
};

#endif /* KEYS_H */
