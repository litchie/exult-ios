/**
 **	Ucbrowse.cc - Browse usecode functions.
 **
 **	Written: Nov. 19, 2006 - JSF
 **/

/*
Copyright (C) 2001-2006 The Exult Team

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "studio.h"
#include "ucbrowse.h"
#include "ucsymtbl.h"
#include "utils.h"

/*	Columns in our table. */
enum { NAME_COL, NUM_COL, TYPE_COL, N_COLS };

/*
 *	Open browser window.
 */

void ExultStudio::open_usecode_browser_window
	(
	)
	{
	if (!ucbrowsewin)			// First time?
		{
		ucbrowsewin = new Usecode_browser();
		ucbrowsewin->setup_list();
		}
	ucbrowsewin->show(true);
	}

/*
 *	Usecode_browser window's cancel button.
 */
C_EXPORT void on_usecodes_cancel_clicked
	(
	GtkButton *btn,
	gpointer user_data
	)
	{
	Usecode_browser *ucb = (Usecode_browser *) gtk_object_get_user_data(
			GTK_OBJECT(gtk_widget_get_toplevel(GTK_WIDGET(btn))));
	ucb->cancel();
	}

/*
 *	Usecode_browser window's X button.
 */
C_EXPORT gboolean on_usecodes_dialog_delete_event
	(
	GtkWidget *widget,
	GdkEvent *event,
	gpointer user_data
	)
	{
	Usecode_browser *ucb = (Usecode_browser *) 
				gtk_object_get_user_data(GTK_OBJECT(widget));
	
	ucb->cancel();
	return TRUE;
	}

/*
 *	Create usecode browser window.
 */

Usecode_browser::Usecode_browser
	(
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	GladeXML *app_xml = studio->get_xml();
	win = glade_xml_get_widget(app_xml, "usecodes_dialog");
	gtk_object_set_user_data(GTK_OBJECT(win), this);
	string ucname = get_system_path("<PATCH>/usecode");
	if (!U7exists(ucname.c_str()))
		{
		ucname = get_system_path("<STATIC>/usecode");
		if (!U7exists(ucname.c_str()))
			ucname = "";
		}
	studio->set_entry("usecodes_file", ucname.c_str());
	/*
	 *	Set up table.
	 */
	model = gtk_tree_store_new (
		N_COLS,
		G_TYPE_STRING,		// Name.
		G_TYPE_STRING,		// Number.
		G_TYPE_STRING);		// Type:  function, class.
					// Create view, and set our model.
	GtkWidget *tree = glade_xml_get_widget(app_xml, "usecodes_treeview");
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(model));;
	g_object_unref(G_OBJECT(model));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(
			"Name", renderer, "text", NAME_COL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);
	col = gtk_tree_view_column_new_with_attributes(
			"Number", renderer, "text", NUM_COL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);
	col = gtk_tree_view_column_new_with_attributes(
			"Type", renderer, "text", TYPE_COL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), TRUE);
	gtk_widget_show(tree);
	}

/*
 *	Delete.
 */

Usecode_browser::~Usecode_browser
	(
	)
	{
	g_object_unref(model);
	gtk_widget_destroy(win);
	}

/*
 *	Show/hide.
 */

void Usecode_browser::show
	(
	bool tf
	)
	{
	if (tf)
		gtk_widget_show(win);
	else
		gtk_widget_hide(win);
	}

/*
 *	Set up list of usecode entries.
 */

void Usecode_browser::setup_list
	(
	)
	{
	ExultStudio *studio = ExultStudio::get_instance();
	const char *ucfile = studio->get_text_entry("usecodes_file");
	GtkTreeIter iter;
	gtk_tree_store_append(model, &iter, NULL);
	//++++TESTING
	gtk_tree_store_set(model, &iter, NAME_COL, "AName",
		NUM_COL, "ANumber", TYPE_COL, "Function", -1);
#if 0
	//+++++FINISH
GtkTreeIter iter1;  /* Parent iter */
GtkTreeIter iter2;  /* Child iter  */

gtk_tree_store_append (model, &iter1, NULL);  /* Acquire a top-level iterator */
gtk_tree_store_set (model, &iter1,
                    TITLE_COLUMN, "The Art of Computer Programming",
                    AUTHOR_COLUMN, "Donald E. Knuth",
                    CHECKED_COLUMN, FALSE,
                    -1);

gtk_tree_store_append (model, &iter2, &iter1);  /* Acquire a child iterator */
gtk_tree_store_set (model, &iter2,
                    TITLE_COLUMN, "Volume 1: Fundamental Algorithms",
                    -1);

gtk_tree_store_append (model, &iter2, &iter1);
gtk_tree_store_set (model, &iter2,
                    TITLE_COLUMN, "Volume 2: Seminumerical Algorithms",
                    -1);

gtk_tree_store_append (model, &iter2, &iter1);
gtk_tree_store_set (model, &iter2,
                    TITLE_COLUMN, "Volume 3: Sorting and Searching",
                    -1);
#endif
	}

