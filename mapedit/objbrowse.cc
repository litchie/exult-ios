#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "objbrowse.h"

Object_browser::Object_browser(Shape_group *grp) : group(grp)
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

void Object_browser::end_terrain_editing()
{
}

GtkWidget *Object_browser::get_widget() 
{
	return widget;
}
