#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "objbrowse.h"
#include "shapegroup.h"
#include "shapefile.h"
#include "studio.h"
#include "exceptions.h"

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

GtkWidget *Object_browser::get_widget() 
{
	return widget;
}

void Object_browser::on_browser_group_add
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
		GtkWidget *mitem = ExultStudio::add_menu_item(popup,
				"Add to group...");
		GtkWidget *group_menu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(mitem), group_menu);
		for (int i = 0; i < gcnt; i++)
			{
			Shape_group *grp = groups->get(i);
			if (grp == group)
				continue;// Skip ourself.
			GtkWidget *gitem = ExultStudio::add_menu_item(
				group_menu, grp->get_name(),
				GTK_SIGNAL_FUNC (
			   Object_browser::on_browser_group_add),
								this);
					// Store group on menu item.
			gtk_object_set_user_data(GTK_OBJECT(gitem), grp);
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
	const char *title,
	File_sel_okay_fun ok_handler,
	gpointer user_data
	)
	{
	GtkFileSelection *fsel = GTK_FILE_SELECTION(gtk_file_selection_new(
								title));
	gtk_window_set_modal(GTK_WINDOW(fsel), true);
	gtk_object_set_user_data(GTK_OBJECT(fsel), (void *)ok_handler);
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

/*
 *	Save file in browser.
 */

void Object_browser::on_browser_file_save
	(
	GtkMenuItem *item,
	gpointer udata
	)
	{
	Object_browser *chooser = (Object_browser *) udata;
	if (!chooser->file_info)
		return;			// Nothing to write to.
	try {
		chooser->file_info->flush();
	} catch (const exult_exception& e) {
		EStudio::Alert(e.what());
	}
	}

/*
 *	Revert file in browser to what's on disk.
 */

void Object_browser::on_browser_file_revert
	(
	GtkMenuItem *item,
	gpointer udata
	)
	{
	Object_browser *chooser = (Object_browser *) udata;
	if (!chooser->file_info)
		return;			// No file?
	char *msg = g_strdup_printf("Okay to throw away any changes to '%s'?",
			chooser->file_info->get_basename());
	if (EStudio::Prompt(msg, "Yes", "No") != 0)
		return;
	if (!chooser->file_info->revert())
		EStudio::Alert("Revert not yet implemented for this file");
	else
		{
		chooser->render();	// Repaint.
		chooser->show();
		}
	}

/*
 *	Set up popup menu for shape browser.
 */

GtkWidget *Object_browser::create_popup
	(
	)
	{
	if (popup)			// Clean out old.
		gtk_widget_destroy(popup);
	popup = gtk_menu_new();		// Create popup menu.
					// Add "File" submenu.
	GtkWidget *mitem = ExultStudio::add_menu_item(popup, "File...");
	GtkWidget *file_menu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mitem), file_menu);
	ExultStudio::add_menu_item(file_menu, "Save",
				GTK_SIGNAL_FUNC(on_browser_file_save), this);
	ExultStudio::add_menu_item(file_menu, "Revert",
				GTK_SIGNAL_FUNC(on_browser_file_revert), this);
	if (selected >= 0)		// Item selected?  Add groups.
		add_group_submenu(popup);
	return popup;
	}
