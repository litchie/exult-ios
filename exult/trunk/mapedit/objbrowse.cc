#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "objbrowse.h"
#include "shapegroup.h"
#include "studio.h"

Object_browser::Object_browser(Shape_group *grp, Shape_file_info *fi) 
	: group(grp), file_info(fi), popup(0),
	selected(-1)
{
	widget = 0;
}

Object_browser::~Object_browser()
{
	if (popup)
		gtk_widget_destroy(popup);
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

void Object_browser::set_background_color(guint32)
{
}

bool Object_browser::closing(bool can_cancel)
{
	return true;			// Default:  Okay to close.
}

GtkWidget *Object_browser::get_widget() 
{
	return widget;
}

void Object_browser::on_shapes_popup_add2group_activate
	(
	GtkMenuItem *item,
	gpointer udata
	)
	{
	Object_browser *chooser = (Object_browser *) udata;
	Shape_group *grp = (Shape_group *) gtk_object_get_user_data(
							GTK_OBJECT(item));
	int id = chooser->get_selected_id();
	if (id >= 0)			// Selected shape?
		{
		grp->add(id);		// Add & redisplay open windows.
		ExultStudio::get_instance()->update_group_windows(grp);
		}
	}

/*
 *	Add an "Add to group..." submenu to a popup for our group.
 */

void Object_browser::add_group_submenu
	(
	GtkWidget *popup
	)
	{
					// Use our group, or assume we're in
					//   the main window.
	Shape_group_file *groups = group ? group->get_file()
			: ExultStudio::get_instance()->get_cur_groups();
	int gcnt = groups ? groups->size() : 0;
	if (gcnt > 1 ||			// Groups besides ours?
	    (gcnt == 1 && !group))
		{
		GtkWidget *mitem = 
			gtk_menu_item_new_with_label("Add to group...");
		gtk_widget_show(mitem);
		gtk_menu_append(GTK_MENU(popup), mitem);
		GtkWidget *group_menu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(mitem), group_menu);
		for (int i = 0; i < gcnt; i++)
			{
			Shape_group *grp = groups->get(i);
			if (grp == group)
				continue;// Skip ourself.
			GtkWidget *gitem = gtk_menu_item_new_with_label(
							grp->get_name());
			gtk_widget_show(gitem);
			gtk_menu_append(GTK_MENU(group_menu), gitem);
					// Store group on menu item.
			gtk_object_set_user_data(GTK_OBJECT(gitem), grp);
			gtk_signal_connect (GTK_OBJECT (gitem), "activate",
				GTK_SIGNAL_FUNC (
			   Object_browser::on_shapes_popup_add2group_activate),
								this);
			}
		}
	}

/*
 *	Okay clicked in file-selector.
 */

void File_selector_ok
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	GtkFileSelection *fsel = GTK_FILE_SELECTION(gtk_widget_get_toplevel(
					GTK_WIDGET(btn)));
	char *fname = gtk_file_selection_get_filename(fsel);
	File_sel_okay_fun fun = (File_sel_okay_fun)
				gtk_object_get_user_data(GTK_OBJECT(fsel));
	if (fname && *fname && fun)
		(*fun)(fname, user_data);
	}

/*
 *	Create a modal file selector.
 */

GtkFileSelection *Create_file_selection
	(
	char *title,
	File_sel_okay_fun ok_handler,
	gpointer user_data
	)
	{
	GtkFileSelection *fsel = GTK_FILE_SELECTION(gtk_file_selection_new(
								title));
	gtk_window_set_modal(GTK_WINDOW(fsel), true);
	gtk_object_set_user_data(GTK_OBJECT(fsel), ok_handler);
	gtk_signal_connect(GTK_OBJECT(fsel->ok_button), "clicked",
			GTK_SIGNAL_FUNC(File_selector_ok), user_data);
					// Destroy when done.
	gtk_signal_connect_object(GTK_OBJECT(fsel->ok_button), "clicked",
				GTK_SIGNAL_FUNC(gtk_widget_destroy), 
						GTK_OBJECT(fsel));
	gtk_signal_connect_object(GTK_OBJECT(fsel->cancel_button), "clicked",
				GTK_SIGNAL_FUNC(gtk_widget_destroy), 
						GTK_OBJECT(fsel));
	return fsel;
	}	

