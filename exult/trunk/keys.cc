/*
 *  Copyright (C) 2000  Grant C. Likely
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

using std::atoi;
using std::cerr;
using std::endl;
using std::ifstream;
using std::isspace;
using std::strchr;
using std::string;
using std::vector;

extern void to_uppercase(string &str);
extern int Get_click(int& x, int& y, Mouse::Mouse_shapes shape, char *chr = 0);

typedef void(*ActionFunc)(int*);

struct Action {
  ActionFunc func;
  char* desc;
  bool show;
  bool cheat;
  Exult_Game game;
} ExultActions[] = {
  { ActionQuit, "Quit", true, false, NONE },
  { ActionFileGump, "Save/restore", true, false, NONE },
  { ActionQuicksave, "Quick-save", true, false, NONE },
  { ActionQuickrestore, "Quick-restore", true, false, NONE },
  { ActionAbout, "About Exult", true, false, NONE },
  { ActionHelp, "List keys", true, false, NONE },
  { ActionCloseGumps, "Close gumps", false, false, NONE },

  { ActionScreenshot, "Take screenshot", true, false, NONE },
  { ActionRepaint, "Repaint screen", false, false, NONE },
  { ActionResIncrease, "Increase resolution", true, true, NONE },
  { ActionResDecrease, "Decrease resolution", true, true, NONE },
  { ActionBrighter, "Increase brightness", true, false, NONE },
  { ActionDarker, "Decrease brightness", true, false, NONE },
  { ActionFullscreen, "Toggle fullscreen", true, false, NONE },

  { ActionUseItem, "Use item", false, false, NONE },
  { ActionCombat, "Toggle combat", true, false, NONE },
  { ActionTarget, "Target mode", true, false, NONE },
  { ActionInventory, "Show inventory", true, false, NONE },
  { ActionTryKeys, "Try keys", true, false, NONE },
  { ActionStats, "Show stats", true, false, NONE },

  { ActionSIIntro,  "Show SI intro", true, true, SERPENT_ISLE },
  { ActionEndgame, "Show endgame", true, true, BLACK_GATE },
  { ActionScrollLeft, "Scroll left", true, true, NONE },
  { ActionScrollRight, "Scroll right", true, true, NONE },
  { ActionScrollUp, "Scroll up", true, true, NONE },
  { ActionScrollDown, "Scroll down", true, true, NONE },
  { ActionCenter, "Center screen", true, true, NONE },
  { ActionShapeBrowser, "Shape browser", true, true, NONE },
  { ActionCreateShape, "Create last shape", true, true, NONE },
  { ActionDeleteObject, "Delete object", true, true, NONE },
  { ActionToggleEggs, "Toggle egg display", true, true, NONE },
  { ActionGodMode, "Toggle god mode", true, true, NONE },
  { ActionGender, "Change gender", true, true, NONE },
  { ActionCheatHelp, "List cheat keys", true, true, NONE },
  { ActionInfravision, "Toggle infravision", true, true, NONE },
  { ActionSkipLift, "Decrement skiplift", true, true, NONE },
  { ActionMapEditor, "Toggle map-editor mode", true, true, NONE },
  { ActionMapTeleport, "Map teleport", true, true, NONE },
  { ActionTeleport, "Teleport to cursor", true, true, NONE },
  { ActionTime, "Next time period", true, true, NONE },
  { ActionWizard, "Toggle archwizard mode", true, true, NONE },
  { ActionHeal, "Heal party", true, true, NONE },
  { ActionLevelup, "Level-up party", true, true, NONE },

  { ActionPlayMusic, "Play song", false, true, NONE },
  { ActionNaked, "Toggle naked mode", true, true, SERPENT_ISLE },
  { ActionPetra, "Toggle Petra mode", true, true, SERPENT_ISLE },
  { ActionSkinColour, "Change skin colour", true, true, SERPENT_ISLE },
  { ActionSoundTester, "Sound tester", false, true, NONE }
};

KeyBinder::KeyBinder()
{
  FillParseMaps();
}

KeyBinder::~KeyBinder()
{
}

void KeyBinder::AddKeyBinding( SDLKey key, int mod, ActionCode  action, 
				 int nparams, int* params)
{
  SDL_keysym k;
  ActionType a;

  k.scancode = 0;
  k.sym      = key;
  k.mod      = (SDLMod) mod;
  k.unicode  = 0;
  a.code    = action;
  for (int i = 0; i < c_maxparams && i < nparams; i++)
    a.params[i] = params[i];
  for (int i = nparams; i < c_maxparams; i++)
    a.params[i] = -1;

  bindings[k] = a;
}

bool KeyBinder::DoAction(ActionType a)
{
  if (a.code == ACTION_NOTHING) return false;

  Action& action = ExultActions[(int)a.code];

  if (action.cheat && !cheat())
    return true;
  if (action.game != NONE && action.game != Game::get_game_type())
    return true;

  action.func(a.params);

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
#ifdef MACOS
  const char LF = '\r';
#else
  const char LF = '\n';
#endif

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
  int i;
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
     
      for (i = 0; i < s.length() && !isspace(s[i]); i++);

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

  for (i = 0; i < s.length() && !isspace(s[i]); i++);
  string t = s.substr(0, i); s.erase(0, i);
  to_uppercase(t);

  ParseCodeMap::iterator code_index;
  code_index = codes.find(t);
  if (code_index != codes.end()) {
    a.code = code_index->second;
  } else {
    cerr << "Keybinder: unsupported action: " << t << endl;
    return;
  }

  // get params
  skipspace(s);

  int np = 0;
  while (s.length() && s[0] != '#' && np < c_maxparams) {
    for (i = 0; i < s.length() && !isspace(s[i]); i++);
    string t = s.substr(0, i); s.erase(0, i); skipspace(s);
    
    int p = atoi(t.c_str());
    a.params[np++] = p;
  }

  // read optional help comment
  if (s[0] == '#') {
    s.erase(0,1);
    skipspace(s);
    d = s;
    show = true;
    
  } else {
    d = ExultActions[a.code].desc;
    show = ExultActions[a.code].show;
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
    if (ExultActions[a.code].cheat)
      cheathelp.push_back(desc);
    else
      keyhelp.push_back(desc);
  }

  // bind key
  AddKeyBinding(k.sym, k.mod, a.code, np, a.params);
}

void KeyBinder::LoadFromFile(const char* filename)
{
  ifstream keyfile;

  Flush();

#ifdef DEBUG
  cerr << "Loading keybindings from file " << filename << endl;
#endif

  U7open(keyfile, filename);
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

#ifdef DEBUG
  cerr << "Loading default keybindings" << endl;
#endif

  U7object txtobj("<DATA>/exult.flx", 23);
  size_t len;
  char *txt = txtobj.retrieve(len);

  ParseText(txt);

  delete [] txt;
}

// codes used in keybindings-files. (use uppercase here)
void KeyBinder::FillParseMaps()
{
  keys["BACKSPACE"] = SDLK_BACKSPACE;
  keys["TAB"]       = SDLK_TAB;
  keys["ENTER"]     = SDLK_RETURN;
  keys["PAUSE"]     = SDLK_PAUSE;
  keys["ESC"]       = SDLK_ESCAPE;
  keys["SPACE"]     = SDLK_SPACE;
  keys["DEL"]       = SDLK_DELETE;
  keys["KP0"]       = SDLK_KP0;
  keys["KP1"]       = SDLK_KP1;
  keys["KP2"]       = SDLK_KP2;
  keys["KP3"]       = SDLK_KP3;
  keys["KP4"]       = SDLK_KP4;
  keys["KP5"]       = SDLK_KP5;
  keys["KP6"]       = SDLK_KP6;
  keys["KP7"]       = SDLK_KP7;
  keys["KP8"]       = SDLK_KP8;
  keys["KP9"]       = SDLK_KP9;
  keys["KP."]       = SDLK_KP_PERIOD;
  keys["KP/"]       = SDLK_KP_DIVIDE;
  keys["KP*"]       = SDLK_KP_MULTIPLY;
  keys["KP-"]       = SDLK_KP_MINUS;
  keys["KP+"]       = SDLK_KP_PLUS;
  keys["KP_ENTER"]  = SDLK_KP_ENTER;
  
  keys["UP"]        = SDLK_UP;
  keys["DOWN"]      = SDLK_DOWN;
  keys["RIGHT"]     = SDLK_RIGHT;
  keys["LEFT"]      = SDLK_LEFT;
  keys["INSERT"]    = SDLK_INSERT;
  keys["HOME"]      = SDLK_HOME;
  keys["END"]       = SDLK_END;
  keys["PAGEUP"]    = SDLK_PAGEUP;
  keys["PAGEDOWN"]  = SDLK_PAGEDOWN;
  
  keys["F1"]        = SDLK_F1;
  keys["F2"]        = SDLK_F2;
  keys["F3"]        = SDLK_F3;
  keys["F4"]        = SDLK_F4;
  keys["F5"]        = SDLK_F5;
  keys["F6"]        = SDLK_F6;
  keys["F7"]        = SDLK_F7;
  keys["F8"]        = SDLK_F8;
  keys["F9"]        = SDLK_F9;
  keys["F10"]       = SDLK_F10;
  keys["F11"]       = SDLK_F11;
  keys["F12"]       = SDLK_F12;
  keys["F13"]       = SDLK_F13;
  keys["F14"]       = SDLK_F14;
  keys["F15"]       = SDLK_F15;

  codes["QUIT"] = ACTION_QUIT;
  codes["SAVE_RESTORE"] = ACTION_SAVE_RESTORE;
  codes["QUICKSAVE"] = ACTION_QUICKSAVE;
  codes["QUICKRESTORE"] = ACTION_QUICKRESTORE;
  codes["ABOUT"] = ACTION_ABOUT;
  codes["HELP"] = ACTION_HELP;
  codes["CLOSE_GUMPS"] = ACTION_CLOSE_GUMPS;
  codes["SCREENSHOT"] = ACTION_SCREENSHOT;

  codes["REPAINT"] = ACTION_REPAINT;
  codes["RESOLUTION_INCREASE"] = ACTION_RESOLUTION_INCREASE;
  codes["RESOLUTION_DECREASE"] = ACTION_RESOLUTION_DECREASE;
  codes["BRIGHTER"] = ACTION_BRIGHTER;
  codes["DARKER"] = ACTION_DARKER;
  codes["TOGGLE_FULLSCREEN"] = ACTION_TOGGLE_FULLSCREEN;

  codes["USEITEM"] = ACTION_USEITEM;
  codes["TOGGLE_COMBAT"] = ACTION_TOGGLE_COMBAT;
  codes["TARGET_MODE"] = ACTION_TARGET_MODE;
  codes["INVENTORY"] = ACTION_INVENTORY;
  codes["TRY_KEYS"] = ACTION_TRY_KEYS;
  codes["STATS"] = ACTION_STATS;

  codes["SHOW_SI_INTRO"] = ACTION_SHOW_SI_INTRO;
  codes["SHOW_ENDGAME"] = ACTION_SHOW_ENDGAME;
  codes["SCROLL_LEFT"] = ACTION_SCROLL_LEFT;
  codes["SCROLL_RIGHT"] = ACTION_SCROLL_RIGHT;
  codes["SCROLL_UP"] = ACTION_SCROLL_UP;
  codes["SCROLL_DOWN"] = ACTION_SCROLL_DOWN;
  codes["CENTER_SCREEN"] = ACTION_CENTER_SCREEN;
  codes["SHAPE_BROWSER"] = ACTION_SHAPE_BROWSER;
  codes["CREATE_ITEM"] = ACTION_CREATE_SHAPE;
  codes["DELETE_OBJECT"] = ACTION_DELETE_OBJECT;
  codes["TOGGLE_EGGS"] = ACTION_TOGGLE_EGGS;
  codes["TOGGLE_GOD_MODE"] = ACTION_TOGGLE_GOD_MODE;
  codes["CHANGE_GENDER"] = ACTION_CHANGE_GENDER;
  codes["CHEAT_HELP"] = ACTION_CHEAT_HELP;
  codes["TOGGLE_INFRAVISION"] = ACTION_TOGGLE_INFRAVISION;
  codes["SKIPLIFT_DECREMENT"] = ACTION_SKIPLIFT_DECREMENT;
  codes["TOGGLE_MAP_EDITOR"] = ACTION_TOGGLE_MAP_EDITOR;
  codes["MAP_TELEPORT"] = ACTION_MAP_TELEPORT;
  codes["CURSOR_TELEPORT"] = ACTION_CURSOR_TELEPORT;
  codes["NEXT_TIME_PERIOD"] = ACTION_NEXT_TIME_PERIOD;
  codes["TOGGLE_WIZARD_MODE"] = ACTION_TOGGLE_WIZARD_MODE;
  codes["PARTY_HEAL"] = ACTION_PARTY_HEAL;
  codes["PARTY_INCREASE_LEVEL"] = ACTION_PARTY_INCREASE_LEVEL;
  
  codes["PLAY_MUSIC"] = ACTION_PLAY_MUSIC;
  codes["TOGGLE_NAKED"] = ACTION_TOGGLE_NAKED;
  codes["TOGGLE_PETRA"] = ACTION_TOGGLE_PETRA;
  codes["CHANGE_SKIN"] = ACTION_CHANGE_SKIN;
  codes["SOUND_TESTER"] = ACTION_SOUND_TESTER;
}
