#ifndef TOUCHUI_H
#define TOUCHUI_H

#include <SDL.h>

class TouchUI {
public:
	static int eventType;
	enum {
		EVENT_CODE_INVALID = 0,
		EVENT_CODE_TEXT_INPUT = 1
	};
	static void onTextInput(const char *text);

	TouchUI();
	~TouchUI();
	virtual void promptForName(const char *name)=0;
	virtual void showGameControls()=0;
	virtual void hideGameControls()=0;
	virtual void showButtonControls()=0;
	virtual void hideButtonControls()=0;
	virtual void onDpadLocationChanged()=0;
};

#endif