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

using std::ifstream;

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
	ifstream in(ucfile);
	Usecode_symbol_table symtbl;
	long magic = Read4(in);		// Test for symbol table.
	if (!in.good()) {
		EStudio::Alert("Error reading '%s'.", ucfile);
		return;
	}
	if (magic != UCSYMTBL_MAGIC0 || (magic = Read4(in)) 
							!= UCSYMTBL_MAGIC1)
		return;
	symtbl.read(in);
	const Usecode_symbol_table::Syms_vector& syms = symtbl.get_symbols();
	Usecode_symbol_table::Syms_vector::const_iterator siter;
	for (siter = syms.begin(); siter != syms.end(); ++siter) {
		Usecode_symbol *sym = *siter;
		Usecode_symbol::Symbol_kind kind = sym->get_kind();
		char *kindstr = 0;
		const char *nm = sym->get_name();
		if (!nm[0])
			continue;
		switch (kind) {
		case Usecode_symbol::fun_defined:
			kindstr = "Function";
			break;
		case Usecode_symbol::class_scope:
			kindstr = "Class";
			break;
		default:
			continue;
		}
		char num[20];
		sprintf(num, "%05xH", sym->get_val());
		GtkTreeIter iter;
		gtk_tree_store_append(model, &iter, NULL);
		gtk_tree_store_set(model, &iter, NAME_COL, nm,
			NUM_COL, num, TYPE_COL, kindstr, -1);
	}
}

