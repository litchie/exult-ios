#ifdef UNDER_CE
#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <sdl.h>

enum TOUCHSCREEN_MODES {    TOUCHMODE_DISABLED,
                            TOUCHMODE_DOUBLE,
                            TOUCHMODE_RIGHT,
                            TOUCHMODE_DOUBLERIGHT,
                            TOUCHMODE_INVALID,
                       };

class clsTouchscreen {
public:
	clsTouchscreen();
	~clsTouchscreen();
	void handle_event(SDL_Event *event);
	void setModes(int Right, int Double);
	void getModes(int *Right, int *Double);
	int toggleRight(void);
	int toggleDouble(void);
private:
	void reinterpret_mouse_event(SDL_Event *event);
	int mode;
	int modeRight;
	int modeDouble;
	bool skipnextdouble;
};

#endif
#endif
