#ifndef IOS_UTILS_H
#define IOS_UTILS_H

#include "touchui.h"

class TouchUI_iOS: public TouchUI {
public:
	TouchUI_iOS();
	void promptForName(const char *name);
	void showGameControls();
	void hideGameControls();
	void showButtonControls();
	void hideButtonControls();
	void onDpadLocationChanged();
};

const char* ios_get_documents_dir();
void ios_open_url(const char *);

#endif
