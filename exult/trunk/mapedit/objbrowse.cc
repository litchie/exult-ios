#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "objbrowse.h"

Object_browser::Object_browser()
{
	widget = 0;
}

Object_browser::~Object_browser()
{
}

void Object_browser::set_widget(GtkWidget *w)
{
	widget = w;
}

bool Object_browser::server_response(int , unsigned char *, int )
{
	return false;			// Not handled here.
}

GtkWidget *Object_browser::get_widget() 
{
	return widget;
}
