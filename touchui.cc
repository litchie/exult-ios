#include "touchui.h"
#include <cstring>

int TouchUI::eventType = -1;

void TouchUI::onTextInput(const char *text)
{
    SDL_Event event;
	
	if (text == NULL) {
		return;
	}
	
    SDL_zero(event);
    event.type = TouchUI::eventType;
    event.user.code = TouchUI::EVENT_CODE_TEXT_INPUT;
    event.user.data1 = strdup(text);
    event.user.data2 = 0;
    SDL_PushEvent(&event);
}

TouchUI::TouchUI()
{
	TouchUI::eventType = SDL_RegisterEvents(1);
}

TouchUI::~TouchUI()
{

}
