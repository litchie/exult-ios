/*
Copyright (C) 2001 The Exult Team

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

#ifndef _POCKETPC_KEYBOARD_GUMP_H
#define _POCKETPC_KEYBOARD_GUMP_H

#ifdef UNDER_CE

#define KEYBOARD_GUMP_NUMBUTTONS 48

enum KEYG_STATES {KEYG_SHOWN, KEYG_MINIMIZED, KEYG_HIDDEN, KEYG_UNKNOWN};
enum KEYG_LOCATIONS {KEYG_LOCBOTTOM, KEYG_LOCTOP, KEYG_LOCUNKNOWN};
class Keyboard_gump
{
 public:
	Keyboard_gump(int placex = 0, int placey = 0, bool upperCase = true);
	~Keyboard_gump();

	const char *caseSet;
	Vga_file pocketpc_vga;

	int handle_event(SDL_Event *event);
	void mouse_down(int mx, int my);
	void mouse_up(int mx, int my);
	void injectKeyEvent(char key, SDLKey sdlkey = SDLK_SPACE);
	void ActivateOtherButton(int button);
	void areaHighlight(int l, int t, int r, int b);
	void show(int placex = -1, int placey = -1);
	void hide();
	void minimize(int placearea = KEYG_LOCBOTTOM);
	bool autopaint;
	void moveToCorner(int corner);
	int locx;
	int locy;
	int lastlocx;
	int lastlocy;
	int width;
	int height;
	int state;
	int buttonDown[4];
	bool altDown;
	bool ctrlDown;
	void paint();
};

#endif
#endif