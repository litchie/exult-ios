#ifdef UNDER_CE
#include "touchscreen.h"
#include <sdl.h>

clsTouchscreen::clsTouchscreen() {
	mode = TOUCHMODE_DISABLED;
	modeRight = 0;
	modeDouble = 0;
	skipnextdouble = false;
}

clsTouchscreen::~clsTouchscreen() {
}

void clsTouchscreen::setModes(int Right, int Double) {
	if (Right == -1)
		Right = modeRight;
	if (Double == -1)
		Double = modeDouble;

	if (Right)
		modeRight = 1;  // In case someone decides to use a value over 1
	else
		modeRight = 0;

	if (Double)
		modeDouble = 1; // In case someone decides to use a value over 1
	else
		modeDouble = 0;

}

int clsTouchscreen::toggleRight(void) {
	if (modeRight)
		modeRight = 0;
	else
		modeRight = 1;
	return modeRight;
}

int clsTouchscreen::toggleDouble(void) {
	if (modeDouble)
		modeDouble = 0;
	else
		modeDouble = 1;
	return modeDouble;
}

void clsTouchscreen::getModes(int *Right, int *Double) {
	*Right = modeRight;
	*Double = modeDouble;
}

void clsTouchscreen::handle_event(SDL_Event *event) {
	if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP)
		reinterpret_mouse_event(event);
}

void clsTouchscreen::reinterpret_mouse_event(SDL_Event *event) {
	if (modeRight)
		event->button.button = SDL_BUTTON_RIGHT;
	if (modeDouble) {
		if (skipnextdouble) {
			skipnextdouble = false;
		} else {
			skipnextdouble = true;
			SDL_PushEvent(event); // Generate double click
		}
	}
}

#endif
